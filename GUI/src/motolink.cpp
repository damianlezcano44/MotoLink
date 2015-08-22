#include "motolink.h"

Motolink::Motolink(QObject *parent) :
    QObject(parent)
{
    mUsb = new QUsbDevice;
    mThread = new QThread;

    mGuid = "656d69a0-4f42-45c4-b92c-bffa3b9e6bd1";
    mVid = 0x0483;
    mPid = 0xABCD;

    QtUsb::DeviceFilter filter;
    QtUsb::DeviceConfig config;

    filter.guid = mGuid;
    filter.pid = mPid;
    filter.vid = mVid;

    config.readEp = 0x83;
    config.writeEp = 0x03;
    config.alternate = 0;
    config.config = 1;
    config.interface = 0;

    mUsb->setFilter(filter);
    mUsb->setConfig(config);
    mUsb->setTimeout(200);

    mBtl = new Bootloader(mUsb);

    mConnected = false;
    mAbortConnect = false;

    mUsb->setDebug(true);

    this->moveToThread(mThread);
    mThread->start();
}

Motolink::~Motolink()
{
    mThread->exit();
    if (!mThread->wait(1000)) {
        mThread->terminate();
        mThread->wait();
    }
    delete mThread;
    delete mBtl;
    this->usbDisconnect();
    delete mUsb;
}

bool Motolink::usbConnect()
{
    if (mConnected)
        return true;
    _LOCK_
    mConnected = (mUsb->open() == 0);
    _UNLOCK_

    if (!mConnected || !this->sendWake())
    {
        mConnected = false;
        mUsb->close();
        return false;
    }

    mUsb->flush();
    return true;
}

bool Motolink::usbProbeConnect()
{
    _LOCK_
    QByteArray tmp;
    QElapsedTimer timer;
    qint32 ret = -1;
    mAbortConnect = false;
    emit connectionProgress(0);

    timer.start();
    while (timer.elapsed() < 10000)
    {
        emit timeElapsed(timer.elapsed());
        ret = mUsb->open();
        if (ret >= 0 || mAbortConnect) {
            break;
        }
        QThread::msleep(100);
        emit connectionProgress(timer.elapsed()/100);
    }

    if (ret < 0) {
        qWarning("Probing Failed");
        emit connectionResult(false);
        mConnected = false;
        emit connectionProgress(0);
        return false;
    }

    /* Clean buffer */
    //mUsb->read(&tmp, 256);
    mUsb->flush();

    _UNLOCK_
    if (!this->sendWake())
    {
        qWarning("Connection Failed");
        return false;
    }

    mConnected = true;

    emit connectionProgress(100);
    emit connectionResult(true);

    return true;
}

bool Motolink::usbDisconnect(void)
{
    if (!mConnected) {
        return true;
    }

    _LOCK_
    mUsb->close();
    mConnected = false;

    return true;
}

quint8 Motolink::getMode(void)
{
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_MODE);

    if (this->sendCmd(&send, &recv, 1, CMD_GET_MODE))
        return recv.at(0);

    return 0;
}


quint16 Motolink::getVersion()
{
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_VERSION);

    if (this->sendCmd(&send, &recv, 2, CMD_GET_VERSION))
        return recv.at(0) + (recv.at(1)*256);

    return 0;
}

bool Motolink::readSensors(void)
{
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_SENSORS);


    if (this->sendCmd(&send, &recv, sizeof(sensors_t), CMD_GET_SENSORS))
    {
        //memcpy((void*)&mSensors, (void*)data->constData(), sizeof(sensors_t));

        const sensors_t * sensors =  (sensors_t *)recv.constData();

        mSensors.vAn7 = sensors->an7/1000.0; /* VBAT */
        mSensors.vAn8 = sensors->an8/1000.0; /* TPS */
        mSensors.vAn9 = sensors->an9/1000.0; /* AFR */
        mSensors.tps = sensors->tps/2.0;
        mSensors.rpm = sensors->rpm*100;
        mSensors.freq1  = sensors->freq1;
        mSensors.freq2  = sensors->freq2;
        mSensors.knock_value = sensors->knock_value;
        mSensors.knock_freq = sensors->knock_freq*100;
        mSensors.afr = sensors->afr/10.0;
        mSensors.row = sensors->cell.row;
        mSensors.col = sensors->cell.col;
        emit receivedSensors(&mSensors);
        return true;
    }

    return false;
}

bool Motolink::readMonitoring(void)
{
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_MONITOR);

    if (this->sendCmd(&send, &recv, sizeof(monitor_t), CMD_GET_MONITOR))
    {
        memcpy((void*)&mMonitoring, (void*)recv.constData(), sizeof(monitor_t));
        emit receivedMonitoring(&mMonitoring);
        return true;
    }

    return false;
}

bool Motolink::readKnockSpectrum(void)
{
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_FFT);

    if (this->sendCmd(&send, &recv, SPECTRUM_SIZE, CMD_GET_FFT))
    {
        mKnockData = recv;
        emit receivedKockSpectrum(&mKnockData);
        return true;
    }

    qWarning("getKnockSpectrum Error!");

    return false;
}

bool Motolink::readTables(void)
{
    if (!mConnected)
        return false;

    QByteArray send, recv;
    int size = sizeof(mAFRTable)+sizeof(mKnockTable);
    this->prepareCmd(&send, CMD_GET_TABLES);

    if (this->sendCmd(&send, &recv, size, CMD_GET_TABLES))
    {
        memcpy((void*)mAFRTable, (void*)recv.constData(), sizeof(mAFRTable));
        memcpy((void*)mKnockTable, (void*)(recv.constData()+sizeof(mAFRTable)), sizeof(mKnockTable));
        emit receivedTables((quint8*)mAFRTable, (quint8*)mKnockTable);
        return true;
    }

    return false;
}

bool Motolink::writeTablesHeaders()
{
    if (!mConnected)
        return false;

    QByteArray send, recv;
    send.append((char*)mTablesRows, sizeof(mTablesRows));
    send.append((char*)mTablesColumns, sizeof(mTablesColumns));
    this->prepareCmd(&send, CMD_SET_TABLES_HEADERS);

    return this->sendCmd(&send, &recv, 0, CMD_SET_TABLES_HEADERS);
}

bool Motolink::readSerialData()
{
    if (!mConnected)
        return false;

    QByteArray send, recv;
    int size = 256;
    this->prepareCmd(&send, CMD_GET_SERIAL_DATA);

    if (this->sendCmd(&send, &recv, size, CMD_GET_SERIAL_DATA))
    {
        emit receivedSerialData(recv);
        return true;
    }

    return false;
}

bool Motolink::readSettings(settings_t *settings)
{
    (void)settings;
    return true;
}

bool Motolink::writeTablesHeaders(const quint8 *rows, const quint8 *cols)
{
    memcpy(mTablesRows, rows, sizeof(mTablesRows));
    memcpy(mTablesColumns, cols, sizeof(mTablesColumns));

    return this->writeTablesHeaders();
}

bool Motolink::clearCell(uint tableId, int row, int col)
{
    if (!mConnected)
        return false;

    QByteArray send, recv;
    cell_t cell = {(uint8_t)row, (uint8_t)col};
    send.append(tableId&0xFF);
    send.append((char*)&cell, sizeof(cell));
    this->prepareCmd(&send, CMD_CLEAR_CELL);

    return this->sendCmd(&send, &recv, 0, CMD_CLEAR_CELL);
}

bool Motolink::clearTables()
{
    return this->sendSimpleCmd(CMD_CLEAR_TABLES);
}

bool Motolink::writeSettings(const settings_t *settings)
{
    (void)settings;
    return true;
}


bool Motolink::sendWake()
{
    return this->sendSimpleCmd(CMD_WAKE);
}

void Motolink::startUpdate(QByteArray *data)
{
    this->sendFirmware(data);
    this->verifyFirmware(data);

    emit updateDone();
}

bool Motolink::resetDevice()
{
    return this->sendSimpleCmd(CMD_RESET);
}

bool Motolink::bootAppIfNeeded()
{
    if (!mConnected)
        this->usbConnect();

    if (this->getMode() == MODE_BL)
    {
        mBtl->boot();
        QThread::msleep(100);
        this->usbDisconnect();
        QThread::msleep(2000);
        this->usbConnect();
        return true;
    }
    return false;
}

quint8 Motolink::checkSum(const quint8 *data, quint8 length) const
{
    quint8 i;
    quint8 sum = 0;

    for (i = 0; i < length; i++)
      sum += data[i];

    return sum;
}

void Motolink::setupConnections()
{
    //QObject::connect(mTft, SIGNAL(updateDone()), mBtl, SLOT(disconnect()));
}

void Motolink::prepareCmd(QByteArray *cmdBuf, quint8 cmd) const
{
    cmdBuf->insert(0, MAGIC1);
    cmdBuf->insert(1, MAGIC2);
    cmdBuf->insert(2, MASK_CMD | cmd);
    cmdBuf->insert(3, cmdBuf->size()+2); // +2 = The byte we are adding now, and the checksum.
    cmdBuf->append(checkSum((quint8*)cmdBuf->constData(), cmdBuf->size()));
}

bool Motolink::sendSimpleCmd(quint8 cmd)
{
    _WAIT_USB_
    _LOCK_
    bool result = false;
    QByteArray send, recv;
    this->prepareCmd(&send, cmd);

    if (mUsb->write(&send, send.size()) != send.size())
    {
        return 0;
    }
    if (mUsb->read(&recv, 1) > 0)
    {
        result = recv.at(0) == (MASK_REPLY_OK | cmd);
        if (!result)
            this->printError(recv.at(0));
    }
    return result;
}

bool Motolink::sendCmd(QByteArray *send, QByteArray *recv, uint len, quint8 cmd)
{
    _WAIT_USB_
    _LOCK_
    bool result;
    recv->clear();

    if (mUsb->write(send, send->size()) < send->size())
    {
        return 0;
    }
    mUsb->read(recv, len+1); // Add result byte

    result = recv->at(0) == (MASK_REPLY_OK | cmd);
    if (!result)
        this->printError(recv->at(0));

    recv->remove(0, 1);

    return result && (uint)recv->size() == len;
}

void Motolink::printError(quint8 reply)
{
    QString error;
    if (reply & MASK_DECODE_ERR)
    {
        if (reply & 1)
            error.append("Header read error");
        else if (reply & 2)
            error.append("Header decode error");
        else if (reply & 3)
            error.append("Data read error");
        else if (reply & 4)
            error.append("Checksum error");
        else if (reply & 5)
            error.append("Unknown command");

        emit communicationError(error);
    }
    else if (reply & MASK_CMD_ERR)
    {
        error.append("Command error: ");

        if ((reply & MASK_CMD_PART) == CMD_GET_MONITOR)
            error.append("Read monitor");
        else if ((reply & MASK_CMD_PART) == CMD_GET_SENSORS)
            error.append("Read sensors");
        else if ((reply & MASK_CMD_PART) == CMD_GET_SETTINGS)
            error.append("Read settings");
        else if ((reply & MASK_CMD_PART) == CMD_GET_TABLES)
            error.append("Read tables");
        else if ((reply & MASK_CMD_PART) == CMD_GET_TABLES_HEADERS)
            error.append("Read tables headers");
        else if ((reply & MASK_CMD_PART) == CMD_SET_TABLES_HEADERS)
            error.append("Write tables headers");
        else if ((reply & MASK_CMD_PART) == CMD_GET_FFT)
            error.append("Read fft");
        else if ((reply & MASK_CMD_PART) == CMD_GET_VERSION)
            error.append("Read version");
        else if ((reply & MASK_CMD_PART) == CMD_CLEAR_CELL)
            error.append("Clear cell");
        else if ((reply & MASK_CMD_PART) == CMD_CLEAR_TABLES)
            error.append("Clear tables");
        else
            error.append(QString::number(reply));

        emit communicationError(error);
    }
}

void Motolink::haltTransfer(void)
{
    mStopTranfer = true;
    emit signalStatus(tr("Aborted"));
}

void Motolink::sendFirmware(QByteArray *data)
{
    QDataStream file(data, QIODevice::ReadOnly);
    emit signalLock(true);
    mStopTranfer = false;
    quint32 step_size = 240;

    quint32 progress, oldprogress;
    qint32 read;
    char *buf2 = new char[step_size];

    emit signalStatus(tr("Erasing..."));
    _LOCK_
    if (!mBtl->eraseFlash(data->size())) {

        emit signalStatus(tr("Erase failed"));
        emit signalLock(false);
        return;
    }
    else {
        emit signalStatus(tr("Erase OK"));
        _UNLOCK_
    }

    emit signalStatus(tr("Writing Flash"));

    progress = 0;
    for (int i=0; i<=data->size(); i+=step_size) {

        if (mStopTranfer)
            break;

        if (file.atEnd()) {
            break;
        }

        memset(buf2, 0, step_size);
        if ((read = file.readRawData(buf2, step_size)) <= 0)
            break;
        QByteArray buf(buf2, read);

        _LOCK_
        int wrote = mBtl->writeFlash(i, &buf, read);
        _UNLOCK_

        if (wrote < read){
            emit signalStatus(tr("Transfer failed"));
            qWarning() << tr("Transfer failed") << wrote << read;
            break;
        }

        oldprogress = progress;
        progress = (i*100)/data->size();
        if (progress > oldprogress) { // Push only if number has increased
            emit transferProgress(progress);
        }

    }
    delete buf2;

    emit transferProgress(100);
    emit signalStatus(tr("Transfer done"));

    emit signalLock(false);
}

void Motolink::verifyFirmware(QByteArray *data)
{
    QDataStream file(data, QIODevice::ReadOnly);
    emit signalLock(true);
    mStopTranfer = false;
    const quint32 buf_size = 508;
    char buf[buf_size];
    quint32 addr, progress, oldprogress;
    QByteArray data_local, data_remote;

    emit signalStatus(tr("Verifying flash"));

    progress = 0;
    for (int i=0; i<data->size(); i+=buf_size)
    {
        if (mStopTranfer)
            break;

        int read = file.readRawData(buf, buf_size);
        data_local.setRawData(buf, read);
        addr = i;

        if (!data_local.size()) break;
        _LOCK_
        int read_size = mBtl->readMem(addr, &data_remote, data_local.size());
        _UNLOCK_
        if (read_size < data_local.size()) { // Read same amount of data as from file.

            emit transferProgress(0);
            emit signalStatus(tr("Verification Failed"));
            qWarning() << tr("Verification Failed");
            emit signalLock(false);
            return;
        }

        if (data_remote != data_local) {

            emit transferProgress(100);
            emit signalStatus(tr("Verification failed at 0x")+QString::number(addr, 16));

            QString stmp, sbuf;
            for (int b=0;b<data_local.size();b++) {
                stmp.append(QString().sprintf("%02X ", (uchar)data_local.at(b)));
                sbuf.append(QString().sprintf("%02X ", (uchar)data_remote.at(b)));
            }
            qWarning() << tr("Verification failed at 0x")+QString::number(addr, 16) <<
                           "\r\n" << tr("Expecting:") << stmp << "\r\n       " << tr("Got:") << sbuf;
            emit signalLock(false);
            return;
        }
        oldprogress = progress;
        progress = (i*100)/data->size();
        if (progress > oldprogress) { // Push only if number has increased
            emit transferProgress(progress);
        }
        //emit sendStatus(tr("Verified ")+QString::number(i/1024)+tr(" kilobytes out of ")+QString::number(data->size()/1024));
    }

    emit transferProgress(100);
    emit signalStatus(tr("Verification OK"));
    qDebug() << tr("Verification OK");
    emit signalLock(false);
}
