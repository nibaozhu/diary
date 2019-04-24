#include "mainwindow.h"
#include <QApplication>

#include "Hummingbird.h"


log4cplus::Logger root = log4cplus::Logger::getRoot();

int main(int argc, char *argv[])
{
    log4cplus::initialize();
    log4cplus::helpers::LogLog::getLogLog()->setInternalDebugging(true);
    log4cplus::ConfigureAndWatchThread configureAndWatchThread(LOG4CPLUS_TEXT ("log4cplus.properties"));
    (void) configureAndWatchThread;

    QApplication a(argc, argv);
    QString remote_addr = "tcp://114.115.208.5:49001";
    QString sub_path = "benchmark";

    QString fileName("Hummingbird.ini");
    QSettings settings(fileName, QSettings::IniFormat);

    if (settings.contains("common/remote_addr"))
    {
        remote_addr = settings.value("common/remote_addr").toString();
    }
    else
    {
        settings.setValue("common/remote_addr", remote_addr);
    }

    if (settings.contains("common/sub_path"))
    {
        sub_path = settings.value("common/sub_path").toString();
    }
    else
    {
        settings.setValue("common/sub_path", sub_path);
    }

    MainWindow w(0, "MainWindow", remote_addr, sub_path);
    w.show();

    Hummingbird *hummingbird = new Hummingbird(0, "Hummingbird", remote_addr);
    hummingbird->start();

    a.exec();
    log4cplus::Logger::shutdown();
    return 0;
}
