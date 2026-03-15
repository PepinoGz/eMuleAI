//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#pragma once

// DebugLeakHelper - CRT leak dump helper for debug builds.
// Include your project's PCH (stdafx.h) BEFORE this header.

// Only meaningful in _DEBUG; in release these are no-ops.
#include <cstddef>
#include <cstdint>

#ifdef _DEBUG
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // for CRITICAL_SECTION
// Global critical section used by call sites in Emule.cpp (kept in global ns on purpose).
extern CRITICAL_SECTION g_allocCS;
#endif

namespace DebugLeakHelper
{
	typedef void (__cdecl *PreDumpHook)();

#ifdef _DEBUG
    void EarlyInit();

    // Call very early (e.g., InitInstance). Captures an initial CRT heap snapshot.
    void Init();

    // Optional: safe to call later too; will just ensure flags/snapshot are ready.
    void LateInit();

	// Parses EMULE_CRT_BREAKALLOC / EMULE_CRT_BREAKALLOCS env vars (e.g. "123,456") and applies breakpoints.
    void ParseAndApplyBreakAllocsFromEnv();

	// Optional hook executed right before DumpLeaksToCrt() calls _CrtDumpMemoryLeaks().
	void RegisterPreDumpHook(PreDumpHook hook);

    // Adds a single break allocation id at runtime (also stored for ShouldBreakAlloc).
    void AddBreakAlloc(std::uint32_t allocId);

    // Returns true if given alloc id is configured to be broken on.
    bool ShouldBreakAlloc(unsigned long allocId);

    // Tracks allocation metadata by CRT request id for same-run leak resolving.
    void TrackAllocHookEvent(int mode, void* pUserData, size_t nSize, int nBlockUse, long lRequest, const unsigned char* pszFileName, int nLine);

	// Returns true while DumpLeaksToCrt() is enumerating the heap.
	bool IsLeakDumpInProgress();

    // Formats tracked allocation metadata for a CRT request id into a caller buffer.
    bool TryDescribeTrackedAlloc(unsigned long allocId, LPTSTR pszBuffer, size_t cchBuffer);

    // Dumps leaks since Init() to the normal CRT debug output, then prints small hints.
    void DumpLeaksToCrt();
#else
    inline void EarlyInit() {}
    inline void Init() {}
    inline void LateInit() {}
    inline void ParseAndApplyBreakAllocsFromEnv() {}
	inline void RegisterPreDumpHook(PreDumpHook) {}
    inline void AddBreakAlloc(std::uint32_t) {}
    inline bool ShouldBreakAlloc(unsigned long) { return false; }
    inline void TrackAllocHookEvent(int, void*, size_t, int, long, const unsigned char*, int) {}
	inline bool IsLeakDumpInProgress() { return false; }
	inline bool TryDescribeTrackedAlloc(unsigned long, LPTSTR, size_t) { return false; }
    inline void DumpLeaksToCrt() {}
#endif
} // namespace DebugLeakHelper
