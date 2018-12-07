#include "Hummingbird.h"

void my_free (void *data, void *hint)
{
    (void)hint;
    free (data);
}

Hummingbird::Hummingbird(QThread *parent, QString connectionName, QString remote_addr,
             int nitems, int timeout, quint64 max_msg_size,
             qint64 msg_timeout) :
    QThread(parent),
    remote_addr(remote_addr),
    nitems(nitems), timeout(timeout), max_msg_size(max_msg_size), msg_timeout(msg_timeout),
    hummingbirdDb(this, connectionName)
{
    QString fileName("Hummingbird.ini");
    QSettings settings(fileName, QSettings::IniFormat);

    if (settings.contains("common/timeout"))
    {
        this->timeout = settings.value("common/timeout").toInt();
    }
    else
    {
        settings.setValue("common/timeout", this->timeout);
    }

    if (settings.contains("common/max_msg_size"))
    {
        this->max_msg_size = 1024 * 1024 * settings.value("common/max_msg_size").toInt();
    }
    else
    {
        settings.setValue("common/max_msg_size", this->max_msg_size / 1024 / 1024);
    }

    if (settings.contains("common/msg_timeout"))
    {
        this->msg_timeout = settings.value("common/msg_timeout").toInt();
    }
    else
    {
        settings.setValue("common/msg_timeout", this->msg_timeout);
    }

    if (settings.contains("common/nitems"))
    {
        this->nitems = settings.value("common/nitems").toInt();
    }
    else
    {
        settings.setValue("common/nitems", this->nitems);
    }

    if (settings.contains("zlib/compress_level"))
    {
        this->compress_level = settings.value("zlib/compress_level").toInt();
    }
    else
    {
        settings.setValue("zlib/compress_level", this->compress_level);
    }

    LOG4CPLUS_DEBUG(root, "Hummingbird compiled: " << __DATE__ << " " << __TIME__);
    LOG4CPLUS_DEBUG(root, "zlibVersion: " << zlibVersion());
    LOG4CPLUS_DEBUG(root, "zmq_version: " << ZMQ_VERSION);

    this->context = zmq_ctx_new ();
    LOG4CPLUS_DEBUG(root, "zmq_ctx_new: " << this->context);

    this->last_check_msecs = QDateTime::currentMSecsSinceEpoch();
    this->last_pollout_check_msecs = this->last_check_msecs;

    this->file_buffer = malloc(this->max_msg_size);

    this->hummingbirdDb.resetHummingbirdDetail();
}

Hummingbird::~Hummingbird()
{
    free(this->file_buffer);
    this->file_buffer = NULL;

    int r = zmq_ctx_term (this->context);
    if (r == -1)
    {
        LOG4CPLUS_ERROR(root, "zmq_ctx_term(" << this->context << "): " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
    }
    else
    {
        LOG4CPLUS_DEBUG(root, "zmq_ctx_term(" << this->context << ")");
    }

    log4cplus::threadCleanup();
}

void Hummingbird::run()
{
    int r;
    for (int i = 0; i < this->nitems; i++)
    {
        zmq_pollitem_t item = {};
        int type = ZMQ_REQ;
        item.socket = zmq_socket (this->context, type);
        if (item.socket == NULL)
        {
            LOG4CPLUS_ERROR(root, "zmq_socket(" << this->context << "," << type << "): " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
            return;
        }

        int option_name = ZMQ_RECONNECT_IVL_MAX;
        int option_value = 200; // NOTE: milliseconds
        size_t option_len = sizeof (int);
        r = zmq_setsockopt (item.socket, option_name, &option_value, option_len);
        if (r == -1)
        {
            LOG4CPLUS_ERROR(root, "zmq_setsockopt(" << item.socket << "," << option_name << "," << option_value << "," << option_len << "): " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
            return;
        }

        r = zmq_connect (item.socket, this->remote_addr.toStdString().c_str());
        if (r == -1)
        {
            LOG4CPLUS_ERROR(root, "zmq_connect(" << item.socket << "," << this->remote_addr.toStdString().c_str() << "): " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
            return;
        }

        item.events = ZMQ_POLLIN | ZMQ_POLLOUT;
        this->items[i] = item;
    }

    while (true)
    {
        // NOTE: check timeout's transferRequest, and resend them
        resendCheck();
        r = zmq_poll (this->items, this->nitems, (long)this->timeout);
        if (r == -1)
        {
            LOG4CPLUS_ERROR(root, "zmq_poll(" << this->items << "," << this->nitems << "," << this->timeout << "): " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
        }
        else if (r == 0)
        {
            qint64 currentMSecs = QDateTime::currentMSecsSinceEpoch();
            LOG4CPLUS_DEBUG(root, "zmq_poll, timeout: " << this->timeout << ", (" << currentMSecs - this->last_check_msecs << "," << this->msg_timeout << "): " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
        }
        else
        {
            for (int i = 0, j = 0; i < this->nitems && j < r; i++)
            {
                zmq_pollitem_t &item = this->items[i];

                if (item.revents & ZMQ_POLLIN)
                {
                    receivedMessage(item);
                }

                if (item.revents & ZMQ_POLLOUT)
                {
                    creep(item);
                }

                if (item.revents & ZMQ_POLLERR)
                {
                    LOG4CPLUS_ERROR(root, "ZMQ_POLLERR:socket: " << item.socket);
                }

                if (item.revents)
                {
                    j++;
                }
            }
        }
    }

    return;
}

void Hummingbird::creep(zmq_pollitem_t &item)
{
    QList<TblHummingbirdDetail> tblHummingbirdDetailList;
    bool rb = this->hummingbirdDb.queryHummingbirdDetailList(tblHummingbirdDetailList, this->max_msg_size, this->hashFragment, this->remote_addr);
    if (!rb)
    {
        return;
    }

    if (tblHummingbirdDetailList.isEmpty())
    {
        item.events = ZMQ_POLLIN;
        return;
    }

    Hummingbirdp::TransferRequest transferRequest;
    QByteArray uuid = QUuid::createUuid().toByteArray();
    transferRequest.set_seq(uuid.toStdString());

    qint64 created = QDateTime::currentMSecsSinceEpoch();
    transferRequest.set_created(created);

    for (QList<TblHummingbirdDetail>::const_iterator it = tblHummingbirdDetailList.begin();
         it != tblHummingbirdDetailList.end(); it++)
    {
        TblHummingbirdDetail tblHummingbirdDetail = *it;
        Hummingbirdp::TransferRequest_Fragment *pfragment = transferRequest.add_fragment();

        pfragment->set_name(tblHummingbirdDetail.f_name.toStdString());
        pfragment->set_offset(tblHummingbirdDetail.f_offset);


#if __DARWIN_C_LEVEL >= 200112L
        const char *filename = tblHummingbirdDetail.f_name.toStdString().c_str();
        const char *mode = "rb";
        FILE *fp = fopen(filename, mode);
        if (!fp)
        {
            LOG4CPLUS_ERROR(root, "fopen(" << tblHummingbirdDetail.f_name.toStdString().c_str() << "," << mode << "): " << strerror(errno) << "[" << errno << "]");
            return;
        }

        off_t offset = tblHummingbirdDetail.f_offset;
        int r = fseeko(fp, offset, SEEK_SET);
        if (r == -1)
        {
            LOG4CPLUS_ERROR(root, "fseeko(" << offset << ",..): " << strerror(errno) << "[" << errno << "]");
            fclose(fp);
            return;
        }
#else
        const wchar_t *filename = reinterpret_cast<const wchar_t *>(tblHummingbirdDetail.f_name.utf16());
        const wchar_t *mode = L"rb";
        FILE *fp = _wfopen(filename, mode);
        if (!fp)
        {
            LOG4CPLUS_ERROR(root, "_wfopen(" << tblHummingbirdDetail.f_name.toStdString().c_str() << "," << mode << "): " << strerror(errno) << "[" << errno << "]");
            return;
        }

        _off64_t offset = tblHummingbirdDetail.f_offset;
        int r = fseeko64(fp, offset, SEEK_SET);
        if (r == -1)
        {
            LOG4CPLUS_ERROR(root, "fseeko64(" << offset << ",..): " << strerror(errno) << "[" << errno << "]");
            fclose(fp);
            return;
        }
#endif

        bool eof = false;
        size_t count = (offset == 0 ? BUFSIZ : this->max_msg_size);
        size_t rfr = fread(this->file_buffer, sizeof (char), count, fp);
        fclose(fp);
        if (rfr < count)
        {
            eof = true;
        }

        const Bytef *buf;
        uInt len;
        uLong sourceLen = rfr;
        uLong destLen = compressBound (sourceLen);
        const Bytef *source = (const Bytef *)this->file_buffer;
        Bytef *dest = (Bytef *)malloc(destLen);
        if (dest == NULL)
        {
            LOG4CPLUS_ERROR(root, "malloc(" << destLen << "): " << strerror(errno) << "[" << errno << "]");
            return;
        }

        r = compress2 (dest, &destLen, source, sourceLen, this->compress_level);
        if (r != Z_OK)
        {
            free(dest);
            LOG4CPLUS_ERROR(root, "r: %d, compress2 returns Z_OK(%d) if success, Z_MEM_ERROR(%d) if there was not enough"
                                  "memory, Z_BUF_ERROR(%d) if there was not enough room in the output buffer,"
                                  "Z_STREAM_ERROR(%d) if the level parameter is invalid." << r
                                  << Z_OK << Z_MEM_ERROR << Z_BUF_ERROR << Z_DATA_ERROR);

            return;
        }

        uInt rate = 0;
        if (sourceLen > 0)
        {
            rate = 100 * (sourceLen >= destLen ? sourceLen - destLen : 0) / sourceLen;
        }
        LOG4CPLUS_DEBUG(root, "Compress Rate: " << rate
                        << "%, sourceLen: " << sourceLen
                        << ", destLen: " << destLen
                        << ", name: " << tblHummingbirdDetail.f_name.toStdString().c_str());

        pfragment->set_ptr(dest, destLen);
        free(dest);
        buf = (const Bytef *)pfragment->ptr().c_str();
        len = destLen;
        quint32 crc = crc32(0L, buf, len);

        pfragment->set_crc32(crc);
        pfragment->set_eof(eof);
        pfragment->set_plain(sourceLen);
        pfragment->set_path(tblHummingbirdDetail.f_remotedir.toStdString());

        if (offset == 0 || eof)
        {
            pfragment->set_distinct(tblHummingbirdDetail.f_distinct.toStdString());
        }
        else
        {
            pfragment->set_distinct("");
        }

        Hummingbirdp::TransferRequest_Fragment fragment = *pfragment;
        this->hashFragment.insert(tblHummingbirdDetail.f_name, fragment);
    }
    this->hashTransferRequest.insert(QString::fromStdString(transferRequest.seq()), transferRequest);

    size_t size = transferRequest.ByteSizeLong();
    void *data = malloc(size);
    if (data == NULL)
    {
        return;
    }

    rb = transferRequest.SerializeToArray(data, size);
    if (!rb)
    {
        free(data);
        return;
    }

    void *socket = item.socket;
    zmq_msg_t msg;
    int r = zmq_msg_init_data (&msg, data, size, my_free, NULL);
    if (r == -1)
    {
        free(data);
        LOG4CPLUS_ERROR(root, "zmq_msg_init_data("
                        << (void *)&msg << ","
                        << data << "): "
                        << size << "): "
                        << zmq_strerror(zmq_errno()) << "("
                        << zmq_errno() << ")");
        return;
    }

    int flags = 0;
    int rs = zmq_sendmsg (socket, &msg, flags);
    if (rs == -1)
    {
        LOG4CPLUS_ERROR(root, "zmq_sendmsg("
                        << socket << ","
                        << (void *)&msg << ","
                        << flags << "): "
                        << zmq_strerror(zmq_errno()) << "("
                        << zmq_errno() << ")");
        r = zmq_msg_close (&msg);
        if (r == -1)
        {
            LOG4CPLUS_ERROR(root, "zmq_msg_close("
                            << (void *)&msg << "): "
                            << zmq_strerror(zmq_errno()) << "("
                            << zmq_errno() << ")");
        }

        return;
    }
    else
    {
        LOG4CPLUS_DEBUG(root, "rs: " << rs << ",zmq_sendmsg: " << data);
    }
    return;
}

void Hummingbird::resendCheck()
{
    qint64 currentMSecs = QDateTime::currentMSecsSinceEpoch();

    if (currentMSecs > this->last_pollout_check_msecs + this->timeout)
    {
        for (int j = 0; j < this->nitems; j++)
        {
            this->items[j].events = ZMQ_POLLIN | ZMQ_POLLOUT;
        }

        this->last_pollout_check_msecs = currentMSecs;
    }

    if (currentMSecs < this->last_check_msecs + this->msg_timeout)
    {
        return;
    }

    QHash<QString, Hummingbirdp::TransferRequest>::const_iterator it = this->hashTransferRequest.begin();
    while (it != this->hashTransferRequest.end())
    {
        const Hummingbirdp::TransferRequest &transferRequest = it.value();
        LOG4CPLUS_DEBUG(root, "hash:[" << &this->hashTransferRequest
                        << "] entry:[key: " << it.key().toStdString().c_str()
                        << ", values(created: " << transferRequest.created() << ",..)]");
        if (currentMSecs < transferRequest.created() + this->msg_timeout)
        {
            it++;
        }
        else
        {
            LOG4CPLUS_WARN(root, "seq: " << transferRequest.seq().c_str()
                           << ", waiting is over, and reset:(" << currentMSecs - transferRequest.created()
                           << "/" << this->msg_timeout << ")");

            Hummingbirdp::TransferRespond transferRespond;
            transferRespond.set_errnum(-1); // NOTE: pretending to timeout
            quint64 f_speed = 0; // NOTE: B/s
            bool rb = this->hummingbirdDb.updateHummingbirdDetail(transferRequest, transferRespond, this->hashFragment, f_speed);
            if (!rb)
            {
                it++;
            }
            else
            {
                it = this->hashTransferRequest.erase(it);
            }
        }
    }

    this->last_check_msecs = currentMSecs;
    return;
}

void Hummingbird::receivedMessage(zmq_pollitem_t &item)
{
    zmq_msg_t msg;
    int r = zmq_msg_init (&msg);
    if (r == -1)
    {
        LOG4CPLUS_ERROR(root, "zmq_msg_init(" << (void *)&msg << "): " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
        return;
    }

    int flags = ZMQ_DONTWAIT;
    int rr = zmq_msg_recv (&msg, item.socket, flags);
    if (rr == -1)
    {
        LOG4CPLUS_ERROR(root, "zmq_msg_recv(" << (void *)&msg << "," << item.socket << "," << flags << "): " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
    }
    else
    {
        const void *msg_content = zmq_msg_data (&msg);
        LOG4CPLUS_DEBUG(root, "rr: " << rr << ",msg_content: " << msg_content);

        Hummingbirdp::TransferRespond transferRespond;
        bool rb = transferRespond.ParseFromArray(msg_content, rr);
        if (!rb)
        {
            return;
        }
        QString seq = QString::fromStdString(transferRespond.seq());
        Hummingbirdp::TransferRequest transferRequest = this->hashTransferRequest.take(seq);
        if (transferRequest.seq() == transferRespond.seq())
        {
            quint64 f_speed = 0; // NOTE: B/s
            this->hummingbirdDb.updateHummingbirdDetail(transferRequest, transferRespond, this->hashFragment, f_speed);
            LOG4CPLUS_INFO(root, "speed: " << HummingbirdDB::numberToHumanReadableValue(f_speed).toStdString().c_str() << "/s");
        }
    }

    r = zmq_msg_close (&msg);
    if (r == -1)
    {
        LOG4CPLUS_ERROR(root, "zmq_msg_close(" << (void *)&msg << "): " << zmq_strerror(zmq_errno()) << "(" << zmq_errno() << ")");
        return;
    }
    return;
}
