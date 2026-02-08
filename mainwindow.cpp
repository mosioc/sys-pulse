#include "mainwindow.h"
#include <QWidget>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QCheckBox>
#include <QSplitter>
#include <QBrush>
#include <QColor>
#include <fstream>
#include <sstream>
#include <sys/statvfs.h>
#include <dirent.h>
#include <pwd.h>
#include <signal.h>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), timePoint(0), lastTotalTime(0), lastIdleTime(0),
      autoRefresh(true), updateInterval(2000) {
    
    lastNetStats = {0, 0, 0, 0};
    setupUI();
    
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateStats);
    timer->start(updateInterval);
    
    updateStats();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    setWindowTitle("SysPulse - System Resource Monitor");
    resize(1000, 700);
    
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // top toolbar
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    exportButton = new QPushButton("Export Data");
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportData);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(exportButton);
    mainLayout->addLayout(toolbarLayout);
    
    // tab widget
    tabWidget = new QTabWidget();
    setupOverviewTab();
    setupProcessTab();
    setupNetworkTab();
    setupChartsTab();
    
    mainLayout->addWidget(tabWidget);
    setCentralWidget(centralWidget);
}

void MainWindow::setupOverviewTab() {
    QWidget *overviewWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(overviewWidget);
    
    // system info group
    QGroupBox *sysInfoGroup = new QGroupBox("System Information");
    QGridLayout *sysInfoLayout = new QGridLayout();
    
    uptimeLabel = new QLabel("Uptime: Calculating...");
    processCountLabel = new QLabel("Processes: 0");
    tempLabel = new QLabel("CPU Temp: N/A");
    
    sysInfoLayout->addWidget(uptimeLabel, 0, 0);
    sysInfoLayout->addWidget(processCountLabel, 0, 1);
    sysInfoLayout->addWidget(tempLabel, 0, 2);
    sysInfoGroup->setLayout(sysInfoLayout);
    
    // cpu section
    QGroupBox *cpuGroup = new QGroupBox("CPU Usage");
    QVBoxLayout *cpuLayout = new QVBoxLayout();
    cpuLabel = new QLabel("CPU: 0%");
    cpuDetailLabel = new QLabel("Cores: Calculating...");
    QFont labelFont = cpuLabel->font();
    labelFont.setPointSize(12);
    labelFont.setBold(true);
    cpuLabel->setFont(labelFont);
    cpuBar = new QProgressBar();
    cpuBar->setRange(0, 100);
    cpuBar->setTextVisible(true);
    cpuBar->setFormat("%p%");
    cpuLayout->addWidget(cpuLabel);
    cpuLayout->addWidget(cpuBar);
    cpuLayout->addWidget(cpuDetailLabel);
    cpuGroup->setLayout(cpuLayout);
    
    // memory section
    QGroupBox *memGroup = new QGroupBox("Memory Usage");
    QVBoxLayout *memLayout = new QVBoxLayout();
    memLabel = new QLabel("Memory: 0%");
    memDetailLabel = new QLabel("Used: 0 MB / Total: 0 MB");
    memLabel->setFont(labelFont);
    memBar = new QProgressBar();
    memBar->setRange(0, 100);
    memBar->setTextVisible(true);
    memBar->setFormat("%p%");
    memLayout->addWidget(memLabel);
    memLayout->addWidget(memBar);
    memLayout->addWidget(memDetailLabel);
    memGroup->setLayout(memLayout);
    
    // disk section
    QGroupBox *diskGroup = new QGroupBox("Disk Usage (/)");
    QVBoxLayout *diskLayout = new QVBoxLayout();
    diskLabel = new QLabel("Disk: 0%");
    diskDetailLabel = new QLabel("Used: 0 GB / Total: 0 GB");
    diskLabel->setFont(labelFont);
    diskBar = new QProgressBar();
    diskBar->setRange(0, 100);
    diskBar->setTextVisible(true);
    diskBar->setFormat("%p%");
    diskLayout->addWidget(diskLabel);
    diskLayout->addWidget(diskBar);
    diskLayout->addWidget(diskDetailLabel);
    diskGroup->setLayout(diskLayout);
    
    layout->addWidget(sysInfoGroup);
    layout->addWidget(cpuGroup);
    layout->addWidget(memGroup);
    layout->addWidget(diskGroup);
    layout->addStretch();
    
    tabWidget->addTab(overviewWidget, "Overview");
}

void MainWindow::setupProcessTab() {
    QWidget *processWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(processWidget);
    
    // control buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    refreshButton = new QPushButton("Refresh Now");
    killButton = new QPushButton("Kill Process");
    autoRefreshCheckbox = new QCheckBox("Auto-refresh");
    autoRefreshCheckbox->setChecked(true);
    
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshProcessList);
    connect(killButton, &QPushButton::clicked, this, &MainWindow::onProcessKillClicked);
    connect(autoRefreshCheckbox, &QCheckBox::toggled, this, &MainWindow::toggleAutoRefresh);
    
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(killButton);
    buttonLayout->addWidget(autoRefreshCheckbox);
    buttonLayout->addStretch();
    
    // process table
    processTable = new QTableWidget();
    processTable->setColumnCount(5);
    processTable->setHorizontalHeaderLabels({"PID", "Name", "CPU %", "Memory %", "User"});
    processTable->horizontalHeader()->setStretchLastSection(true);
    processTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    processTable->setSelectionMode(QAbstractItemView::SingleSelection);
    processTable->setSortingEnabled(true);
    processTable->setAlternatingRowColors(true);
    
    layout->addLayout(buttonLayout);
    layout->addWidget(processTable);
    
    tabWidget->addTab(processWidget, "Processes");
}

void MainWindow::setupNetworkTab() {
    QWidget *networkWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(networkWidget);
    
    QGroupBox *netGroup = new QGroupBox("Network Statistics");
    QGridLayout *netLayout = new QGridLayout();
    
    downloadLabel = new QLabel("Download Speed: 0 KB/s");
    uploadLabel = new QLabel("Upload Speed: 0 KB/s");
    totalDownloadLabel = new QLabel("Total Downloaded: 0 MB");
    totalUploadLabel = new QLabel("Total Uploaded: 0 MB");
    
    QFont netFont;
    netFont.setPointSize(11);
    downloadLabel->setFont(netFont);
    uploadLabel->setFont(netFont);
    
    networkBar = new QProgressBar();
    networkBar->setRange(0, 100);
    networkBar->setValue(0);
    networkBar->setFormat("Network Activity");
    
    netLayout->addWidget(downloadLabel, 0, 0);
    netLayout->addWidget(uploadLabel, 0, 1);
    netLayout->addWidget(totalDownloadLabel, 1, 0);
    netLayout->addWidget(totalUploadLabel, 1, 1);
    netLayout->addWidget(networkBar, 2, 0, 1, 2);
    
    netGroup->setLayout(netLayout);
    layout->addWidget(netGroup);
    layout->addStretch();
    
    tabWidget->addTab(networkWidget, "Network");
}

void MainWindow::setupChartsTab() {
    QWidget *chartsWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(chartsWidget);
    
    // cpu chart
    cpuSeries = new QLineSeries();
    cpuChart = new QChart();
    cpuChart->addSeries(cpuSeries);
    cpuChart->setTitle("CPU Usage History");
    cpuChart->createDefaultAxes();
    cpuChart->axes(Qt::Horizontal).first()->setRange(0, 60);
    cpuChart->axes(Qt::Vertical).first()->setRange(0, 100);
    cpuChart->legend()->hide();
    
    // dark theme for chart
    cpuChart->setBackgroundBrush(QBrush(QColor(53, 53, 53)));
    cpuChart->setTitleBrush(QBrush(Qt::white));
    cpuSeries->setColor(QColor(42, 130, 218));
    for (auto axis : cpuChart->axes()) {
        axis->setLabelsColor(Qt::white);
        axis->setGridLineColor(QColor(80, 80, 80));
    }
    
    cpuChartView = new QChartView(cpuChart);
    cpuChartView->setRenderHint(QPainter::Antialiasing);
    cpuChartView->setBackgroundBrush(QBrush(QColor(53, 53, 53)));
    
    // memory Chart
    memSeries = new QLineSeries();
    memChart = new QChart();
    memChart->addSeries(memSeries);
    memChart->setTitle("Memory Usage History");
    memChart->createDefaultAxes();
    memChart->axes(Qt::Horizontal).first()->setRange(0, 60);
    memChart->axes(Qt::Vertical).first()->setRange(0, 100);
    memChart->legend()->hide();
    
    // dark theme for chart
    memChart->setBackgroundBrush(QBrush(QColor(53, 53, 53)));
    memChart->setTitleBrush(QBrush(Qt::white));
    memSeries->setColor(QColor(76, 175, 80));
    for (auto axis : memChart->axes()) {
        axis->setLabelsColor(Qt::white);
        axis->setGridLineColor(QColor(80, 80, 80));
    }
    
    memChartView = new QChartView(memChart);
    memChartView->setRenderHint(QPainter::Antialiasing);
    memChartView->setBackgroundBrush(QBrush(QColor(53, 53, 53)));
    
    // network chart
    downloadSeries = new QLineSeries();
    uploadSeries = new QLineSeries();
    downloadSeries->setName("Download");
    uploadSeries->setName("Upload");
    downloadSeries->setColor(QColor(76, 175, 80));
    uploadSeries->setColor(QColor(244, 67, 54));
    
    networkChart = new QChart();
    networkChart->addSeries(downloadSeries);
    networkChart->addSeries(uploadSeries);
    networkChart->setTitle("Network Traffic");
    networkChart->createDefaultAxes();
    networkChart->axes(Qt::Horizontal).first()->setRange(0, 60);
    
    // dark theme for chart
    networkChart->setBackgroundBrush(QBrush(QColor(53, 53, 53)));
    networkChart->setTitleBrush(QBrush(Qt::white));
    networkChart->legend()->setLabelColor(Qt::white);
    for (auto axis : networkChart->axes()) {
        axis->setLabelsColor(Qt::white);
        axis->setGridLineColor(QColor(80, 80, 80));
    }
    
    networkChartView = new QChartView(networkChart);
    networkChartView->setRenderHint(QPainter::Antialiasing);
    networkChartView->setBackgroundBrush(QBrush(QColor(53, 53, 53)));
    
    layout->addWidget(cpuChartView);
    layout->addWidget(memChartView);
    layout->addWidget(networkChartView);
    
    tabWidget->addTab(chartsWidget, "Charts");
}

void MainWindow::updateStats() {
    // get metrics
    double cpu = getCPUUsage();
    double mem = getMemoryUsage();
    double disk = getDiskUsage();
    QString uptime = getUptime();
    int procCount = getProcessCount();
    double temp = getCPUTemperature();
    NetworkStats netStats = getNetworkStats();
    
    // update Overview tab
    cpuLabel->setText(QString("CPU: %1%").arg(cpu, 0, 'f', 1));
    cpuBar->setValue(static_cast<int>(cpu));
    
    // cpu details (read from /proc/cpuinfo)
    std::ifstream cpuinfo("/proc/cpuinfo");
    int cores = 0;
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("processor") == 0) cores++;
    }
    cpuDetailLabel->setText(QString("Cores: %1").arg(cores));
    
    // memory details
    std::ifstream meminfo("/proc/meminfo");
    unsigned long long memTotal = 0, memAvailable = 0;
    while (std::getline(meminfo, line)) {
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
    double memUsedMB = (memTotal - memAvailable) / 1024.0;
    double memTotalMB = memTotal / 1024.0;
    
    memLabel->setText(QString("Memory: %1%").arg(mem, 0, 'f', 1));
    memBar->setValue(static_cast<int>(mem));
    memDetailLabel->setText(QString("Used: %1 MB / Total: %2 MB")
                           .arg(memUsedMB, 0, 'f', 0)
                           .arg(memTotalMB, 0, 'f', 0));
    
    // disk details
    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        double totalGB = (stat.f_blocks * stat.f_frsize) / (1024.0 * 1024.0 * 1024.0);
        double usedGB = ((stat.f_blocks - stat.f_bavail) * stat.f_frsize) / (1024.0 * 1024.0 * 1024.0);
        diskDetailLabel->setText(QString("Used: %1 GB / Total: %2 GB")
                                .arg(usedGB, 0, 'f', 1)
                                .arg(totalGB, 0, 'f', 1));
    }
    
    diskLabel->setText(QString("Disk: %1%").arg(disk, 0, 'f', 1));
    diskBar->setValue(static_cast<int>(disk));
    
    uptimeLabel->setText(QString("Uptime: %1").arg(uptime));
    processCountLabel->setText(QString("Processes: %1").arg(procCount));
    tempLabel->setText(QString("CPU Temp: %1").arg(temp > 0 ? QString::number(temp, 'f', 1) + "°C" : "N/A"));
    
    // update Network tab
    if (lastNetStats.rxBytes > 0) {
        double downloadSpeed = (netStats.rxBytes - lastNetStats.rxBytes) / 1024.0 / (updateInterval / 1000.0);
        double uploadSpeed = (netStats.txBytes - lastNetStats.txBytes) / 1024.0 / (updateInterval / 1000.0);
        
        downloadLabel->setText(QString("Download Speed: %1 KB/s").arg(downloadSpeed, 0, 'f', 2));
        uploadLabel->setText(QString("Upload Speed: %1 KB/s").arg(uploadSpeed, 0, 'f', 2));
        totalDownloadLabel->setText(QString("Total Downloaded: %1 MB").arg(netStats.rxBytes / (1024.0 * 1024.0), 0, 'f', 2));
        totalUploadLabel->setText(QString("Total Uploaded: %1 MB").arg(netStats.txBytes / (1024.0 * 1024.0), 0, 'f', 2));
        
        int networkActivity = qMin(100, static_cast<int>((downloadSpeed + uploadSpeed) / 100));
        networkBar->setValue(networkActivity);
    }
    lastNetStats = netStats;
    
    // update charts
    cpuSeries->append(timePoint, cpu);
    memSeries->append(timePoint, mem);
    
    if (timePoint > 60) {
        cpuChart->axes(Qt::Horizontal).first()->setRange(timePoint - 60, timePoint);
        memChart->axes(Qt::Horizontal).first()->setRange(timePoint - 60, timePoint);
        networkChart->axes(Qt::Horizontal).first()->setRange(timePoint - 60, timePoint);
    }
    
    cpuHistory.append(cpu);
    memHistory.append(mem);
    
    timePoint++;
    
    // update process list if auto-refresh is enabled
    if (autoRefresh) {
        refreshProcessList();
    }
}

void MainWindow::refreshProcessList() {
    QVector<ProcessInfo> processes = getProcessList();
    
    processTable->setRowCount(processes.size());
    processTable->setSortingEnabled(false);
    
    for (int i = 0; i < processes.size(); i++) {
        processTable->setItem(i, 0, new QTableWidgetItem(QString::number(processes[i].pid)));
        processTable->setItem(i, 1, new QTableWidgetItem(processes[i].name));
        processTable->setItem(i, 2, new QTableWidgetItem(QString::number(processes[i].cpu, 'f', 1)));
        processTable->setItem(i, 3, new QTableWidgetItem(QString::number(processes[i].memory, 'f', 1)));
        processTable->setItem(i, 4, new QTableWidgetItem(processes[i].user));
    }
    
    processTable->setSortingEnabled(true);
}

void MainWindow::onProcessKillClicked() {
    QList<QTableWidgetItem*> selected = processTable->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "No Selection", "Please select a process to kill.");
        return;
    }
    
    int pid = processTable->item(selected[0]->row(), 0)->text().toInt();
    QString name = processTable->item(selected[0]->row(), 1)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Kill",
        QString("Are you sure you want to kill process '%1' (PID: %2)?").arg(name).arg(pid),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (kill(pid, SIGTERM) == 0) {
            QMessageBox::information(this, "Success", "Process terminated successfully.");
            refreshProcessList();
        } else {
            QMessageBox::critical(this, "Error", "Failed to kill process. You may need root privileges.");
        }
    }
}

void MainWindow::toggleAutoRefresh() {
    autoRefresh = autoRefreshCheckbox->isChecked();
}

void MainWindow::exportData() {
    QString fileName = QFileDialog::getSaveFileName(this, "Export Data", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for writing.");
        return;
    }
    
    QTextStream out(&file);
    out << "Time,CPU %,Memory %\n";
    
    for (int i = 0; i < cpuHistory.size(); i++) {
        out << i << "," << cpuHistory[i] << "," << memHistory[i] << "\n";
    }
    
    file.close();
    QMessageBox::information(this, "Success", "Data exported successfully!");
}

double MainWindow::getCPUUsage() {
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);
    
    std::istringstream ss(line);
    std::string cpu;
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    
    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
    
    unsigned long long totalTime = user + nice + system + idle + iowait + irq + softirq;
    unsigned long long idleTime = idle + iowait;
    
    if (lastTotalTime == 0) {
        lastTotalTime = totalTime;
        lastIdleTime = idleTime;
        return 0.0;
    }
    
    unsigned long long totalDiff = totalTime - lastTotalTime;
    unsigned long long idleDiff = idleTime - lastIdleTime;
    
    double usage = 100.0 * (totalDiff - idleDiff) / totalDiff;
    
    lastTotalTime = totalTime;
    lastIdleTime = idleTime;
    
    return usage;
}

double MainWindow::getMemoryUsage() {
    std::ifstream file("/proc/meminfo");
    std::string line;
    unsigned long long memTotal = 0, memAvailable = 0;
    
    while (std::getline(file, line)) {
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
    
    if (memTotal == 0) return 0.0;
    return 100.0 * (memTotal - memAvailable) / memTotal;
}

double MainWindow::getDiskUsage() {
    struct statvfs stat;
    if (statvfs("/", &stat) != 0) {
        return 0.0;
    }
    
    unsigned long long total = stat.f_blocks * stat.f_frsize;
    unsigned long long available = stat.f_bavail * stat.f_frsize;
    
    return 100.0 * (total - available) / total;
}

QString MainWindow::getUptime() {
    std::ifstream file("/proc/uptime");
    double uptime;
    file >> uptime;
    
    int days = uptime / 86400;
    int hours = (static_cast<int>(uptime) % 86400) / 3600;
    int minutes = (static_cast<int>(uptime) % 3600) / 60;
    
    return QString("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
}

int MainWindow::getProcessCount() {
    int count = 0;
    DIR* dir = opendir("/proc");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR) {
                int pid = atoi(entry->d_name);
                if (pid > 0) count++;
            }
        }
        closedir(dir);
    }
    return count;
}

double MainWindow::getCPUTemperature() {
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (tempFile.is_open()) {
        int temp;
        tempFile >> temp;
        return temp / 1000.0;
    }
    return -1.0;
}

NetworkStats MainWindow::getNetworkStats() {
    NetworkStats stats = {0, 0, 0, 0};
    std::ifstream file("/proc/net/dev");
    std::string line;
    
    // skip header lines
    std::getline(file, line);
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        if (line.find("lo:") != std::string::npos) continue; // Skip loopback
        
        std::istringstream ss(line);
        std::string interface;
        unsigned long long rxBytes, rxPackets, txBytes, txPackets;
        unsigned long long dummy;
        
        ss >> interface >> rxBytes >> rxPackets;
        for (int i = 0; i < 6; i++) ss >> dummy;
        ss >> txBytes >> txPackets;
        
        stats.rxBytes += rxBytes;
        stats.rxPackets += rxPackets;
        stats.txBytes += txBytes;
        stats.txPackets += txPackets;
    }
    
    return stats;
}

QVector<ProcessInfo> MainWindow::getProcessList() {
    QVector<ProcessInfo> processes;
    DIR* dir = opendir("/proc");
    
    if (!dir) return processes;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            int pid = atoi(entry->d_name);
            if (pid <= 0) continue;
            
            ProcessInfo info;
            info.pid = pid;
            
            // read process name
            std::string statPath = "/proc/" + std::string(entry->d_name) + "/stat";
            std::ifstream statFile(statPath);
            if (statFile.is_open()) {
                std::string line;
                std::getline(statFile, line);
                
                size_t start = line.find('(');
                size_t end = line.find(')');
                if (start != std::string::npos && end != std::string::npos) {
                    info.name = QString::fromStdString(line.substr(start + 1, end - start - 1));
                }
                
                // simple cpu calculation 
                info.cpu = 0.0;
            }
            
            // read memory usage
            std::string statusPath = "/proc/" + std::string(entry->d_name) + "/status";
            std::ifstream statusFile(statusPath);
            if (statusFile.is_open()) {
                std::string line;
                while (std::getline(statusFile, line)) {
                    if (line.find("VmRSS:") == 0) {
                        std::istringstream ss(line);
                        std::string label;
                        unsigned long long vmrss;
                        ss >> label >> vmrss;
                        
                        // get total memory
                        std::ifstream meminfo("/proc/meminfo");
                        std::string memline;
                        unsigned long long memTotal = 0;
                        while (std::getline(meminfo, memline)) {
                            if (memline.find("MemTotal:") == 0) {
                                std::istringstream mss(memline);
                                std::string mlabel;
                                mss >> mlabel >> memTotal;
                                break;
                            }
                        }
                        
                        if (memTotal > 0) {
                            info.memory = (vmrss * 100.0) / memTotal;
                        }
                        break;
                    }
                    if (line.find("Uid:") == 0) {
                        std::istringstream ss(line);
                        std::string label;
                        int uid;
                        ss >> label >> uid;
                        struct passwd* pw = getpwuid(uid);
                        info.user = pw ? QString(pw->pw_name) : QString::number(uid);
                    }
                }
            }
            
            processes.append(info);
        }
    }
    
    closedir(dir);
    return processes;
}