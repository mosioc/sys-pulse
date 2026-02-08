#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QVector>

using namespace QtCharts;

struct ProcessInfo {
    int pid;
    QString name;
    double cpu;
    double memory;
    QString user;
};

struct NetworkStats {
    unsigned long long rxBytes;
    unsigned long long txBytes;
    unsigned long long rxPackets;
    unsigned long long txPackets;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateStats();
    void refreshProcessList();
    void onProcessKillClicked();
    void toggleAutoRefresh();
    void exportData();

private:
    void setupUI();
    void setupOverviewTab();
    void setupProcessTab();
    void setupNetworkTab();
    void setupChartsTab();
    
    // system metrics
    double getCPUUsage();
    double getMemoryUsage();
    double getDiskUsage();
    QString getUptime();
    int getProcessCount();
    double getCPUTemperature();
    NetworkStats getNetworkStats();
    QVector<ProcessInfo> getProcessList();
    
    // ui components - overview tab
    QProgressBar *cpuBar;
    QProgressBar *memBar;
    QProgressBar *diskBar;
    QLabel *cpuLabel;
    QLabel *memLabel;
    QLabel *diskLabel;
    QLabel *uptimeLabel;
    QLabel *processCountLabel;
    QLabel *tempLabel;
    QLabel *cpuDetailLabel;
    QLabel *memDetailLabel;
    QLabel *diskDetailLabel;
    
    // ui components - process tab
    QTableWidget *processTable;
    QPushButton *killButton;
    QPushButton *refreshButton;
    QCheckBox *autoRefreshCheckbox;
    
    // ui components - network tab
    QLabel *downloadLabel;
    QLabel *uploadLabel;
    QLabel *totalDownloadLabel;
    QLabel *totalUploadLabel;
    QProgressBar *networkBar;
    
    // ui components - charts tab
    QChart *cpuChart;
    QChart *memChart;
    QChart *networkChart;
    QLineSeries *cpuSeries;
    QLineSeries *memSeries;
    QLineSeries *downloadSeries;
    QLineSeries *uploadSeries;
    QChartView *cpuChartView;
    QChartView *memChartView;
    QChartView *networkChartView;
    
    // main components
    QTabWidget *tabWidget;
    QTimer *timer;
    QPushButton *exportButton;
    
    // data tracking
    int timePoint;
    unsigned long long lastTotalTime;
    unsigned long long lastIdleTime;
    NetworkStats lastNetStats;
    QVector<double> cpuHistory;
    QVector<double> memHistory;
    
    // settings
    bool autoRefresh;
    int updateInterval;
};

#endif