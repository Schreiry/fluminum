#pragma once
#include "Common.h"

// --- Global SIMD Flags ---
extern bool has_avx_global;
extern bool has_sse2_global;

// --- NEW: Global Tile Size ---
// This will be set by the auto-tuner on startup.
extern int G_OPTIMAL_TILE_SIZE;

// --- System Information ---
SystemMemoryInfo getSystemMemoryInfo();
unsigned int getCpuCoreCount();
ProcessMemoryInfo getProcessMemoryUsage();

// --- Performance Counter ---
void initializePerformanceCounter();
extern LARGE_INTEGER g_performanceFrequency;

// --- SIMD Support ---
void check_simd_support();

// --- NEW: Tiling Auto-Tuner ---
void autoTuneTileSize();

// --- Memory Estimation ---
unsigned long long estimateStrassenMemoryMB(int n_padded);
unsigned long long estimateComparisonMemoryMB(int n_padded);

// --- Process Management ---
void LaunchMonitorProcess();