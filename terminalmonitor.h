#ifndef TERMINALMONITOR_H
#define TERMINALMONITOR_H

#include <string>
#include <vector>

struct SystemStats {
    double cpu;
    double memory;
    double disk;
    std::string uptime;
    int processes;
    double temperature;
    unsigned long long netRxBytes;
    unsigned long long netTxBytes;
    unsigned long long netRxSpeed;
    unsigned long long netTxSpeed;
};

class TerminalMonitor {
public:
    TerminalMonitor();
    void run();
    void runOnce();
    
private:
    SystemStats getStats();
    void displayStats(const SystemStats& stats);
    void clearScreen();
    std::string getProgressBar(double percentage, int width = 40);
    std::string formatBytes(unsigned long long bytes);
    std::string colorize(const std::string& text, const std::string& color);
    
    // ANSI color codes
    static const std::string RESET;
    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string MAGENTA;
    static const std::string CYAN;
    static const std::string BOLD;
    
    unsigned long long lastTotalTime;
    unsigned long long lastIdleTime;
    unsigned long long lastRxBytes;
    unsigned long long lastTxBytes;
};

#endif