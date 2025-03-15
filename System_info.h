// system_info.h
#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <string>

extern std::string cpuName;
extern int cpuCores;
extern int cpuThreads;
extern const int reservedThreadsForOS;

void getCPUInfo();
int calculateOptimalThreads(double complexityA, double complexityB);

#endif // SYSTEM_INFO_H