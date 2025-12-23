/*
 * (C) 2024 MPC-HC Performance Optimization
 *
 * Performance optimization utilities to reduce CPU usage
 * without changing logic or features
 */

#pragma once

#include <windows.h>
#include <atlstr.h>

// Cached time formatting to avoid repeated string operations
class CTimeFormatCache
{
private:
    REFERENCE_TIME m_rtCached;
    CString m_strCached;
    bool m_bShowHours;
    bool m_bValid;

public:
    CTimeFormatCache() : m_rtCached(0), m_bShowHours(true), m_bValid(false) {}

    // Get formatted time string, using cache if time hasn't changed
    CString GetFormattedTime(REFERENCE_TIME rt, bool showHours = true, 
                             CString (*formatFunc)(const REFERENCE_TIME&) = nullptr);

    void Invalidate() { m_bValid = false; }
    void Reset() { m_rtCached = 0; m_bValid = false; }
};

// Window title cache to avoid unnecessary SetWindowText calls
class CWindowTitleCache
{
private:
    CString m_strCached;
    bool m_bValid;

public:
    CWindowTitleCache() : m_bValid(false) {}

    // Set window text only if it changed
    void SetWindowTextIfChanged(HWND hWnd, const CString& strNew);

    void Invalidate() { m_bValid = false; }
    void Reset() { m_strCached.Empty(); m_bValid = false; }
};

// Position update throttler - only update UI when position changes significantly
class CPositionUpdateThrottler
{
private:
    REFERENCE_TIME m_rtLastUpdate;
    REFERENCE_TIME m_rtThreshold;  // Minimum change to trigger update
    bool m_bInitialized;

public:
    CPositionUpdateThrottler(REFERENCE_TIME threshold = 10000000) // 1 second default
        : m_rtLastUpdate(0), m_rtThreshold(threshold), m_bInitialized(false) {}

    // Check if position update should be processed
    bool ShouldUpdate(REFERENCE_TIME rtCurrent);

    void Reset() { m_rtLastUpdate = 0; m_bInitialized = false; }
    void SetThreshold(REFERENCE_TIME threshold) { m_rtThreshold = threshold; }
};

// String operation optimizations
namespace StringOptimizer
{
    // Fast string concatenation for known sizes
    inline CString FastConcat(const CString& str1, const CString& str2)
    {
        CString result;
        result.Preallocate(str1.GetLength() + str2.GetLength() + 1);
        result = str1;
        result += str2;
        return result;
    }

    // Fast string concatenation with format
    CString FastFormat(LPCTSTR format, ...);
}

