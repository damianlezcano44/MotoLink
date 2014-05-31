#include "motolink.h"

Motolink::Motolink(QObject *parent) :
    QObject(parent)
{
    mUsb = new QUsb;

    mGuid = "656d69a0-4f42-45c4-b92c-bffa3b9e6bd1";
    mPid = 0x0483;
    mVid = 0xABCD;

    mUsb->setGuid(mGuid);
    mUsb->setDeviceIds(mPid, mVid);
    mUsb->setEndPoints(0x83, 0x03);

    mBtl = new Bootloader(mUsb);
    mTft = new TransferThread(mBtl);

    mConnected = false;
    mAbortConnect = false;
}

Motolink::~Motolink()
{
    delete mBtl;
    this->usbDisconnect();
    delete mUsb;
}

bool Motolink::usbConnect()
{
    if (mConnected)
        return true;

    mConnected = (mUsb->open() == 0);

    this->sendWake();

    return mConnected;
}

bool Motolink::probeConnect()
{
    QByteArray tmp;
    QElapsedTimer timer;
    qint32 ret = -1;
    mAbortConnect = false;
    emit connectionProgress(0);

    timer.start();
    while (timer.elapsed() < 10000) {

        emit timeElapsed(timer.elapsed());
        ret = mUsb->open();
        if (ret >= 0 || mAbortConnect) {
            break;
        }
        _usleep(100000);
        emit connectionProgress(timer.elapsed()/100);
    }

    if (ret < 0) {
        emit connectionResult(false);
        mConnected = false;
        emit connectionProgress(0);
        return false;
    }

    /* Clean buffer */
    mUsb->read(&tmp, 256);
    mConnected = true;

    this->sendWake();

    emit connectionProgress(100);
    emit connectionResult(true);

    return true;
}

bool Motolink::usbDisconnect(void)
{
    if (!mConnected)
        return true;

    mUsb->close();
    mConnected = false;

    return true;
}

quint8 Motolink::getMode(void)
{
    QByteArray send, recv;

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_GET_MODE);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    mUsb->write(&send, send.size());
    mUsb->read(&recv, 2);

    if (recv.size() > 1 && recv.at(0) == MASK_REPLY_OK)
        return recv.at(1);

    return 0;
}


quint16 Motolink::getVersion()
{
    QByteArray send, recv;

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_GET_VERSION);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    mUsb->write(&send, send.size());
    mUsb->read(&recv, 3);

    if (recv.size() > 2 && recv.at(0) == MASK_REPLY_OK)
        return recv.at(1) + (recv.at(2)*256);

    return 0;
}

bool Motolink::sendWake()
{
    QByteArray send, recv;

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_WAKE);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    mUsb->write(&send, send.size());
    mUsb->read(&recv, 1);

    return recv.at(0) == MASK_REPLY_OK;
}

quint8 Motolink::checkSum(const quint8 *data, quint8 length) const
{
    quint8 i;
    quint8 sum = 0;

    for (i = 0; i < length; i++)
      sum += data[i];

    return sum;
}

void Motolink::setupConnection()
{
    QObject::connect(mTft, SIGNAL(finished()), mBtl, SLOT(disconnect()));
}
