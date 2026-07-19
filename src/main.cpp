/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * This file is part of PvZ-Portable.
 *
 * PvZ-Portable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PvZ-Portable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.
 */

#include "LawnApp.h"
#include "Resources.h"
#include "Sexy.TodLib/TodStringFile.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace Sexy;

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#ifdef __3DS__
#include <3ds.h>
#include <malloc.h>
extern "C" {
	unsigned int __stacksize__ = 512 * 1024;
}
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

bool (*gAppCloseRequest)();
bool (*gAppHasUsedCheatKeys)();
std::string (*gGetCurrentLevelName)();

#ifdef _WIN32
static std::vector<std::string> gUtf8ArgsStorage;
static std::vector<char*> gUtf8Argv;

static void BuildUtf8ArgsFromWin32(int& argc, char**& argv)
{
	int aWideArgc = 0;
	LPWSTR* aWideArgv = CommandLineToArgvW(GetCommandLineW(), &aWideArgc);
	if (aWideArgv == nullptr || aWideArgc <= 0)
		return;

	gUtf8ArgsStorage.clear();
	gUtf8Argv.clear();
	gUtf8ArgsStorage.reserve(static_cast<size_t>(aWideArgc));
	gUtf8Argv.reserve(static_cast<size_t>(aWideArgc));

	for (int i = 0; i < aWideArgc; ++i)
	{
		const wchar_t* aWide = aWideArgv[i];
		int aLen = WideCharToMultiByte(CP_UTF8, 0, aWide, -1, nullptr, 0, nullptr, nullptr);
		if (aLen <= 0)
		{
			gUtf8ArgsStorage.emplace_back();
		}
		else
		{
			std::string aUtf8;
			aUtf8.resize(static_cast<size_t>(aLen - 1));
			WideCharToMultiByte(CP_UTF8, 0, aWide, -1, aUtf8.data(), aLen, nullptr, nullptr);
			gUtf8ArgsStorage.emplace_back(std::move(aUtf8));
		}
	}

	for (auto& aStr : gUtf8ArgsStorage)
		gUtf8Argv.push_back(const_cast<char*>(aStr.c_str()));

	argc = static_cast<int>(gUtf8Argv.size());
	argv = gUtf8Argv.data();

	LocalFree(aWideArgv);
}
#endif

#ifdef _WIN32
// Windows SEH 异常捕获：把崩溃信息写入 log.txt，便于定位自定义僵尸等问题
// 使用 Vectored Exception Handler，在 SEH filter 之前触发，
// 即使栈溢出或 C 运行时 abort 也能捕获（比 SetUnhandledExceptionFilter 更可靠）

// 把一个代码地址解析成 "模块名+偏移" 的字符串，写入 buf
// 例如 0x00007ff7ef819691 -> "pvz-portable.exe+0x19691"
static void FormatAddressWithModule(char* buf, size_t bufSize, void* addr)
{
	HMODULE hMod = nullptr;
	if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCWSTR)addr, &hMod) && hMod != nullptr)
	{
		wchar_t wpath[MAX_PATH] = { 0 };
		GetModuleFileNameW(hMod, wpath, MAX_PATH);
		// 取 basename
		const wchar_t* base = wpath;
		for (const wchar_t* p = wpath; *p; ++p) {
			if (*p == L'\\' || *p == L'/') base = p + 1;
		}
		char name[MAX_PATH] = { 0 };
		WideCharToMultiByte(CP_UTF8, 0, base, -1, name, MAX_PATH, nullptr, nullptr);
		ptrdiff_t offset = (char*)addr - (char*)hMod;
		std::snprintf(buf, bufSize, "%s+0x%llX", name, static_cast<unsigned long long>(offset));
	}
	else
	{
		std::snprintf(buf, bufSize, "0x%p", addr);
	}
}

static LONG WINAPI CrashLogger(EXCEPTION_POINTERS* ep)
{
	const char* excName = "Unknown";
	switch (ep->ExceptionRecord->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:         excName = "ACCESS_VIOLATION"; break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    excName = "ARRAY_BOUNDS_EXCEEDED"; break;
		case EXCEPTION_BREAKPOINT:               excName = "BREAKPOINT"; break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:      excName = "ILLEGAL_INSTRUCTION"; break;
		case EXCEPTION_STACK_OVERFLOW:           excName = "STACK_OVERFLOW"; break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:       excName = "INT_DIVIDE_BY_ZERO"; break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:    excName = "DATATYPE_MISALIGNMENT"; break;
		case EXCEPTION_PRIV_INSTRUCTION:         excName = "PRIV_INSTRUCTION"; break;
		case EXCEPTION_IN_PAGE_ERROR:            excName = "IN_PAGE_ERROR"; break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: excName = "NONCONTINUABLE_EXCEPTION"; break;
		case EXCEPTION_INVALID_DISPOSITION:      excName = "INVALID_DISPOSITION"; break;
		case EXCEPTION_GUARD_PAGE:               excName = "GUARD_PAGE"; break;
		case EXCEPTION_INVALID_HANDLE:           excName = "INVALID_HANDLE"; break;
		default: break;
	}

	char addrBuf[256] = { 0 };
	FormatAddressWithModule(addrBuf, sizeof(addrBuf), ep->ExceptionRecord->ExceptionAddress);

	std::fprintf(stderr, "[CRASH] VEH exception 0x%08X (%s) at %s\n",
		static_cast<unsigned int>(ep->ExceptionRecord->ExceptionCode),
		excName,
		addrBuf);
	if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && ep->ExceptionRecord->NumberParameters >= 2) {
		std::fprintf(stderr, "[CRASH] Access violation: %s address 0x%p\n",
			ep->ExceptionRecord->ExceptionInformation[0] == 0 ? "reading" : "writing",
			(void*)ep->ExceptionRecord->ExceptionInformation[1]);
	}

#if defined(_M_X64) || defined(__x86_64__)
	// 打印关键寄存器，便于定位
	std::fprintf(stderr, "[CRASH] Registers: RIP=0x%llX RSP=0x%llX RBP=0x%llX\n",
		(unsigned long long)ep->ContextRecord->Rip,
		(unsigned long long)ep->ContextRecord->Rsp,
		(unsigned long long)ep->ContextRecord->Rbp);
	std::fprintf(stderr, "[CRASH] Registers: RCX=0x%llX RDX=0x%llX R8=0x%llX R9=0x%llX\n",
		(unsigned long long)ep->ContextRecord->Rcx,
		(unsigned long long)ep->ContextRecord->Rdx,
		(unsigned long long)ep->ContextRecord->R8,
		(unsigned long long)ep->ContextRecord->R9);
	std::fprintf(stderr, "[CRASH] Registers: RAX=0x%llX RBX=0x%llX RCX=0x%llX RDI=0x%llX RSI=0x%llX\n",
		(unsigned long long)ep->ContextRecord->Rax,
		(unsigned long long)ep->ContextRecord->Rbx,
		(unsigned long long)ep->ContextRecord->Rcx,
		(unsigned long long)ep->ContextRecord->Rdi,
		(unsigned long long)ep->ContextRecord->Rsi);
#endif

	// 捕获调用栈（最多 32 帧）
	// RtlCaptureStackBackTrace 在 VEH 中能捕获当前栈，但会包含 VEH handler 自身的帧
	{
		const int kMaxFrames = 32;
		void* frames[kMaxFrames] = { 0 };
		USHORT n = RtlCaptureStackBackTrace(0 /*跳过0帧，保留全部*/, kMaxFrames, frames, nullptr);
		std::fprintf(stderr, "[CRASH] Backtrace (%u frames):\n", (unsigned)n);
		for (USHORT i = 0; i < n; ++i) {
			char buf[256] = { 0 };
			FormatAddressWithModule(buf, sizeof(buf), frames[i]);
			std::fprintf(stderr, "[CRASH]   #%02u %s\n", (unsigned)i, buf);
		}
	}

	std::fflush(stderr);
	// 不处理异常，让下一个 handler 继续处理（包括默认的崩溃对话框）
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

int main(int argc, char** argv)
{
#ifdef __3DS__
	osSetSpeedupEnable(true);
#endif

#ifdef _WIN32
	BuildUtf8ArgsFromWin32(argc, argv);
	// 用 VectoredExceptionHandler 而非 SetUnhandledExceptionFilter，
	// 因为前者在 SEH 链最早处触发，能捕获栈溢出等 SetUnhandledExceptionFilter 无法处理的场景
	AddVectoredExceptionHandler(1 /*第一个被调用*/, CrashLogger);
#endif

	TodStringListSetColors(gLawnStringFormats, gLawnStringFormatCount);
	gGetCurrentLevelName = LawnGetCurrentLevelName;
	gAppCloseRequest = LawnGetCloseRequest;
	gAppHasUsedCheatKeys = LawnHasUsedCheatKeys;
	gExtractResourcesByName = Sexy::ExtractResourcesByName;
	gLawnApp = new LawnApp();
	gLawnApp->SetArgs(argc, argv);
	gLawnApp->Init();
	gLawnApp->Start();
#ifndef __EMSCRIPTEN__
	gLawnApp->Shutdown();
	if (gLawnApp)
		delete gLawnApp;
#endif

	return 0;
};
