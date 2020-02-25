#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "qusbdevice.h"
#include "qusbendpoint.h"
#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QtEndian>
#include <compat.h>

typedef quint8 uint8_t;
typedef quint16 uint16_t;
typedef quint32 uint32_t;
#include "protocol.h"

//#define WAIT_USB _QThread::usleep(40000);
#define _WAIT_USB_

class Bootloader : public QObject
{
    Q_OBJECT
public:
    explicit Bootloader(QUsbEndpoint *read_ep, QUsbEndpoint *write_ep, QObject *parent = Q_NULLPTR);
    ~Bootloader();

public slots:
    quint8 getFlags();
    bool boot();
    qint32 writeFlash(quint32 addr, const QByteArray *data, quint32 len);
    qint32 readMem(quint32 addr, QByteArray *data, quint32 len);
    bool eraseFlash(quint32 len);

private slots:
    quint8 checkSum(const quint8 *data, quint8 length) const;

private:
    void prepareCmd(QByteArray *cmdBuf, quint8 cmd) const;
    QUsbEndpoint *mReadEp;
    QUsbEndpoint *mWriteEp;

signals:
    void connectionResult(bool result);
    void timeElapsed(int time);
};

#endif // BOOTLOADER_H
