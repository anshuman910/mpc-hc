/*
 * (C) 2024 MPC-HC Performance Optimization
 *
 * Performance optimization implementation
 */

#include "stdafx.h"
#include "PerformanceOptimizer.h"
#include "DSUtil.h"
#include <stdarg.h>

CString CTimeFormatCache::GetFormattedTime(REFERENCE_TIME rt, bool showHours, 
                                           CString (*formatFunc)(const REFERENCE_TIME&))
{
    // Use default format function if not provided
    if (!formatFunc) {
        formatFunc = showHours ? ReftimeToString2 : ReftimeToString3;
    }

    // Check if we can use cached value
    if (m_bValid && m_rtCached == rt && m_bShowHours == showHours) {
        return m_strCached;
    }

    // Format time (expensive operation)
    m_strCached = formatFunc(rt);
    m_rtCached = rt;
    m_bShowHours = showHours;
    m_bValid = true;

    return m_strCached;
}

void CWindowTitleCache::SetWindowTextIfChanged(HWND hWnd, const CString& strNew)
{
    if (!hWnd) {
        return;
    }

    // Only update if title actually changed
    if (!m_bValid || m_strCached != strNew) {
        ::SetWindowText(hWnd, strNew);
        m_strCached = strNew;
        m_bValid = true;
    }
}

bool CPositionUpdateThrottler::ShouldUpdate(REFERENCE_TIME rtCurrent)
{
    if (!m_bInitialized) {
        m_rtLastUpdate = rtCurrent;
        m_bInitialized = true;
        return true;
    }

    REFERENCE_TIME diff = (rtCurrent > m_rtLastUpdate) ? 
                          (rtCurrent - m_rtLastUpdate) : 
                          (m_rtLastUpdate - rtCurrent);

    if (diff >= m_rtThreshold) {
        m_rtLastUpdate = rtCurrent;
        return true;
    }

    return false;
}

CString StringOptimizer::FastFormat(LPCTSTR format, ...)
{
    va_list args;
    va_start(args, format);
    
    // Try to estimate size first
    int nLen = _vsctprintf(format, args);
    va_end(args);
    
    if (nLen < 0) {
        return CString();
    }
    
    CString str;
    str.GetBufferSetLength(nLen + 1);
    
    va_start(args, format);
    _vstprintf_s(str.GetBuffer(), nLen + 1, format, args);
    va_end(args);
    
    str.ReleaseBuffer();
    return str;
}

