#ifndef MOTOLINK_H
#define MOTOLINK_H

#include <QObject>
#include <QUsb>

class Motolink : public QObject
{
    Q_OBJECT
public:
    explicit Motolink(QUsb *usb, QObject *parent = 0);

signals:

public slots:


private:
    QUsb *mUsb;
};

#endif // MOTOLINK_H
