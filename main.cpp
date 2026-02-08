#include <QApplication>
#include <QPalette>
#include <QCommandLineParser>
#include "mainwindow.h"
#include "terminalmonitor.h"

int main(int argc, char *argv[]) {
    // check for terminal mode flag before creating QApplication
    bool terminalMode = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--terminal" || arg == "-t") {
            terminalMode = true;
            break;
        }
    }
    
    if (terminalMode) {
        // run in terminal mode (no GUI)
        TerminalMonitor monitor;
        monitor.run();
        return 0;
    }
    
    // GUI mode
    QApplication app(argc, argv);
    
    // set application metadata
    app.setApplicationName("SysPulse");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("SysPulse");
    
    // command line parser for GUI options
    QCommandLineParser parser;
    parser.setApplicationDescription("SysPulse - System Resource Monitor");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption terminalOption(QStringList() << "t" << "terminal",
                                      "Run in terminal mode (text-based interface)");
    parser.addOption(terminalOption);
    
    parser.process(app);
    
    // dark mode setup
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    app.setPalette(darkPalette);
    
    // set stylesheet for better dark mode appearance
    app.setStyleSheet(
        "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"
        "QProgressBar { border: 2px solid grey; border-radius: 5px; text-align: center; }"
        "QProgressBar::chunk { background-color: #2a82da; width: 10px; margin: 0.5px; }"
    );
    
    MainWindow window;
    window.show();
    
    return app.exec();
}