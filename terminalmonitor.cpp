#include "terminalmonitor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <sys/statvfs.h>
#include <thread>
#include <chrono>
#include <cmath>

// ANSI color codes
const std::string TerminalMonitor::RESET = "\033[0m";
const std::string TerminalMonitor::RED = "\033[31m";
const std::string TerminalMonitor::GREEN = "\033[32m";
const std::string TerminalMonitor::YELLOW = "\033[33m";
const std::string TerminalMonitor::BLUE = "\033[34m";
const std::string TerminalMonitor::MAGENTA = "\033[35m";
const std::string TerminalMonitor::CYAN = "\033[36m";
const std::string TerminalMonitor::BOLD = "\033[1m";

TerminalMonitor::TerminalMonitor() 
    : lastTotalTime(0), lastIdleTime(0), lastRxBytes(0), lastTxBytes(0) {
}

void TerminalMonitor::clearScreen() {
    std::cout << "\033[2J\033[H"; // clear screen and move cursor to home; note to self: don't touch :) 
}

std::string TerminalMonitor::colorize(const std::string& text, const std::string& color) {
    return color + text + RESET;
}

std::string TerminalMonitor::getProgressBar(double percentage, int width) {
    int filled = static_cast<int>(percentage * width / 100.0);
    int empty = width - filled;
    
    std::string color;
    if (percentage < 50) color = GREEN;
    else if (percentage < 80) color = YELLOW;
    else color = RED;
    
    std::string bar = "[";
    bar += color;
    for (int i = 0; i < filled; i++) bar += "█";
    bar += RESET;
    for (int i = 0; i < empty; i++) bar += "░";
    bar += "]";
    
    return bar;
}

std::string TerminalMonitor::formatBytes(unsigned long long bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        unit++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return oss.str();
}

SystemStats TerminalMonitor::getStats() {
    SystemStats stats;
    
    // cpu usage
    std::ifstream cpuFile("/proc/stat");
    std::string line;
    std::getline(cpuFile, line);
    
    std::istringstream ss(line);
    std::string cpu;
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
    
    unsigned long long totalTime = user + nice + system + idle + iowait + irq + softirq;
    unsigned long long idleTime = idle + iowait;
    
    if (lastTotalTime != 0) {
        unsigned long long totalDiff = totalTime - lastTotalTime;
        unsigned long long idleDiff = idleTime - lastIdleTime;
        stats.cpu = 100.0 * (totalDiff - idleDiff) / totalDiff;
    } else {
        stats.cpu = 0.0;
    }
    
    lastTotalTime = totalTime;
    lastIdleTime = idleTime;
    
    // memory usage
    std::ifstream memFile("/proc/meminfo");
    unsigned long long memTotal = 0, memAvailable = 0;
    while (std::getline(memFile, line)) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream ss(line);
            std::string label;
            ss >> label >> memTotal;
        } else if (line.find("MemAvailable:") == 0) {
            std::istringstream ss(line);
            std::string label;
            ss >> label >> memAvailable;
        }
    }
    stats.memory = memTotal > 0 ? 100.0 * (memTotal - memAvailable) / memTotal : 0.0;
    
    // Disk Usage
    struct statvfs diskStat;
    if (statvfs("/", &diskStat) == 0) {
        unsigned long long total = diskStat.f_blocks * diskStat.f_frsize;
        unsigned long long available = diskStat.f_bavail * diskStat.f_frsize;
        stats.disk = 100.0 * (total - available) / total;
    } else {
        stats.disk = 0.0;
    }
    
    // uptime
    std::ifstream uptimeFile("/proc/uptime");
    double uptime;
    uptimeFile >> uptime;
    
    int days = uptime / 86400;
    int hours = (static_cast<int>(uptime) % 86400) / 3600;
    int minutes = (static_cast<int>(uptime) % 3600) / 60;
    
    std::ostringstream uptimeStream;
    uptimeStream << days << "d " << hours << "h " << minutes << "m";
    stats.uptime = uptimeStream.str();
    
    // process count
    stats.processes = 0;
    std::ifstream loadavg("/proc/loadavg");
    std::string dummy;
    int running, total;
    loadavg >> dummy >> dummy >> dummy >> running >> total;
    stats.processes = total;
    
    // temperature
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (tempFile.is_open()) {
        int temp;
        tempFile >> temp;
        stats.temperature = temp / 1000.0;
    } else {
        stats.temperature = -1.0;
    }
    
    // network stats
    std::ifstream netFile("/proc/net/dev");
    std::getline(netFile, line); // skip header
    std::getline(netFile, line); // same, skip header
    
    unsigned long long rxBytes = 0, txBytes = 0;
    while (std::getline(netFile, line)) {
        if (line.find("lo:") != std::string::npos) continue;
        
        std::istringstream ss(line);
        std::string interface;
        unsigned long long rx, tx, dummy;
        
        ss >> interface >> rx;
        for (int i = 0; i < 7; i++) ss >> dummy;
        ss >> tx;
        
        rxBytes += rx;
        txBytes += tx;
    }
    
    stats.netRxBytes = rxBytes;
    stats.netTxBytes = txBytes;
    
    if (lastRxBytes > 0) {
        stats.netRxSpeed = rxBytes - lastRxBytes;
        stats.netTxSpeed = txBytes - lastTxBytes;
    } else {
        stats.netRxSpeed = 0;
        stats.netTxSpeed = 0;
    }
    
    lastRxBytes = rxBytes;
    lastTxBytes = txBytes;
    
    return stats;
}

void TerminalMonitor::displayStats(const SystemStats& stats) {
    clearScreen();
    
    // header
    std::cout << colorize("╔════════════════════════════════════════════════════════════════════╗", CYAN) << std::endl;
    std::cout << colorize("║", CYAN) << colorize("                    SYSPULSE SYSTEM MONITOR                     ", BOLD) << colorize("║", CYAN) << std::endl;
    std::cout << colorize("╚════════════════════════════════════════════════════════════════════╝", CYAN) << std::endl;
    std::cout << std::endl;
    
    // system info
    std::cout << colorize("┌─ System Information ", CYAN) << std::string(48, '─') << std::endl;
    std::cout << "│ " << colorize("Uptime:", BOLD) << " " << stats.uptime 
              << "  │  " << colorize("Processes:", BOLD) << " " << stats.processes;
    if (stats.temperature > 0) {
        std::cout << "  │  " << colorize("CPU Temp:", BOLD) << " " 
                  << std::fixed << std::setprecision(1) << stats.temperature << "°C";
    }
    std::cout << std::endl;
    std::cout << "└" << std::string(68, '─') << std::endl;
    std::cout << std::endl;
    
    // cpu
    std::cout << colorize("┌─ CPU Usage ", BLUE) << std::string(56, '─') << std::endl;
    std::cout << "│ " << getProgressBar(stats.cpu, 50) 
              << " " << colorize(std::to_string(static_cast<int>(stats.cpu)) + "%", BOLD) << std::endl;
    std::cout << "└" << std::string(68, '─') << std::endl;
    std::cout << std::endl;
    
    // memory
    std::cout << colorize("┌─ Memory Usage ", GREEN) << std::string(53, '─') << std::endl;
    std::cout << "│ " << getProgressBar(stats.memory, 50) 
              << " " << colorize(std::to_string(static_cast<int>(stats.memory)) + "%", BOLD) << std::endl;
    std::cout << "└" << std::string(68, '─') << std::endl;
    std::cout << std::endl;
    
    // disk
    std::cout << colorize("┌─ Disk Usage (/) ", YELLOW) << std::string(51, '─') << std::endl;
    std::cout << "│ " << getProgressBar(stats.disk, 50) 
              << " " << colorize(std::to_string(static_cast<int>(stats.disk)) + "%", BOLD) << std::endl;
    std::cout << "└" << std::string(68, '─') << std::endl;
    std::cout << std::endl;
    
    // network
    std::cout << colorize("┌─ Network Statistics ", MAGENTA) << std::string(47, '─') << std::endl;
    std::cout << "│ " << colorize("Download Speed:", BOLD) << " " << formatBytes(stats.netRxSpeed) << "/s" 
              << "  │  " << colorize("Upload Speed:", BOLD) << " " << formatBytes(stats.netTxSpeed) << "/s" << std::endl;
    std::cout << "│ " << colorize("Total Downloaded:", BOLD) << " " << formatBytes(stats.netRxBytes)
              << "  │  " << colorize("Total Uploaded:", BOLD) << " " << formatBytes(stats.netTxBytes) << std::endl;
    std::cout << "└" << std::string(68, '─') << std::endl;
    std::cout << std::endl;
    
    // footer
    std::cout << colorize("Press Ctrl+C to exit", CYAN) << " | Refreshing every 2 seconds..." << std::endl;
}

void TerminalMonitor::run() {
    while (true) {
        SystemStats stats = getStats();
        displayStats(stats);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void TerminalMonitor::runOnce() {
    SystemStats stats = getStats();
    displayStats(stats);
}