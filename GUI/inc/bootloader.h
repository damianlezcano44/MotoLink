#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <QObject>
#include <compat.h>
#include <QString>
#include <QtEndian>
#include <QTimer>
#include <QThread>
#include <QUsb>

typedef quint8 uint8_t;
#include "protocol.h"

#define WAIT_USB _usleep(40000);

class Bootloader : public QObject
{
    Q_OBJECT
public:
    explicit Bootloader(QUsb *usb, QObject *parent = 0);
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
    QUsb *mUsb;

signals:
    void connectionResult(bool result);
    void timeElapsed(int time);
    
};

#endif // BOOTLOADER_H
