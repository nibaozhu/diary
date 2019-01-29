#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QProgressBar>
#include <QList>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include "Hummingbirddb.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, QString connectionName = "",
                        QString remote_addr = "");
    ~MainWindow();

    bool findFiles(const TblHummingbird &tblHummingbird, const QString &path, qint32 rowid);

private slots:
    void HummingbirdMonitor();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QString remote_addr;
    HummingbirdDB hummingbirdDb;
};

#endif // MAINWINDOW_H
