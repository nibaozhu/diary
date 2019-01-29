#ifndef SNAILDB_H
#define SNAILDB_H

#include <QObject>
#include <QList>
#include <QUuid>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/version.h>

#include "Hummingbirdp.pb.h"
extern log4cplus::Logger root;

class TblHummingbird
{
public:
    QString f_uuid;
    qint32 f_status;
    QString f_addr;
    QString f_localdir;
    QString f_remotedir;
    QString f_created;
    QString f_updated;
    QString f_remark;

    quint64 pending;
    quint64 uploading;
    quint64 speed;
    quint64 uploaded;
    quint64 uploaded_offset;
    quint64 all_size;
};

class TblHummingbirdDetail
{
public:
    QString f_uuid;
    QString f_name;
    QString f_distinct;
    QString f_remotedir;
    quint64 f_offset;
    quint64 f_size;
    quint64 f_speed; // Bps
    qint32 f_status;
    QString f_created;
    QString f_updated;
    QString f_remark;
};

class HummingbirdDB : public QObject
{
    Q_OBJECT
public:
    explicit HummingbirdDB(QObject *parent = 0, QString connectionName = "");
    ~HummingbirdDB();

    void initLocalDb();

    bool insertHummingbird(TblHummingbird &tblHummingbird, qint32 &rowid);
    bool queryHummingbird(TblHummingbird &tblHummingbird, qint32 rowid);

    bool insertHummingbirdDetail(TblHummingbirdDetail &tblHummingbirdDetail, qint32 &rowid);
    bool queryHummingbirdDetailList(QList<TblHummingbirdDetail> &tblHummingbirdDetailList, int nitems);
    bool queryHummingbirdDetailList(QList<TblHummingbirdDetail> &tblHummingbirdDetailList, quint64 max_msg_size,
                              QHash<QString, Hummingbirdp::TransferRequest_Fragment> &hashFragment,
                              QString remote_addr);

    bool updateHummingbird(QString uuid, qint32 status);
    bool updateHummingbirdDetail(TblHummingbirdDetail &tblHummingbirdDetail);
    bool updateHummingbirdDetail(const Hummingbirdp::TransferRequest &transferRequest,
                           const Hummingbirdp::TransferRespond &transferRespond,
                           QHash<QString, Hummingbirdp::TransferRequest_Fragment> &hashFragment,
                           quint64 &f_speed);

    bool resetHummingbirdDetail();
    static QString numberToHumanReadableValue(quint64 num);

    QSqlDatabase &get_db();
private:
    QSqlDatabase db;
    QString connectionName;

    bool checkObjectExist(QString name, QString type = "table");

signals:

public slots:
};

#endif // SNAILDB_H
