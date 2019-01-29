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

    QString remote_addr = "tcp://114.116.16.108:49001";
    MainWindow w(0, "MainWindow", remote_addr);
    w.show();

    Hummingbird *hummingbird = new Hummingbird(0, "Hummingbird", remote_addr);
    hummingbird->start();

    a.exec();
    log4cplus::Logger::shutdown();
    return 0;
}
