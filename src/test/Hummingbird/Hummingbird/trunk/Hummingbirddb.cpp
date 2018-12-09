#include "Hummingbirddb.h"


HummingbirdDB::HummingbirdDB(QObject *parent, QString connectionName) : QObject(parent),
    connectionName(connectionName)
{
    this->db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    this->db.setDatabaseName("Hummingbird.db");

    bool isopen = this->db.open();
    if (!isopen)
    {
        QSqlError sqlError = this->db.lastError();
        LOG4CPLUS_ERROR(root, "lastError: " << sqlError.text().toStdString().c_str());
    }

    LOG4CPLUS_INFO(root, "connectOptions: " << this->db.connectOptions().toStdString().c_str());
    this->initLocalDb();
}

HummingbirdDB::~HummingbirdDB()
{
    this->db.close();
}

void HummingbirdDB::initLocalDb()
{
    bool rb = this->checkObjectExist("t_Hummingbird");
    if (!rb)
    {
        QSqlQuery sqlQuery(this->db);
        QString query;
        query = "CREATE TABLE [t_Hummingbird](";
        query+= "[f_uuid] TEXT PRIMARY KEY,";
        query+= "[f_addr] TEXT NOT NULL,";
        query+= "[f_localdir] TEXT NOT NULL,";
        query+= "[f_remotedir] TEXT,";
        query+= "[f_status] INTEGER DEFAULT 0,";
        query+= "[f_created] TIMESTAMP,";
        query+= "[f_updated] TIMESTAMP,";
        query+= "[f_remark] TEXT)";
        rb = sqlQuery.exec(query);
        if (!rb)
        {
            LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                            << ", query: " << query.toStdString().c_str());
            return;
        }        
        LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str());
    }

    rb = this->checkObjectExist("t_Hummingbird_detail");
    if (!rb)
    {
        QSqlQuery sqlQuery(this->db);
        QString query;
        query = "CREATE TABLE [t_Hummingbird_detail](";
        query+= "[f_uuid] TEXT REFERENCES [t_Hummingbird]([f_uuid]),";
        query+= "[f_name] TEXT NOT NULL,";
        query+= "[f_distinct] TEXT NOT NULL,";
        query+= "[f_remotedir] TEXT,";
        query+= "[f_offset] BIGINT DEFAULT 0,";
        query+= "[f_size] BIGINT DEFAULT 0,";
        query+= "[f_speed] BIGINT DEFAULT 0,";
        query+= "[f_status] INTEGER DEFAULT 0,";
        query+= "[f_created] TIMESTAMP,";
        query+= "[f_updated] TIMESTAMP,";
        query+= "[f_remark] TEXT)";
        rb = sqlQuery.exec(query);
        if (!rb)
        {
            LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                            << ", query: " << query.toStdString().c_str());
            return;
        }
        LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str());
    }

    rb = this->checkObjectExist("idx_uuid_name", "index");
    if (!rb)
    {
        QSqlQuery sqlQuery(this->db);
        QString query;
        query = "CREATE UNIQUE INDEX [idx_uuid_name] ";
        query+= "ON [t_Hummingbird_detail](";
        query+= "[f_uuid],";
        query+= "[f_name])";
        rb = sqlQuery.exec(query);
        if (!rb)
        {
            LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                            << ", query: " << query.toStdString().c_str());
            return;
        }
        LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str());
    }

    return;
}

bool HummingbirdDB::checkObjectExist(QString name, QString type)
{
    QString query = QString("SELECT * FROM sqlite_master WHERE type='%1' AND name='%2'").arg(type).arg(name);
    QSqlQuery sqlQuery(query, this->db);
    bool rb = sqlQuery.exec();
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                        << ", query: " << query.toStdString().c_str());
        return rb;
    }

    rb = sqlQuery.first();
    LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str() << ", rb: " << rb);
    return rb;
}

bool HummingbirdDB::insertHummingbird(TblHummingbird &tblHummingbird, qint32 &rowid)
{
    QString query = QString("INSERT INTO t_Hummingbird(f_uuid, f_addr, f_localdir, f_remotedir, f_created) VALUES('%1','%2','%3','%4',DATETIME('NOW','LOCALTIME'))")
            .arg(tblHummingbird.f_uuid)
            .arg(tblHummingbird.f_addr)
            .arg(tblHummingbird.f_localdir)
            .arg(tblHummingbird.f_remotedir);
    QSqlQuery sqlQuery(this->db);
    bool rb = sqlQuery.exec(query);
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                        << ", query: " << query.toStdString().c_str());
        return rb;
    }

    QString queryLast = QString("SELECT LAST_INSERT_ROWID()");
    QSqlQuery sqlQueryLast(this->db);
    sqlQueryLast.exec(queryLast);
    sqlQueryLast.first();
    rowid = sqlQueryLast.value(0).toInt();

    LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str() << ", rb: " << rb << ", numRowsAffected: " << sqlQuery.numRowsAffected() << ", rowid: " << rowid);
    return rb;
}

bool HummingbirdDB::queryHummingbird(TblHummingbird &tblHummingbird, qint32 rowid)
{
    QString query = QString("SELECT t1.f_status,COUNT(*) counts,SUM(t1.f_offset) sum_offset,SUM(t1.f_size) sum_size,SUM(t1.f_speed) sum_speed FROM t_Hummingbird_detail AS t1 INNER JOIN t_Hummingbird AS t2 ON t1.f_uuid = t2.f_uuid AND t2.rowid = '%1' GROUP BY t1.f_status")
            .arg(rowid);
    QSqlQuery sqlQuery(query, this->db);
    bool rb = sqlQuery.exec();
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                        << ", query: " << query.toStdString().c_str());
        return rb;
    }

    while (sqlQuery.next())
    {
        tblHummingbird.f_status = sqlQuery.value("f_status").toInt();
        tblHummingbird.all_size += sqlQuery.value("sum_size").toLongLong();

        if (tblHummingbird.f_status == 0) // NOTE: pending
        {
            tblHummingbird.pending = sqlQuery.value("counts").toInt();
        }
        else if (tblHummingbird.f_status == 1) // NOTE: processing
        {
            tblHummingbird.uploading = sqlQuery.value("counts").toInt();
            tblHummingbird.speed = sqlQuery.value("sum_speed").toLongLong();
        }
        else if (tblHummingbird.f_status == 2) // NOTE: finished
        {
            tblHummingbird.uploaded = sqlQuery.value("counts").toInt();
            tblHummingbird.uploaded_offset = sqlQuery.value("sum_offset").toLongLong();
        }
        else
        {
            LOG4CPLUS_ERROR(root, "tblHummingbird.f_status: " << tblHummingbird.f_status);
        }
    }

    LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str() << ", rb: " << rb);
    return rb;
}

bool HummingbirdDB::insertHummingbirdDetail(TblHummingbirdDetail &tblHummingbirdDetail, qint32 &rowid)
{
    tblHummingbirdDetail.f_name = tblHummingbirdDetail.f_name.replace("'", "''");
    tblHummingbirdDetail.f_distinct = tblHummingbirdDetail.f_distinct.replace("'", "''");
    tblHummingbirdDetail.f_remotedir = tblHummingbirdDetail.f_remotedir.replace("'", "''");

    QString query = QString("INSERT INTO t_Hummingbird_detail(f_uuid,f_name,f_distinct,f_remotedir,f_size,f_created) VALUES('%1','%2','%3','%4','%5',DATETIME('NOW','LOCALTIME'))")
            .arg(tblHummingbirdDetail.f_uuid)
            .arg(tblHummingbirdDetail.f_name)
            .arg(tblHummingbirdDetail.f_distinct)
            .arg(tblHummingbirdDetail.f_remotedir)
            .arg(tblHummingbirdDetail.f_size);
    QSqlQuery sqlQuery(this->db);
    bool rb = sqlQuery.exec(query);
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                        << ", query: " << query.toStdString().c_str());
        return rb;
    }

    QString queryLast = QString("SELECT LAST_INSERT_ROWID()");
    QSqlQuery sqlQueryLast(this->db);
    sqlQueryLast.exec(queryLast);
    sqlQueryLast.first();
    rowid = sqlQueryLast.value(0).toInt();

    LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str() << ", rb: " << rb << ", numRowsAffected: " << sqlQuery.numRowsAffected() << ", rowid: " << rowid);
    return rb;
}

bool HummingbirdDB::queryHummingbirdDetailList(QList<TblHummingbirdDetail> &tblHummingbirdDetailList, int nitems)
{
    QString query = QString("SELECT * FROM t_Hummingbird_detail where f_status=1 LIMIT %1").arg(nitems);
    QSqlQuery sqlQuery(query, this->db);
    bool rb = sqlQuery.exec();
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                        << ", query: " << query.toStdString().c_str());
        return rb;
    }

    while (sqlQuery.next())
    {
        TblHummingbirdDetail tblHummingbirdDetail;
        tblHummingbirdDetail.f_uuid = sqlQuery.value("f_uuid").toString();
        tblHummingbirdDetail.f_name = sqlQuery.value("f_name").toString();
        tblHummingbirdDetail.f_distinct = sqlQuery.value("f_distinct").toString();
        tblHummingbirdDetail.f_remotedir = sqlQuery.value("f_remotedir").toString();
        tblHummingbirdDetail.f_offset = sqlQuery.value("f_offset").toULongLong();
        tblHummingbirdDetail.f_size = sqlQuery.value("f_size").toULongLong();
        tblHummingbirdDetail.f_speed = sqlQuery.value("f_speed").toULongLong();
        tblHummingbirdDetail.f_status = sqlQuery.value("f_status").toInt();
        tblHummingbirdDetail.f_created = sqlQuery.value("f_created").toString();
        tblHummingbirdDetail.f_updated = sqlQuery.value("f_updated").toString();
        tblHummingbirdDetail.f_remark = sqlQuery.value("f_remark").toString();

        tblHummingbirdDetailList.push_back(tblHummingbirdDetail);
    }
    LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str() << ", rb: " << rb);
    return rb;
}

bool HummingbirdDB::queryHummingbirdDetailList(QList<TblHummingbirdDetail> &tblHummingbirdDetailList, quint64 max_msg_size,
                                   QHash<QString, Hummingbirdp::TransferRequest_Fragment> &hashFragment,
                                   QString remote_addr)
{
    QString query = QString("SELECT t1.* FROM t_Hummingbird_detail AS t1 INNER JOIN t_Hummingbird AS t2 ON t1.f_status=0 AND t2.f_status=0 AND (t1.f_offset<t1.f_size OR (t1.f_offset=0 AND t1.f_size=0)) AND t1.f_uuid=t2.f_uuid AND t2.f_addr='%1'")
            .arg(remote_addr);
    QSqlQuery sqlQuery(query, this->db);
    bool rb = sqlQuery.exec();
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                        << ", query: " << query.toStdString().c_str());
        return rb;
    }

    this->db.transaction();
    quint64 msg_size = 0;
    while (sqlQuery.next())
    {
        TblHummingbirdDetail tblHummingbirdDetail;
        tblHummingbirdDetail.f_uuid = sqlQuery.value("f_uuid").toString();
        tblHummingbirdDetail.f_name = sqlQuery.value("f_name").toString();
        tblHummingbirdDetail.f_distinct = sqlQuery.value("f_distinct").toString();
        tblHummingbirdDetail.f_remotedir = sqlQuery.value("f_remotedir").toString();
        tblHummingbirdDetail.f_offset = sqlQuery.value("f_offset").toULongLong();
        tblHummingbirdDetail.f_size = sqlQuery.value("f_size").toULongLong();
        tblHummingbirdDetail.f_speed = sqlQuery.value("f_speed").toULongLong();
        tblHummingbirdDetail.f_status = sqlQuery.value("f_status").toInt();
        tblHummingbirdDetail.f_created = sqlQuery.value("f_created").toString();
        tblHummingbirdDetail.f_updated = sqlQuery.value("f_updated").toString();
        tblHummingbirdDetail.f_remark = sqlQuery.value("f_remark").toString();

        QHash<QString, Hummingbirdp::TransferRequest_Fragment>::iterator it = hashFragment.find(tblHummingbirdDetail.f_name);
        if (it != hashFragment.end())
        {
            // FIXME: avoid!!
            // LOG4CPLUS_WARN(root, "has been sent, waiting respond [" << tblHummingbirdDetail.f_name.toStdString().c_str() << "]");
            continue;
        }

        if (tblHummingbirdDetail.f_offset + max_msg_size <= tblHummingbirdDetail.f_size)
        {
            msg_size += max_msg_size; // NOTE: only one file
        }
        else
        {
            /**
              * NOTE: for some small files, merge them to a fragment for high performance
              */
            msg_size += tblHummingbirdDetail.f_size - tblHummingbirdDetail.f_offset; // NOTE: the last fragment of a file
        }

        /**
         * TODO: refresh offset..
         * */
        TblHummingbirdDetail tblHummingbirdDetailCurrent;
        tblHummingbirdDetailCurrent.f_offset = tblHummingbirdDetail.f_offset;
        tblHummingbirdDetailCurrent.f_speed = tblHummingbirdDetail.f_speed;
        tblHummingbirdDetailCurrent.f_status = 1; // NOTE: processing
        tblHummingbirdDetailCurrent.f_name = tblHummingbirdDetail.f_name;
        tblHummingbirdDetailCurrent.f_remotedir = tblHummingbirdDetail.f_remotedir;
        this->updateHummingbirdDetail(tblHummingbirdDetailCurrent);

        tblHummingbirdDetailList.push_back(tblHummingbirdDetail);
        if (msg_size >= max_msg_size)
        {
            LOG4CPLUS_DEBUG(root, "reached max msg(" << this->numberToHumanReadableValue(msg_size).toStdString().c_str() << "/" << this->numberToHumanReadableValue(max_msg_size).toStdString().c_str() << ")");
            break;
        }
    }
    this->db.commit();

    LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str() << ", rb: " << rb);
    return rb;
}

bool HummingbirdDB::updateHummingbird(QString uuid, qint32 status)
{
    QString query = QString("UPDATE t_Hummingbird SET f_status='%1',f_updated=DATETIME('NOW','LOCALTIME') WHERE f_uuid='%2'").arg(status).arg(uuid);
    QSqlQuery sqlQuery(query, this->db);
    bool rb = sqlQuery.exec();
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                        << ", query: " << query.toStdString().c_str());
        return rb;
    }

    LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str() << ", rb: " << rb << ", numRowsAffected: " << sqlQuery.numRowsAffected());
    return rb;
}

bool HummingbirdDB::updateHummingbirdDetail(TblHummingbirdDetail &tblHummingbirdDetail)
{
    tblHummingbirdDetail.f_name = tblHummingbirdDetail.f_name.replace("'", "''");
    tblHummingbirdDetail.f_distinct = tblHummingbirdDetail.f_distinct.replace("'", "''");
    tblHummingbirdDetail.f_remotedir = tblHummingbirdDetail.f_remotedir.replace("'", "''");

    QString query = QString("UPDATE t_Hummingbird_detail SET f_offset='%1',f_speed='%2',f_status='%3',f_updated=DATETIME('NOW','LOCALTIME') WHERE f_name='%4' AND f_remotedir='%5'")
            .arg(tblHummingbirdDetail.f_offset)
            .arg(tblHummingbirdDetail.f_speed)
            .arg(tblHummingbirdDetail.f_status)
            .arg(tblHummingbirdDetail.f_name)
            .arg(tblHummingbirdDetail.f_remotedir);
    QSqlQuery sqlQuery(query, this->db);
    bool rb = sqlQuery.exec();
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                        << ", query: " << query.toStdString().c_str());
        return rb;
    }

    LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str() << ", rb: " << rb << ", numRowsAffected: " << sqlQuery.numRowsAffected());
    return rb;
}

bool HummingbirdDB::updateHummingbirdDetail(const Hummingbirdp::TransferRequest &transferRequest,
                                const Hummingbirdp::TransferRespond &transferRespond,
                                QHash<QString, Hummingbirdp::TransferRequest_Fragment> &hashFragment,
                                quint64 &f_speed)
{
    this->db.transaction();
    quint64 offsets = 0;
    for (int i = 0; i < transferRequest.fragment_size(); i++)
    {
        const Hummingbirdp::TransferRequest_Fragment &fragment = transferRequest.fragment(i);
        Hummingbirdp::TransferRequest_Fragment sent_fragment = hashFragment.take(QString::fromStdString(fragment.name()));
        if (sent_fragment.name() == fragment.name())
        {
            TblHummingbirdDetail tblHummingbirdDetail = {};
            tblHummingbirdDetail.f_name = QString::fromStdString(fragment.name());
            tblHummingbirdDetail.f_status = 0; // NOTE: pending
            tblHummingbirdDetail.f_offset = fragment.offset(); // NOTE: sent
            tblHummingbirdDetail.f_remotedir = QString::fromStdString(fragment.path());

            if (transferRespond.errnum() == 0)
            {
                quint64 offset = fragment.plain();
                tblHummingbirdDetail.f_offset += offset; // NOTE: applied
                if (fragment.eof())
                {
                    tblHummingbirdDetail.f_status = 2; // NOTE: finished
                }

                for (int j = 0; j < transferRespond.copyonwrite_size(); j++)
                {
                    const Hummingbirdp::TransferRespond_CopyOnWrite &copyonwrite = transferRespond.copyonwrite(j);
                    if (copyonwrite.name() == fragment.name()
                            && copyonwrite.path() == fragment.path()
                            && copyonwrite.distinct() == fragment.distinct())
                    {
                        tblHummingbirdDetail.f_status = 3; // NOTE: finished
                    }
                }

                qint64 currentMSecs = QDateTime::currentMSecsSinceEpoch();
                tblHummingbirdDetail.f_speed = 1000 * offset / (currentMSecs - transferRequest.created());
                offsets += offset;
            }

            /**
             * TODO: refresh offset.., and etc
             * */
            bool rb = this->updateHummingbirdDetail(tblHummingbirdDetail);
            if (!rb)
            {
                return this->db.rollback();
            }
        }
    }

    qint64 currentMSecs = QDateTime::currentMSecsSinceEpoch();
    f_speed = 1000 * offsets / (currentMSecs - transferRequest.created());
    return this->db.commit();
}

bool HummingbirdDB::resetHummingbirdDetail()
{
    QString query = QString("UPDATE t_Hummingbird_detail SET f_speed='0',f_status='0',f_updated=DATETIME('NOW','LOCALTIME') WHERE f_status='1'");
    QSqlQuery sqlQuery(query, this->db);
    bool rb = sqlQuery.exec();
    if (!rb)
    {
        LOG4CPLUS_ERROR(root, "lastError: " << sqlQuery.lastError().text().toStdString().c_str()
                        << ", query: " << query.toStdString().c_str());
        return rb;
    }

    LOG4CPLUS_DEBUG(root, "query: " << query.toStdString().c_str() << ", rb: " << rb << ", numRowsAffected: " << sqlQuery.numRowsAffected());
    return rb;
}

QString HummingbirdDB::numberToHumanReadableValue(quint64 num)
{
    QString str;

    if (num < 1024)
    {
        str = QString("%1B").arg(num);
    }
    else if (num < (quint64)1024*1024)
    {
        str = QString("%1KB").arg((double)num/1024, 0, 'f', 2).replace(QRegExp("\\.{0,1}0+KB"), "KB");
    }
    else if (num < (quint64)1024*1024*1024)
    {
        str = QString("%1MB").arg((double)num/1024/1024, 0, 'f', 2).replace(QRegExp("\\.{0,1}0+MB"), "MB");
    }
    else if (num < (quint64)1024*1024*1024*1024)
    {
        str = QString("%1GB").arg((double)num/1024/1024/1024, 0, 'f', 2).replace(QRegExp("\\.{0,1}0+GB"), "GB");
    }
    else if (num < (quint64)1024*1024*1024*1024*1024)
    {
        str = QString("%1TB").arg((double)num/1024/1024/1024/1024, 0, 'f', 2).replace(QRegExp("\\.{0,1}0+TB"), "TB");
    }
    else
    {
        str = QString("∞");
    }
    return str;
}

QSqlDatabase &HummingbirdDB::get_db()
{
    return this->db;
}
