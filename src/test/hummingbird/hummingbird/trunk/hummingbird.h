#ifndef SNAIL_H
#define SNAIL_H

#include <QThread>
#include <QHash>
#include <QDateTime>
#include <QUuid>
#include <QtEndian>
#include <QtGlobal>
#include <QSettings>
#include <QDir>
#include <QFile>

#include <zmq.h>
#include <zlib.h>

#include "Hummingbirddb.h"

extern log4cplus::Logger root;

/**
 * @brief The Hummingbird class
 */
class Hummingbird : public QThread
{
    Q_OBJECT
public:
    explicit Hummingbird(QThread *parent = 0, QString connectionName = "",
                   QString remote_addr = "",
                   int nitems = ZMQ_POLLITEMS_DFLT,
                   int timeout = 1000,
                   quint64 max_msg_size = 4*1024*1024,
                   qint64 msg_timeout = 60*1000);
    ~Hummingbird();
    void run();

private:
    void *context; // ØMQ

    /**
     * @brief remote_addr
     * such as "tcp://<ip-or-domain-name>:<port>"
     */
    QString remote_addr;

    zmq_pollitem_t items[ZMQ_MAX_SOCKETS_DFLT];

    /**
     * @brief nitems
     */
    int nitems;

    /**
     * @brief timeout
     */
    int timeout; // NOTE: milliseconds
    void *file_buffer;

    QHash<QString, hummingbirdp::Request> hashRequest; // NOTE: key:seq
    QHash<QString, hummingbirdp::Request_Fragment> hashFragment; // NOTE: key:file name

    /**
     * @brief max_msg_size
     */
    quint64 max_msg_size;

    /**
     * @brief level
     *      The compression level must be Z_DEFAULT_COMPRESSION, or between 0 and 9:
     *      1 gives best speed, 9 gives best compression, 0 gives no compression at
     *      all (the input data is simply copied a block at a time).
     *      Z_DEFAULT_COMPRESSION requests a default compromise between speed and
     *      compression (currently equivalent to level 6).
     */
    int compress_level = Z_DEFAULT_COMPRESSION;

    /**
     * @brief msg_timeout
     */
    qint64 msg_timeout;
    qint64 last_check_msecs;
    qint64 last_pollout_check_msecs;

    /**
     * @brief hummingbirdDb
     */
    HummingbirdDB hummingbirdDb;

    void creep(zmq_pollitem_t &item);
    void resendCheck();
    void receivedMessage(zmq_pollitem_t &item);

signals:

public slots:
};


#endif // SNAIL_H
