#include "updatewizard.h"
#include "ui_updatewizard.h"
#include <QtXml>

UpdateWizard::UpdateWizard(Motolink * const mtl, QWidget *parent) :
    QWizard(parent),
    mUi(new Ui::UpdateWizard),
    mMtl(mtl)
{
    mUi->setupUi(this);
    this->setOptions(QWizard::NoBackButtonOnStartPage);
    this->setupConnections();
}

UpdateWizard::~UpdateWizard()
{
    delete mUi;
}

void UpdateWizard::showWizard()
{
    this->loadFirmareData();
    this->mUi->lNewVersion->setText(mNewVersion);
    this->enableButtons();
    this->show();
}

void UpdateWizard::startUpdate()
{
    mUi->pbProgress->setValue(50);
}

void UpdateWizard::retranslate()
{
    mUi->retranslateUi(this);
}

void UpdateWizard::pageUpdated(int page)
{
    switch (page)
    {
        case 0:
            break;
        case 1:
            this->startFwUpdate();
            break;
        case 2:
            break;
        default:
            break;
    }
}

void UpdateWizard::updateStatus(QString text)
{
    mUi->tbStatus->append(text);
}

void UpdateWizard::enableButtons(void)
{
    this->button(QWizard::CancelButton)->setEnabled(true);
    this->button(QWizard::FinishButton)->setEnabled(true);
    this->button(QWizard::BackButton)->setEnabled(true);
}

void UpdateWizard::disableButtons(void)
{
    this->button(QWizard::CancelButton)->setEnabled(false);
    this->button(QWizard::FinishButton)->setEnabled(false);
    this->button(QWizard::BackButton)->setEnabled(false);
}

void UpdateWizard::setupConnections()
{
    QObject::connect(mMtl->getTft(), SIGNAL(sendStatus(QString)), this, SLOT(updateStatus(QString)));
    QObject::connect(mMtl->getTft(), SIGNAL(sendProgress(int)), mUi->pbProgress, SLOT(setValue(int)));
    //QObject::connect(mMtl->getTft(), SIGNAL(sendLock(bool)), this, SLOT(disableButtons(bool)));
    QObject::connect(mMtl->getTft(), SIGNAL(finished()), this, SLOT(enableButtons()));
    QObject::connect(mMtl->getTft(), SIGNAL(finished()), mMtl->getBtl(), SLOT(boot()));

    QObject::connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageUpdated(int)));
    QObject::connect(this, SIGNAL(startTransfer()), mMtl->getTft(), SLOT(start()));
    QObject::connect(this, SIGNAL(send(QByteArray*)), mMtl->getTft(), SLOT(send(QByteArray*)));
    QObject::connect(this, SIGNAL(verify(QByteArray*)), mMtl->getTft(), SLOT(verify(QByteArray*)));
}

void UpdateWizard::loadFirmareData()
{
    QDomDocument doc("firmware");
    QFile file(":/fw/firmware.xml");

    if (!file.open(QIODevice::ReadOnly))
        return;

    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();

    while(!n.isNull())
    {
        QDomElement e = n.toElement();
        if (n.childNodes().count() > 1)
        {
            QDomNode cn = n.firstChild();
            while (!cn.isNull())
            {
                QDomElement ce = cn.toElement();
                if(!ce.isNull())
                {
                    if (ce.tagName() == "version")
                    {
                        this->mNewVersion = ce.text();
                    }
                }
                cn = cn.nextSibling();
            }
        }
        else
        {
            if(!e.isNull())
            {
                if (e.tagName() == "data")
                {
                    this->mFwData = QByteArray::fromBase64(e.text().toLocal8Bit());
                }
            }
        }
        n = n.nextSibling();
    }
}

void UpdateWizard::startFwUpdate()
{
    mUi->tbStatus->clear();
    this->disableButtons();

    if (mMtl->usbConnect())
    {
        if (mMtl->getMode() != MODE_BL)
        {
            /* App mode, reset to bootloader */
            this->updateStatus(tr("Reset to Bootloader"));

            mMtl->reset();
            mMtl->usbDisconnect();
            mMtl->probeConnect();

            if (mMtl->getMode() != MODE_BL)
            {
                this->updateStatus(tr("Failed to reset device"));
                return;
            }

        }
        this->updateStatus(tr("Connected"));
        mUi->pbProgress->setValue(10);

        const quint8 flags = mMtl->getBtl()->getFlags();

        if (flags & FLAG_IWDRST)
            this->updateStatus(tr("Watchdog reset"));

        if (flags & FLAG_SFTRST)
            this->updateStatus(tr("Software reset"));

        if (flags & FLAG_NOAPP)
            this->updateStatus(tr("No valid user application"));

        if (flags & FLAG_WAKE)
            this->updateStatus(tr("Bootloader Wakeup"));

        mMtl->getTft()->setParams(&mFwData, true, true);
        emit startTransfer();
    }
    else
    {
        this->updateStatus(tr("Connection Failed"));
        this->enableButtons();
        return;
    }
}
