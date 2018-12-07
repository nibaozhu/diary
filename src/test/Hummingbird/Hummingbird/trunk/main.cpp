#include "mainwindow.h"
#include <QApplication>

#include "Hummingbird.h"


log4cplus::Logger root = log4cplus::Logger::getRoot();
log4cplus::ConfigureAndWatchThread configureAndWatchThread(LOG4CPLUS_TEXT ("log4cplus.properties"));

int main(int argc, char *argv[])
{
    log4cplus::initialize();
    QApplication a(argc, argv);

    QString remote_addr = "tcp://10.66.1.66:7789";
    MainWindow w(0, "MainWindow", remote_addr);
    w.show();

    Hummingbird *hummingbird = new Hummingbird(0, "Hummingbird", remote_addr);
    Hummingbird->start();

    a.exec();
    log4cplus::Logger::shutdown();
    return 0;
}
