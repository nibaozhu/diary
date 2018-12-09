#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent, QString connectionName, QString remote_addr) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    remote_addr(remote_addr),
    hummingbirdDb(this, connectionName)
{
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(HummingbirdMonitor()));
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    log4cplus::threadCleanup();
    delete ui;
}

bool MainWindow::findFiles(const TblHummingbird &tblHummingbird, const QString &path, qint32 rowid)
{
    QDir dir(path);
    if (!dir.exists())
    {
        LOG4CPLUS_WARN(root, "Not exists! path: " << dir.absolutePath().constData());
        return false;
    }
    LOG4CPLUS_DEBUG(root, "path: " << dir.absolutePath().constData());

    dir.setFilter(QDir::Files | QDir::Dirs);
    dir.setSorting(QDir::Size | QDir::Reversed);

    int i = 0;
    QFileInfoList list = dir.entryInfoList();
    do {
        QFileInfo fileInfo = list.at(i++);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
        {
            continue;
        }

        bool rb = fileInfo.isDir();
        if (rb)
        {
            this->findFiles(tblHummingbird, fileInfo.filePath(), rowid);
        }

        rb = fileInfo.isFile();
        if (rb)
        {
            TblHummingbirdDetail tblHummingbirdDetail;
            tblHummingbirdDetail.f_uuid = tblHummingbird.f_uuid;
            tblHummingbirdDetail.f_name = fileInfo.absoluteFilePath();
            tblHummingbirdDetail.f_size = fileInfo.size();
            tblHummingbirdDetail.f_distinct = fileInfo.absoluteFilePath() + "|" + QString::number(fileInfo.size(), 10) + "|" + fileInfo.lastModified().toString() + "|";
            tblHummingbirdDetail.f_remotedir = fileInfo.absoluteFilePath().replace(QRegExp(":"), "$");

            this->hummingbirdDb.insertHummingbirdDetail(tblHummingbirdDetail, rowid);
        }
    } while (i < list.size());
    return true;
}

void MainWindow::HummingbirdMonitor()
{
    QList<TblHummingbirdDetail> tblHummingbirdDetailList;
    bool rb = this->hummingbirdDb.queryHummingbirdDetailList(tblHummingbirdDetailList, 16);
    if (!rb)
    {
        return;
    }

    ui->tableWidget->setRowCount(0);
    for (QList<TblHummingbirdDetail>::const_iterator it = tblHummingbirdDetailList.begin();
         it != tblHummingbirdDetailList.end(); it++)
    {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(it->f_name));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(HummingbirdDB::numberToHumanReadableValue(it->f_offset)));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(HummingbirdDB::numberToHumanReadableValue(it->f_size)));

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setValue(100 * it->f_offset / it->f_size);
        ui->tableWidget->setCellWidget(row, 3, progressBar);

        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString("%1/s").arg(HummingbirdDB::numberToHumanReadableValue(it->f_speed))));
        ui->tableWidget->setItem(row, 5, new QTableWidgetItem(QString::number(it->f_status)));
        ui->tableWidget->setItem(row, 6, new QTableWidgetItem(it->f_updated));
        ui->tableWidget->setItem(row, 7, new QTableWidgetItem(it->f_remark));
    }
}

void MainWindow::on_pushButton_clicked()
{
    bool rb;
    TblHummingbird tblHummingbird;
    qint32 rowid;

    tblHummingbird.f_uuid = QUuid::createUuid().toString();
    tblHummingbird.f_addr = this->remote_addr;
    QString local_path = QFileDialog::getExistingDirectory(this);
    if (local_path.isEmpty())
    {
        return;
    }
    tblHummingbird.f_localdir = local_path;
    this->hummingbirdDb.get_db().transaction();
    rb = this->hummingbirdDb.insertHummingbird(tblHummingbird, rowid);
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "insertHummingbird: " << tblHummingbird.f_uuid.toStdString().c_str());
        this->hummingbirdDb.get_db().rollback();
        return;
    }

    rb = this->findFiles(tblHummingbird, local_path, rowid);
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "findFiles: " << tblHummingbird.f_uuid.toStdString().c_str() << ", local_path: " << local_path.toStdString().c_str());
        this->hummingbirdDb.get_db().rollback();
        return;
    }
    this->hummingbirdDb.get_db().commit();
}
