/*
 * (C) 2024 MPC-HC Optimization
 *
 * Hardware Detection Implementation
 */

#include "stdafx.h"
#include "HardwareDetection.h"

#ifdef _MSC_VER
#include <intrin.h>
#define CPUID __cpuid
#else
// For non-MSVC compilers, define __cpuid
static void CPUID(int* cpuInfo, int function) {
    __asm__ __volatile__(
        "cpuid"
        : "=a"(cpuInfo[0]), "=b"(cpuInfo[1]), "=c"(cpuInfo[2]), "=d"(cpuInfo[3])
        : "a"(function)
    );
}
#endif

SystemCapabilities CHardwareDetection::s_caps = {};
bool CHardwareDetection::s_bInitialized = false;

void CHardwareDetection::Initialize()
{
    if (s_bInitialized) {
        return;
    }

    // Get system info
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    
    s_caps.dwNumberOfProcessors = si.dwNumberOfProcessors;
    s_caps.dwPageSize = si.dwPageSize;
    s_caps.dwProcessorType = si.dwProcessorType;
    s_caps.bIsSingleCore = (si.dwNumberOfProcessors == 1);

    // Detect CPU features
    DetectCPUFeatures();

    // Detect memory
    DetectMemoryInfo();

    // Determine tier
    s_caps.tier = DetermineTier();

    s_bInitialized = true;
}

void CHardwareDetection::DetectCPUFeatures()
{
    // Initialize to false
    s_caps.bHasSSE2 = false;
    s_caps.bHasSSE3 = false;
    s_caps.bHasSSE41 = false;
    s_caps.bHasAVX = false;
    s_caps.bIsSlowCPU = false;

    // Use CPUID instruction via inline assembly or intrinsic
    int cpuInfo[4] = {-1};
    
    // Get maximum supported function
    CPUID(cpuInfo, 0);
    int nIds = cpuInfo[0];

    if (nIds >= 1) {
        // Get feature flags
        CPUID(cpuInfo, 1);
        
        // Check for SSE2 (bit 26 of EDX)
        s_caps.bHasSSE2 = (cpuInfo[3] & (1 << 26)) != 0;
        
        // Check for SSE3 (bit 0 of ECX)
        s_caps.bHasSSE3 = (cpuInfo[2] & (1 << 0)) != 0;
        
        // Check for SSE4.1 (bit 19 of ECX)
        s_caps.bHasSSE41 = (cpuInfo[2] & (1 << 19)) != 0;
        
        // Check for AVX (bit 28 of ECX)
        s_caps.bHasAVX = (cpuInfo[2] & (1 << 28)) != 0;
    }

    // Rough CPU speed estimation (check if likely slow)
    // This is a heuristic - actual frequency detection requires more complex code
    // We'll use processor count and features as proxy
    s_caps.bIsSlowCPU = s_caps.bIsSingleCore && !s_caps.bHasSSE41;
}

void CHardwareDetection::DetectMemoryInfo()
{
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memStatus);

    s_caps.dwTotalPhys = memStatus.ullTotalPhys;
    s_caps.dwAvailPhys = memStatus.ullAvailPhys;

    // Check memory thresholds (2GB = 2 * 1024 * 1024 * 1024)
    const SIZE_T LOW_MEMORY_THRESHOLD = 2ULL * 1024 * 1024 * 1024;
    const SIZE_T VERY_LOW_MEMORY_THRESHOLD = 1ULL * 1024 * 1024 * 1024;

    s_caps.bIsLowMemory = (s_caps.dwTotalPhys < LOW_MEMORY_THRESHOLD);
    s_caps.bIsVeryLowMemory = (s_caps.dwTotalPhys < VERY_LOW_MEMORY_THRESHOLD);
}

HardwareTier CHardwareDetection::DetermineTier()
{
    // Low-end criteria:
    // - Very low memory (< 1GB) OR
    // - Low memory (< 2GB) AND single core OR
    // - Single core AND no SSE4.1
    if (s_caps.bIsVeryLowMemory ||
        (s_caps.bIsLowMemory && s_caps.bIsSingleCore) ||
        (s_caps.bIsSingleCore && !s_caps.bHasSSE41)) {
        return HardwareTier::LOW_END;
    }

    // Mid-range criteria:
    // - Low memory (< 2GB) OR
    // - 2 cores or less
    if (s_caps.bIsLowMemory || s_caps.dwNumberOfProcessors <= 2) {
        return HardwareTier::MID_RANGE;
    }

    // Otherwise high-end
    return HardwareTier::HIGH_END;
}

const SystemCapabilities& CHardwareDetection::GetCapabilities()
{
    if (!s_bInitialized) {
        Initialize();
    }
    return s_caps;
}

bool CHardwareDetection::IsLowEndHardware()
{
    return GetCapabilities().tier == HardwareTier::LOW_END;
}

int CHardwareDetection::GetRecommendedStreamPosPollerInterval()
{
    const auto& caps = GetCapabilities();
    
    if (caps.tier == HardwareTier::LOW_END) {
        return 200;  // Reduce polling frequency on low-end hardware
    } else if (caps.tier == HardwareTier::MID_RANGE) {
        return 150;
    }
    
    return 100;  // Default
}

int CHardwareDetection::GetRecommendedTimerInterval()
{
    const auto& caps = GetCapabilities();
    
    if (caps.tier == HardwareTier::LOW_END) {
        return 2000;  // Reduce UI update frequency
    } else if (caps.tier == HardwareTier::MID_RANGE) {
        return 1000;
    }
    
    return 500;  // Default
}

bool CHardwareDetection::ShouldDisableSeekPreview()
{
    return IsLowEndHardware();
}

bool CHardwareDetection::ShouldDisableThumbnails()
{
    return IsLowEndHardware() || GetCapabilities().bIsVeryLowMemory;
}

bool CHardwareDetection::ShouldReduceUIUpdates()
{
    return IsLowEndHardware();
}

bool CHardwareDetection::ShouldUseLightweightRenderer()
{
    const auto& caps = GetCapabilities();
    return caps.tier == HardwareTier::LOW_END || caps.bIsVeryLowMemory;
}

DWORD CHardwareDetection::GetRecommendedPriorityClass()
{
    const auto& caps = GetCapabilities();
    
    // On low-end hardware, use normal priority to avoid starving the system
    if (caps.tier == HardwareTier::LOW_END) {
        return NORMAL_PRIORITY_CLASS;
    }
    
    // On mid-range, slightly elevated
    if (caps.tier == HardwareTier::MID_RANGE) {
        return ABOVE_NORMAL_PRIORITY_CLASS;
    }
    
    // High-end can use higher priority
    return HIGH_PRIORITY_CLASS;
}

