#!/usr/bin/env python3
"""
gen_sol2_bindings.py — 解析 C++ 头文件，批量生成 sol2 Lua 绑定代码

用法:
    python tools/gen_sol2_bindings.py

功能:
    1. 解析指定的 C++ 头文件（Zombie.h, Plant.h, Board.h 等）
    2. 提取 class 的 public 方法和成员变量
    3. 生成 sol2 usertype 绑定代码到 src/Mod/Lua/BindSol2Generated.inl

生成的绑定代码由 BindSol2.cpp 包含，在 ModLua::Initialize() 中调用。

绑定策略:
    - 成员变量: 直接绑定 &Class::member（sol2 自动生成 getter/setter）
    - 简单方法（值参数/指针返回）: 直接绑定 &Class::method
    - 复杂方法（引用参数/默认值/const 限定）: 跳过并输出注释，需手动绑定
    - 跳过: 构造/析构函数、运算符重载、static 方法、内联实现的方法
"""

import re
import os
import sys
from dataclasses import dataclass, field
from typing import List, Tuple, Optional

# ===== 配置 =====

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# 要绑定的类: (头文件相对路径, 类名, metatable名, 绑定函数名)
BIND_TARGETS = [
    ("src/Lawn/Zombie.h",       "Zombie",      "PvZ.Zombie",      "BindSol2Zombie"),
    ("src/Lawn/Plant.h",        "Plant",       "PvZ.Plant",       "BindSol2Plant"),
    ("src/Lawn/Board.h",        "Board",       "PvZ.Board",       "BindSol2Board"),
    ("src/Lawn/Projectile.h",   "Projectile",  "PvZ.Projectile",  "BindSol2Projectile"),
    ("src/Lawn/Coin.h",         "Coin",        "PvZ.Coin",        "BindSol2Coin"),
    ("src/Lawn/GridItem.h",     "GridItem",    "PvZ.GridItem",    "BindSol2GridItem"),
    ("src/Lawn/LawnMower.h",    "LawnMower",   "PvZ.LawnMower",   "BindSol2LawnMower"),
]

# 输出文件
OUTPUT_FILE = "src/Mod/Lua/BindSol2Generated.inl"

# 需要跳过的方法名（构造/析构/运算符等在 parse_class_body 中内联处理）
# 需要跳过的成员名模式
SKIP_MEMBERS = set()

# 类型映射: C++ 类型 → Lua 绑定方式提示
# sol2 可以直接处理大部分内置类型，这里只标记需要特殊处理的
SIMPLE_RETURN_TYPES = {
    'void', 'bool', 'int', 'int32_t', 'int64_t', 'uint32_t', 'uint64_t',
    'float', 'double', 'size_t', 'unsigned', 'long',
}

# 不绑定的方法名（手动绑定过或会引起问题）
SKIP_METHOD_NAMES = {
    # 已手动绑定的（避免冲突）
    # Zombie
    'get_ptr',  # 已有手动绑定
    # Board — 实现被注释掉的方法（头文件有声明但 .cpp 中被 #if 0 或注释）
    'GetSquirrelAt',  # Board.cpp 中实现被注释掉，链接时会报 undefined reference
}


@dataclass
class MethodInfo:
    return_type: str
    name: str
    params: str  # 原始参数列表
    is_const: bool
    is_static: bool
    is_inline: bool  # 有 {} 实现的
    raw_line: str


@dataclass
class MemberInfo:
    type: str
    name: str
    is_array: bool  # 是否是数组类型（mMember[...]）
    raw_line: str


@dataclass
class ClassInfo:
    name: str
    methods: List[MethodInfo] = field(default_factory=list)
    members: List[MemberInfo] = field(default_factory=list)


def read_file(path: str) -> str:
    with open(path, 'r', encoding='utf-8') as f:
        return f.read()


def remove_preprocessor(content: str) -> str:
    """移除 #ifdef/#ifndef/#endif 等预处理指令块和行内注释，保留 #else 分支内容"""
    lines = content.split('\n')
    result = []
    # 简单策略: 跳过所有预处理行
    for line in lines:
        stripped = line.strip()
        if stripped.startswith('#'):
            continue
        # 移除行内 /* */ 注释（如 /*inline*/ Zombie* AddZombie(...)）
        line = re.sub(r'/\*[^*]*\*/', '', line)
        result.append(line)
    return '\n'.join(result)


def extract_class(content: str, class_name: str) -> Optional[str]:
    """提取 class 定义块（从 'class ClassName' 到匹配的 '};'）"""
    # 找到 class 声明
    pattern = rf'(class|struct)\s+{re.escape(class_name)}\s*(?::|{{|\n)'
    match = re.search(pattern, content)
    if not match:
        return None

    start = match.start()
    # 找到匹配的闭合大括号
    depth = 0
    i = content.index('{', match.start())
    start_brace = i
    while i < len(content):
        if content[i] == '{':
            depth += 1
        elif content[i] == '}':
            depth -= 1
            if depth == 0:
                # 包含到 '};' 后面
                end = content.index(';', i) + 1 if i + 1 < len(content) and content[i+1] == ';' else i + 1
                return content[start:end]
        i += 1
    return None


def parse_class_body(class_body: str, class_name: str) -> ClassInfo:
    """解析 class 体，提取 public 方法和成员"""
    info = ClassInfo(name=class_name)

    # 找到 public: 段
    lines = class_body.split('\n')
    in_public = False
    current_access = 'private'

    for line in lines:
        stripped = line.strip()

        # 检测访问修饰符
        if re.match(r'^public\s*:', stripped):
            current_access = 'public'
            in_public = True
            continue
        elif re.match(r'^protected\s*:', stripped):
            current_access = 'protected'
            in_public = False
            continue
        elif re.match(r'^private\s*:', stripped):
            current_access = 'private'
            in_public = False
            continue

        if not in_public:
            continue

        # 跳过空行和注释
        if not stripped or stripped.startswith('//') or stripped.startswith('/*'):
            continue

        # 跳过 enum 定义
        if stripped.startswith('enum ') or stripped.startswith('enum{'):
            continue

        # 跳过 typedef / using
        if stripped.startswith('typedef') or stripped.startswith('using'):
            continue

        # 跳过 friend
        if stripped.startswith('friend'):
            continue

        # 跳过嵌套 class/struct
        if re.match(r'^(class|struct)\s+', stripped):
            continue

        # 检测是否是内联方法实现（有 {}）
        has_implementation = '{' in stripped or (not stripped.endswith(';') and '{' not in stripped)

        # 多行声明可能在下一行有 ;，这里简单处理单行
        # 方法声明: ReturnType  MethodName(params) [const];
        # 成员变量: Type  mMemberName;

        # 方法匹配
        method_match = re.match(
            r'^(.+?)\s+(\w+)\s*\(([^)]*)\)\s*(const)?\s*(=\s*0)?\s*;',
            stripped
        )
        if method_match:
            return_type = method_match.group(1).strip()
            method_name = method_match.group(2).strip()
            params = method_match.group(3).strip()
            is_const = method_match.group(4) is not None
            is_pure_virtual = method_match.group(5) is not None

            # 跳过构造/析构
            if method_name == class_name or method_name.startswith('~'):
                continue

            # 跳过运算符重载
            if method_name.startswith('operator'):
                continue

            # 跳过 static 方法
            is_static = 'static' in return_type
            if is_static:
                continue

            # 跳过手动绑定的
            if method_name in SKIP_METHOD_NAMES:
                continue

            info.methods.append(MethodInfo(
                return_type=return_type,
                name=method_name,
                params=params,
                is_const=is_const,
                is_static=is_static,
                is_inline=False,
                raw_line=stripped,
            ))
            continue

        # 成员变量匹配
        member_match = re.match(
            r'^(.+?)\s+(m\w+)\s*((?:\[[^\]]*\])+)\s*;',
            stripped
        )
        if member_match:
            # 数组类型成员（mMember[...]），sol2 无法直接绑定
            var_name = member_match.group(2).strip()
            # 仍然记录，但标记为数组，should_bind_member 会跳过
            var_type = member_match.group(1).strip()
            if 'static' not in var_type:
                info.members.append(MemberInfo(
                    type=var_type,
                    name=var_name,
                    is_array=True,
                    raw_line=stripped,
                ))
            continue

        # 非数组成员变量匹配
        member_match = re.match(
            r'^(.+?)\s+(m\w+)\s*;',
            stripped
        )
        if member_match:
            var_type = member_match.group(1).strip()
            var_name = member_match.group(2).strip()

            # 跳过 static 成员
            if 'static' in var_type:
                continue

            # 跳过 const 成员（不可写）
            # 不跳过，sol2 可以只读绑定

            info.members.append(MemberInfo(
                type=var_type,
                name=var_name,
                is_array=False,
                raw_line=stripped,
            ))
            continue

    return info


# 已注册 sol2 usertype 的类型（可以安全绑定指针参数）
REGISTERED_USERTYPES = {
    'Zombie', 'Plant', 'Board', 'Projectile', 'Coin', 'GridItem', 'LawnMower',
}

# 不可直接绑定的自定义结构体类型（sol2 无法自动处理值传递）
# 这些类型需要手动注册 usertype 或提供 sol::stack 特化
UNBINDABLE_VALUE_TYPES = {
    'Rect', 'Color', 'SexyVector3', 'SexyVector2', 'SexyMatrix3', 'SexyMatrix2',
    'ZombieDrawPosition', 'PlantDrawPosition', 'TodParticleHolder',
    'TodWeightedArray', 'TodColor', 'TodParticle', 'SexyMatrix',
    'GridItemGridType', 'PlantRowType',  # 枚举，但可能被当作结构体
}

# 未注册但可以作为 opaque pointer 传递的类型（sol2 可编译，但运行时需注意）
# 实际上这些类型的指针参数会导致 sol2 的默认指针转换与 NewUserdata 格式不兼容
# 所以必须跳过含有这些类型指针参数的方法
# 这里用黑名单方式：检测参数中的类型名，如果在白名单外则跳过


def extract_param_types(params: str) -> List[str]:
    """从参数列表中提取类型名"""
    if not params or params.strip() == 'void' or params.strip() == '':
        return []
    types = []
    # 按逗号分割参数（简化处理，不处理模板中的逗号）
    for param in params.split(','):
        param = param.strip()
        if not param:
            continue
        # 提取类型：去掉参数名（最后一个标识符）和修饰符
        # 例如 "Graphics* g" → "Graphics*"
        # "int theGridX" → "int"
        # "const char* name" → "const char*"
        parts = param.split()
        if not parts:
            continue
        # 去掉最后一个部分（参数名），除非整个就是一个类型（如 "int"）
        type_part = ' '.join(parts[:-1]) if len(parts) > 1 else parts[0]
        # 去掉 const/volatile 等修饰符，提取纯类型名
        type_part = type_part.replace('const ', '').replace('volatile ', '').strip()
        # 去掉 * 和 &
        clean_type = type_part.rstrip('*& ').strip()
        types.append(clean_type)
    return types


def should_bind_method(method: MethodInfo) -> Tuple[bool, str]:
    """判断方法是否可以自动绑定，返回 (可绑定, 原因)"""
    # 跳过返回引用的方法
    if '&' in method.return_type and '&&' not in method.return_type:
        return False, "returns reference"

    # 跳过有引用参数的方法
    if '&' in method.params and '&&' not in method.params:
        return False, "has reference param"

    # 有默认参数的方法不再跳过，改为生成 lambda wrapper（见 generate_lambda_wrapper）
    # 但仍需检查参数类型的可绑定性（在下方统一检查）

    # 检查返回值是否是不可绑定的自定义结构体类型
    return_type_clean = method.return_type.replace('const ', '').strip()
    return_type_clean = return_type_clean.rstrip('*& ').strip()
    for bad_type in UNBINDABLE_VALUE_TYPES:
        if bad_type in return_type_clean:
            return False, f"returns unbindable type ({bad_type})"

    # 检查返回值是否是指向未注册 usertype 的指针
    if '*' in method.return_type:
        if return_type_clean and return_type_clean not in REGISTERED_USERTYPES:
            if return_type_clean not in ('char', 'void'):
                return False, f"returns unregistered pointer type ({return_type_clean}*)"

    # 检查参数中是否包含未注册 usertype 的指针类型
    if method.params and method.params.strip() != 'void':
        for param in method.params.split(','):
            param = param.strip()
            if '*' in param:
                parts = param.split()
                type_part = ' '.join(parts[:-1]) if len(parts) > 1 else parts[0]
                type_part = type_part.replace('const ', '').strip()
                clean_type = type_part.rstrip('* ').strip()
                if clean_type and clean_type not in REGISTERED_USERTYPES:
                    if clean_type not in ('char', 'void'):
                        return False, f"has unregistered pointer param ({clean_type}*)"

    return True, ""


def should_bind_member(member: MemberInfo) -> Tuple[bool, str]:
    """判断成员变量是否可以自动绑定"""
    # 跳过数组类型（sol2 无法直接绑定 C 数组成员）
    if member.is_array:
        return False, "array member"
    # 跳过 const 成员
    if member.type.startswith('const '):
        return False, "const member"
    # 跳过复杂模板类型（DataArray<T>, TodList<T> 等未注册 usertype 的类型）
    for skip_type in ('DataArray<', 'TodList<', 'TodParticleHolder', 'TodSmoothArray'):
        if skip_type in member.type:
            return False, f"complex type ({member.type})"
    # 跳过不可绑定的自定义结构体类型
    member_type_clean = member.type.replace('const ', '').strip().rstrip('*& ').strip()
    for bad_type in UNBINDABLE_VALUE_TYPES:
        if bad_type in member_type_clean:
            return False, f"unbindable value type ({bad_type})"
    # 跳过指针类型成员变量（sol2 无法处理不完整类型的指针成员）
    if '*' in member.type:
        ptr_type = member_type_clean
        if ptr_type and ptr_type not in REGISTERED_USERTYPES:
            return False, f"pointer to unregistered type ({ptr_type}*)"
    return True, ""


def has_default_params(params: str) -> bool:
    """检查参数列表中是否有默认参数"""
    if not params or params.strip() == 'void':
        return False
    for param in params.split(','):
        param = param.strip()
        # 排除 != <= >= == 等运算符（虽然这些在参数声明中极罕见）
        if '=' in param and '!=' not in param and '<=' not in param and '>=' not in param and '==' not in param:
            return True
    return False


def parse_params_for_wrapper(params: str) -> List[Tuple[str, str, Optional[str]]]:
    """解析参数列表，返回 [(类型, 名称, 默认值或None), ...]"""
    if not params or params.strip() == 'void' or params.strip() == '':
        return []
    result = []
    for param in params.split(','):
        param = param.strip()
        if not param:
            continue

        # 分离默认值
        default_val = None
        for i in range(len(param)):
            if param[i] == '=' and i > 0 and param[i-1] not in ('!', '<', '>', '='):
                default_val = param[i+1:].strip()
                param = param[:i].strip()
                break

        # 分离类型和名称
        parts = param.split()
        if len(parts) >= 2:
            param_type = ' '.join(parts[:-1])
            param_name = parts[-1]
        else:
            param_type = parts[0]
            param_name = '_arg'

        result.append((param_type, param_name, default_val))
    return result


def generate_lambda_wrapper(class_name: str, method: MethodInfo) -> str:
    """为有默认参数的方法生成 lambda wrapper（去掉默认参数，自动填充默认值）"""
    params = parse_params_for_wrapper(method.params)

    # 必填参数（没有默认值的）
    required = [(t, n) for t, n, d in params if d is None]
    # 有默认值的参数（用于调用时填充）
    default_args = [d for t, n, d in params if d is not None]

    # lambda 参数列表：self + 必填参数
    lambda_params = ', '.join([f'{class_name}* self'] + [f'{t} {n}' for t, n in required])

    # 调用参数列表：必填参数名 + 默认值
    call_args = ', '.join([n for t, n in required] + default_args)

    # 返回类型处理
    ret = method.return_type.strip()
    if ret == 'void':
        return f'[]({lambda_params}) {{ self->{method.name}({call_args}); }}'
    else:
        return f'[]({lambda_params}) -> {ret} {{ return self->{method.name}({call_args}); }}'


def to_snake_case(name: str) -> str:
    """将 CamelCase 转为 snake_case"""
    result = []
    for i, c in enumerate(name):
        if c.isupper() and i > 0 and (name[i-1].islower() or (i+1 < len(name) and name[i+1].islower())):
            result.append('_')
        result.append(c.lower())
    return ''.join(result)


def generate_binding_code(class_info: ClassInfo, mt_name: str, bind_func_name: str) -> str:
    """为一个类生成 sol2 绑定代码"""
    lines = []
    lines.append(f"// ===== 自动生成的 {class_info.name} sol2 绑定 =====")
    lines.append(f"// 由 tools/gen_sol2_bindings.py 生成，请勿手动编辑")
    lines.append(f"static void {bind_func_name}(sol::state_view& lua) {{")
    lines.append(f'    auto ut = lua.new_usertype<{class_info.name}>("{mt_name}",')
    lines.append(f"        sol::no_constructor,")

    entries = []
    bind_entries = []  # 实际的绑定条目（非注释），用于确定最后一个不加逗号

    # 绑定成员变量
    for member in class_info.members:
        can_bind, reason = should_bind_member(member)
        if not can_bind:
            entries.append(("comment", f"        // {member.name}: 跳过 ({reason})"))
            continue
        entries.append(("bind", f'        "{member.name}", &{class_info.name}::{member.name}'))

    # 绑定方法
    for method in class_info.methods:
        can_bind, reason = should_bind_method(method)
        if not can_bind:
            lua_name = to_snake_case(method.name)
            entries.append(("comment", f"        // {lua_name}: 跳过 ({reason}) — {method.raw_line}"))
            continue
        lua_name = to_snake_case(method.name)

        # 有默认参数的方法生成 lambda wrapper
        if has_default_params(method.params):
            wrapper = generate_lambda_wrapper(class_info.name, method)
            entries.append(("bind", f'        "{lua_name}", {wrapper}'))
        else:
            entries.append(("bind", f'        "{lua_name}", &{class_info.name}::{method.name}'))

    # 找到最后一个 bind 条目的索引
    last_bind_idx = -1
    for i in range(len(entries) - 1, -1, -1):
        if entries[i][0] == "bind":
            last_bind_idx = i
            break

    # 写入 entries，最后一个 bind 不加逗号
    for i, (kind, content) in enumerate(entries):
        if kind == "bind":
            suffix = "" if i == last_bind_idx else ","
            lines.append(content + suffix)
        else:
            lines.append(content)

    lines.append("    );")
    lines.append("}")
    lines.append("")

    return '\n'.join(lines)


def generate_header(targets: list) -> str:
    """生成文件头"""
    lines = [
        "// ===== sol2 自动绑定代码 =====",
        "// 此文件由 tools/gen_sol2_bindings.py 自动生成",
        "// 请勿手动编辑，修改请运行: python tools/gen_sol2_bindings.py",
        "//",
        "// 绑定策略:",
        "//   - 成员变量: &Class::member (sol2 自动生成 getter/setter)",
        "//   - 简单方法: &Class::method (sol2 自动处理类型转换)",
        "//   - 默认参数方法: 生成 lambda wrapper 自动填充默认值",
        "//   - 复杂方法(引用参数/未注册指针): 跳过，需手动绑定",
        "//",
        f"// 绑定类数: {len(targets)}",
        "",
        "#pragma once",
        "",
        "#include <sol/sol.hpp>",
        "",
        "// 项目头文件",
    ]

    for header, class_name, mt_name, bind_func in targets:
        # 将 src/Lawn/Zombie.h 转为 ../../Lawn/Zombie.h (相对于 src/Mod/Lua/)
        rel_path = header.replace("src/", "../../")
        lines.append(f'#include "{rel_path}"')

    lines.append("")
    return '\n'.join(lines)


def main():
    os.chdir(PROJECT_ROOT)

    all_code = []
    all_code.append(generate_header(BIND_TARGETS))

    total_methods = 0
    total_members = 0
    total_skipped = 0

    for header_path, class_name, mt_name, bind_func in BIND_TARGETS:
        full_path = os.path.join(PROJECT_ROOT, header_path)
        if not os.path.exists(full_path):
            print(f"WARNING: {full_path} not found, skipping {class_name}")
            continue

        content = read_file(full_path)
        content = remove_preprocessor(content)
        class_body = extract_class(content, class_name)
        if not class_body:
            print(f"WARNING: class {class_name} not found in {header_path}")
            continue

        info = parse_class_body(class_body, class_name)

        bound_methods = 0
        bound_members = 0
        skipped = 0
        for m in info.methods:
            can, _ = should_bind_method(m)
            if can:
                bound_methods += 1
            else:
                skipped += 1
        for m in info.members:
            can, _ = should_bind_member(m)
            if can:
                bound_members += 1
            else:
                skipped += 1

        total_methods += bound_methods
        total_members += bound_members
        total_skipped += skipped

        print(f"  {class_name}: {bound_members} members + {bound_methods} methods bound ({skipped} skipped)")

        code = generate_binding_code(info, mt_name, bind_func)
        all_code.append(code)

    # 生成调用入口
    all_code.append("// ===== 绑定注册入口 =====")
    all_code.append("static void BindAllSol2(sol::state_view& lua) {")
    for _, _, _, bind_func in BIND_TARGETS:
        all_code.append(f"    {bind_func}(lua);")
    all_code.append("}")
    all_code.append("")

    # 写入文件
    output_path = os.path.join(PROJECT_ROOT, OUTPUT_FILE)
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(all_code))

    print(f"\n生成完成: {OUTPUT_FILE}")
    print(f"  总计: {total_members} 成员 + {total_methods} 方法绑定, {total_skipped} 跳过")


if __name__ == '__main__':
    main()
