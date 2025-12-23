/*
 * (C) 2024 MPC-HC Optimization
 *
 * Hardware Detection Utility for Low-End System Optimization
 * Detects system capabilities and adjusts settings accordingly
 */

#pragma once

#include <windows.h>
#include <psapi.h>

// Hardware performance tier
enum class HardwareTier {
    HIGH_END,      // Modern system with good CPU/RAM
    MID_RANGE,     // Average system
    LOW_END        // "Potato" computer - minimal resources
};

// System capabilities
struct SystemCapabilities {
    HardwareTier tier;
    DWORD dwNumberOfProcessors;
    DWORD dwPageSize;
    SIZE_T dwTotalPhys;          // Total physical RAM in bytes
    SIZE_T dwAvailPhys;          // Available physical RAM in bytes
    DWORD dwProcessorType;
    bool bHasSSE2;
    bool bHasSSE3;
    bool bHasSSE41;
    bool bHasAVX;
    bool bIsLowMemory;            // Less than 2GB RAM
    bool bIsVeryLowMemory;        // Less than 1GB RAM
    bool bIsSingleCore;           // Single core CPU
    bool bIsSlowCPU;              // CPU frequency < 1.5GHz (rough estimate)
};

class CHardwareDetection
{
private:
    static SystemCapabilities s_caps;
    static bool s_bInitialized;

    static void DetectCPUFeatures();
    static void DetectMemoryInfo();
    static HardwareTier DetermineTier();

public:
    // Initialize hardware detection (call once at startup)
    static void Initialize();

    // Get system capabilities
    static const SystemCapabilities& GetCapabilities();

    // Check if running on low-end hardware
    static bool IsLowEndHardware();

    // Get recommended settings for current hardware
    static int GetRecommendedStreamPosPollerInterval();
    static int GetRecommendedTimerInterval();
    static bool ShouldDisableSeekPreview();
    static bool ShouldDisableThumbnails();
    static bool ShouldReduceUIUpdates();
    static bool ShouldUseLightweightRenderer();
    static DWORD GetRecommendedPriorityClass();
};

