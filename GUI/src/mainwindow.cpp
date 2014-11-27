#include "mainwindow.h"
#include <QShortcut>
#include <QSignalMapper>
#include <ui_main.h>
#include <ui_tasks.h>
#include <ui_knock.h>
#include <ui_logs.h>
#include <QFileInfo>
#include <QModelIndex>
#include <QDateTime>
#include <QItemSelection>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mSettings("Motolink", "Motolink"),
    mHelpViewer(NULL),
    mUndoStack(NULL),
    mFuelModel(&mUndoStack, -30, 30, 0),
    mStagingModel(&mUndoStack, -15, 15, 0, true),
    mAFRModel(&mUndoStack, 70, 240, 130, false, false),
    mAFRTgtModel(&mUndoStack, 80, 200, 130),
    mIgnModel(&mUndoStack, -20, 3, 0),
    mKnockModel(&mUndoStack, 0, 800, 0, false, false)
{
    mMainUi = new Ui::MainWindow();
    mTasksUi = new Ui::Tasks();
    mLogsUi = new Ui::Logs();
    mKnockGraphUi = new Ui::KnockGraph();

    mTasksWidget = new QWidget();
    mKnockGraphWidget = new QWidget;
    mLogsWidget = new QWidget();

    mMainUi->setupUi(this);
    mTasksUi->setupUi(mTasksWidget);
    mKnockGraphUi->setupUi(mKnockGraphWidget);
    mLogsUi->setupUi(mLogsWidget);

    mMtl = new Motolink();
    mHrc = new Hrc();
    mUpdateWizard = new UpdateWizard(mMtl);

    mUndoStack.setUndoLimit(100);

    this->setupDefaults();
    this->setupConnections();
    this->setupTabShortcuts();
    this->setupSettings();
    this->setupKnockGraph();

    mUndoView.setStack(&mUndoStack);
    mUndoView.setWindowTitle(tr("Actions History - ")+this->windowTitle());
    mHasChanged = false;

    mMainUi->sbIdle->setName(tr("Idle"));
    mMainUi->sbPitLimiter->setName(tr("Pit Limiter"));
    mMainUi->sbShiftLight->setName(tr("Shift Light"));
    mMainUi->sbThresholdMin->setName(tr("Minimum Threshold"));
    mMainUi->sbThresholdMax->setName(tr("Maximum Threshold"));

    mMainUi->sbIdle->setUndoStack(&mUndoStack);
    mMainUi->sbPitLimiter->setUndoStack(&mUndoStack);
    mMainUi->sbShiftLight->setUndoStack(&mUndoStack);
    mMainUi->sbThresholdMin->setUndoStack(&mUndoStack);
    mMainUi->sbThresholdMax->setUndoStack(&mUndoStack);

    mFastPollingTimer.setInterval(50);
    mSlowPollingTimer.setInterval(500);
    mTablesTimer.setInterval(250);
    mRedrawTimer.setInterval(250);

    this->exportToMTLFile();
    this->uiDisable();

    emit signalStartupComplete();
}

MainWindow::~MainWindow()
{
    delete mMainUi;
    delete mUpdateWizard;
    delete mTasksUi;
    delete mTasksWidget;
    delete mKnockGraphUi;
    delete mKnockGraphWidget;
    delete mLogsUi;
    delete mLogsWidget;
    delete mMtl;
    delete mHrc;

    for (int i = 0; i < MAX_RECENT_FILES; ++i)
    {
        if (mRecentFilesActions[i] != NULL)
            delete mRecentFilesActions[i];
    }
}

void MainWindow::Quit()
{
    if (mHasChanged) {

        int ret = QMessageBox::question(this, tr("Confirm"),
                    tr("Unsaved changes!\nSave file before closing?"),
                    QMessageBox::Save | QMessageBox::Discard
                    | QMessageBox::Cancel,
                    QMessageBox::Cancel);

        switch (ret) {
          case QMessageBox::Save:
              this->saveFile();
              break;
          case QMessageBox::Discard:
              break;
          case QMessageBox::Cancel:
              return;
        }
    }
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->Quit();
}

void MainWindow::openFile(void)
{
    QString fileName(QFileDialog::getOpenFileName(this,
                       tr("Open Tune File"), "", tr("Tune Files (*.xml)")));

    this->openFile(fileName);
}

void MainWindow::openFile(const QString &filename)
{
    if (filename.isEmpty())
        return;

    qWarning() << "Opening" << filename;

    QStringList files = mSettings.value(SETTINGS_RECENT_FILES).toStringList();

    mCurrentFile = filename;
    files.removeAll(filename);
    files.prepend(filename);
    while (files.size() > MAX_RECENT_FILES)
             files.removeLast();

    mSettings.setValue(SETTINGS_RECENT_FILES, files);
    this->updateRecentFilesActions();

    QFile file(mCurrentFile);

    if (!file.open(QFile::ReadOnly))
    {
        mMainUi->statusBar->showMessage(
                    tr("Failed to open file for reading!"));
        return;
    }

    mFile.read(&file);
    file.close();

    this->importFromMTLFile();
}

void MainWindow::saveFile(void)
{
    if (mCurrentFile.length() == 0)
    {
        this->saveFileAs();
        return;
    }

    QFile file(mCurrentFile);

    qWarning() << "Saving" << mCurrentFile;
    if (!file.open(QFile::WriteOnly))
    {
        mMainUi->statusBar->showMessage(
                    tr("Failed to open file for writing!"));
        qWarning() << tr("Failed to open file for writing!") << mCurrentFile;
        return;
    }

    file.seek(0);
    this->exportToMTLFile();
    mFile.write(&file);
    file.close();
    mMainUi->statusBar->showMessage(
                tr("File saved."));

    mHasChanged = false;
}

void MainWindow::saveFileAs(void)
{
    QString fileName(QFileDialog::getSaveFileName(this,
                       tr("Save Tune File"), "", tr("Tune Files (*.xml)")));

    if (fileName.isEmpty())
    {
        return;
    }
    mCurrentFile = fileName;

    this->saveFile();
}

void MainWindow::connectMtl()
{
    if (mMtl->usbConnect())
    {
        mMtl->bootAppIfNeeded();
        this->uiEnable();
        mFastPollingTimer.start();
        mSlowPollingTimer.start();
        mTablesTimer.start();
        mRedrawTimer.start();
        mMainUi->statusBar->showMessage("Connected");
    }
    else {
        mMainUi->statusBar->showMessage(tr("Connection Failed"));
    }
}

void MainWindow::disconnectMtl()
{
    mFastPollingTimer.stop();
    mSlowPollingTimer.stop();
    mTablesTimer.stop();
    mRedrawTimer.stop();
    mMtl->usbDisconnect();
    this->uiDisable();
    mMainUi->statusBar->showMessage("Disconnected");
}

void MainWindow::showAbout()
{
    QMessageBox::information(this,tr("About Motolink"),
       tr("<strong>Version: " __MTL_VER__ "</strong><br/><br/>")+
       tr("Motolink is a smart interface designed for Honda HRC ECUs.<br/><br/>"
       "You can find more information "
       "<a href=\"https://github.com/fpoussin/MotoLink\">here.</a>"));
}

void MainWindow::showUpdateDialog()
{
    mUpdateWizard->showWizard();
}

void MainWindow::importHrc()
{
    QString fileName(QFileDialog::getOpenFileName(this,
                       tr("Import HRC File"), "", tr("HRC File (*.E2P)")));
    mHrc->openFile(fileName);
}

void MainWindow::exportHrc()
{
    QString fileName(QFileDialog::getSaveFileName(this,
                       tr("Export HRC File"), "", tr("HRC File (*.E2P)")));
    mHrc->saveFile(fileName);
}

void MainWindow::setupDefaults(void)
{
    mMainUi->statusBar->showMessage(tr("Disconnected"));

    mTablesModelList.append(&mFuelModel);
    mTablesModelList.append(&mStagingModel);
    mTablesModelList.append(&mAFRModel);
    mTablesModelList.append(&mAFRTgtModel);
    mTablesModelList.append(&mIgnModel);
    mTablesModelList.append(&mKnockModel);

    mTablesViewList.append(mMainUi->tableAfrMap);
    mTablesViewList.append(mMainUi->tableAfrTgt);
    mTablesViewList.append(mMainUi->tableFuel);
    mTablesViewList.append(mMainUi->tableStaging);
    mTablesViewList.append(mMainUi->tableIgnMap);
    mTablesViewList.append(mMainUi->tableKnk);

    mSpinBoxList.append(mMainUi->sbIdle);
    mSpinBoxList.append(mMainUi->sbPitLimiter);
    mSpinBoxList.append(mMainUi->sbShiftLight);
    mSpinBoxList.append(mMainUi->sbThresholdMax);
    mSpinBoxList.append(mMainUi->sbThresholdMin);

    mFuelModel.setName(tr("Fuel"));
    mStagingModel.setName(tr("Staging"));
    mAFRModel.setName(tr("AFR"));
    mAFRTgtModel.setName(tr("AFR Target"));
    mIgnModel.setName(tr("Ignition"));
    mKnockModel.setName(tr("Knock"));

    mAFRModel.setId(1);
    mKnockModel.setId(2);

    mStagingModel.setSingleRow(true);

    mDegreeSuffix.setSuffix(QString::fromUtf8("°"));
    mPercentSuffix.setSuffix("%");

    mMainUi->tableFuel->setItemDelegate(&mPercentSuffix);
    mMainUi->tableStaging->setItemDelegate(&mPercentSuffix);
    mMainUi->tableIgnMap->setItemDelegate(&mDegreeSuffix);
    mMainUi->tableAfrMap->setItemDelegate(&mAfrDisplay);
    mMainUi->tableAfrTgt->setItemDelegate(&mAfrDisplay);

    mMainUi->tableFuel->setModel(&mFuelModel);
    mMainUi->tableStaging->setModel(&mStagingModel);
    mMainUi->tableIgnMap->setModel(&mIgnModel);
    mMainUi->tableAfrMap->setModel(&mAFRModel);
    mMainUi->tableAfrTgt->setModel(&mAFRTgtModel);
    mMainUi->tableKnk->setModel(&mKnockModel);

    mMainUi->tableAfrMap->setMenuReadOnly(true);
    mMainUi->tableKnk->setMenuReadOnly(true);
}

void MainWindow::setupConnections(void)
{
    QObject::connect(mMainUi->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    QObject::connect(mMainUi->actionAbout_QT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QObject::connect(mMainUi->actionQuit, SIGNAL(triggered()), this, SLOT(Quit()));
    QObject::connect(mMainUi->actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    QObject::connect(mMainUi->actionSave, SIGNAL(triggered()), this, SLOT(saveFile()));
    QObject::connect(mMainUi->actionSave_As, SIGNAL(triggered()), this, SLOT(saveFileAs()));
    QObject::connect(mMainUi->actionConnect, SIGNAL(triggered()), this, SLOT(connectMtl()));
    QObject::connect(mMainUi->actionDisconnect, SIGNAL(triggered()), this, SLOT(disconnectMtl()));
    QObject::connect(mMainUi->actionUpdate, SIGNAL(triggered()), this, SLOT(showUpdateDialog()));
    QObject::connect(mMainUi->actionImport, SIGNAL(triggered()), this, SLOT(importHrc()));
    QObject::connect(mMainUi->actionExport, SIGNAL(triggered()), this, SLOT(exportHrc()));
    QObject::connect(mMainUi->actionEnglish, SIGNAL(triggered()), this, SLOT(setLanguageEnglish()));
    QObject::connect(mMainUi->actionFran_ais, SIGNAL(triggered()), this, SLOT(setLanguageFrench()));
    QObject::connect(mMainUi->actionShowHelpIndex, SIGNAL(triggered()), this, SLOT(showHelp()));
    QObject::connect(mMainUi->actionShow_tasks, SIGNAL(triggered()),this, SLOT(showTasks()));
    QObject::connect(mMainUi->actionShow_Knock_Spectrum, SIGNAL(triggered()),this, SLOT(showKnockGraph()));
    QObject::connect(mMainUi->actionShow_Logs, SIGNAL(triggered()), this, SLOT(showLogs()));

    QObject::connect(mMainUi->actionShow_actions, SIGNAL(triggered()), &mUndoView, SLOT(show()));
    QObject::connect(mMainUi->actionUndo, SIGNAL(triggered()), &mUndoStack, SLOT(undo()));
    QObject::connect(mMainUi->actionRedo, SIGNAL(triggered()), &mUndoStack, SLOT(redo()));
    QObject::connect(&mUndoStack, SIGNAL(canRedoChanged(bool)), mMainUi->actionRedo, SLOT(setEnabled(bool)));
    QObject::connect(&mUndoStack, SIGNAL(canUndoChanged(bool)), mMainUi->actionUndo, SLOT(setEnabled(bool)));

    for (int i=0; i<mTablesModelList.size(); i++)
    {
        TableModel* tbl = mTablesModelList.at(i);
        QObject::connect(tbl, SIGNAL(cellValueChanged(int,int)), this, SLOT(onDataChanged()));
        QObject::connect(tbl, SIGNAL(headerDataNeedSync(int,Qt::Orientation,QVariant)),
                         this, SLOT(onHeadersNeedSync(int,Qt::Orientation,QVariant)));
        QObject::connect(tbl->view(), SIGNAL(modelUpdated(QWidget*)), mMainUi->tabMain, SLOT(setCurrentWidget(QWidget*)));
        if (tbl->id() > 0)
            QObject::connect(tbl->view(), SIGNAL(cellCleared(uint,int,int)), mMtl, SLOT(clearCell(uint,int,int)));
    }

    for (int i=0; i<mSpinBoxList.size(); i++)
    {
        QSpinBox * sb = mSpinBoxList.at(i);
        QObject::connect(sb, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));
        QObject::connect(sb, SIGNAL(valueChanged(int)), this, SLOT(onDataChanged()));
    }

    QObject::connect(this, SIGNAL(signalStartupComplete()), &mUpdate, SLOT(getLatestVersion()));
    QObject::connect(&mUpdate, SIGNAL(newVersionAvailable(QString)), this, SLOT(showNewVersionPopup(QString)));
    QObject::connect(mUpdateWizard, SIGNAL(sendDisconnect()), this, SLOT(disconnectMtl()));

    QObject::connect(mMainUi->bTpsSet0, SIGNAL(clicked()), this, SLOT(onSetTps0Pct()));
    QObject::connect(mMainUi->bTpsSet100, SIGNAL(clicked()), this, SLOT(onSetTps100Pct()));

    QObject::connect(mMainUi->bReadMtl, SIGNAL(clicked()), this, SLOT(onReadMtlSettings()));
    QObject::connect(mMainUi->bWriteMtl, SIGNAL(clicked()), this, SLOT(onWriteMtlSettings()));

    for (int i = 0; i < MAX_RECENT_FILES; ++i) {
             mRecentFilesActions[i] = new QAction(this);
             mRecentFilesActions[i]->setVisible(false);
             mRecentFilesActions[i]->setIcon(QIcon("://oxygen/32x32/actions/quickopen-file.png"));
             connect(mRecentFilesActions[i], SIGNAL(triggered()),
                     this, SLOT(openRecenFile()));
             mMainUi->menuRecent_files->addAction(mRecentFilesActions[i]);
         }

    /* Sensors UI update */
    QObject::connect(&mFastPollingTimer, SIGNAL(timeout()), this, SLOT(doFastPolling()));
    QObject::connect(&mSlowPollingTimer, SIGNAL(timeout()), this, SLOT(doSlowPolling()));
    QObject::connect(&mTablesTimer, SIGNAL(timeout()), this, SLOT(doTablesPolling()));
    QObject::connect(&mRedrawTimer, SIGNAL(timeout()), this, SLOT(doSensorsRedraw()));

    QObject::connect(mMtl, SIGNAL(receivedSensors(sensors_data_t const*)), this, SLOT(onSensorsReceived(sensors_data_t const*)));
    QObject::connect(mMtl, SIGNAL(receivedMonitoring(monitor_t const*)), this, SLOT(onMonitoringReceived(monitor_t const*)));
    QObject::connect(mMtl, SIGNAL(receivedKockSpectrum(QByteArray const*)), this, SLOT(onKnockSpectrumReceived(QByteArray const*)));
    QObject::connect(mMtl, SIGNAL(receivedTables(const quint8*,const quint8*)), this, SLOT(onTablesReceived(const quint8*,const quint8*)));
    QObject::connect(mMtl, SIGNAL(communicationError(QString)), this, SLOT(writeLogs(QString)));

    QObject::connect(&mFile, SIGNAL(readFailed(QString)), this, SLOT(onSimpleError(QString)));
}

void MainWindow::setupTabShortcuts()
{
    QSignalMapper *signalMapper = new QSignalMapper(this);

    /* Assign tab shortcuts starting from F5 up to F12 */
    for(int index=0; index < mMainUi->tabMain->count(); ++index)
    {
       if (index > 7) break;
       QShortcut *shortcut = new QShortcut(
                   QKeySequence(QString("F%1").arg(index + 5)), this);

       QObject::connect(shortcut, SIGNAL( activated()),
                         signalMapper, SLOT(map()));
       signalMapper->setMapping(shortcut, index);
    }
    QObject::connect(signalMapper, SIGNAL(mapped(int)),
                     mMainUi->tabMain, SLOT(setCurrentIndex(int)));
}

void MainWindow::setupSettings()
{
    const QString lang = mSettings.value(SETTINGS_LANGUAGE, "English").toString();
    mRecentFiles = mSettings.value(SETTINGS_RECENT_FILES, QStringList()).toStringList();

    if (lang == "French") {
        this->setLanguageFrench();
    }
    else {
        this->setLanguageEnglish();
    }

    this->updateRecentFilesActions();

}

void MainWindow::setupKnockGraph()
{
    QCustomPlot * plot = mKnockGraphUi->mainPlot;
    mKnockFreqLabel = new QCPItemText(plot);

    plot->addGraph();
    plot->xAxis->setLabel(tr("Knock Frequency (Hertz)"));
    plot->yAxis->setLabel(tr("Knock Intensity (Volts, AC)"));
    plot->xAxis->setRange(0, FFT_FREQ);
    plot->yAxis->setRange(0, KNOCK_MAX);

    plot->addItem(mKnockFreqLabel);
    mKnockFreqLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    mKnockFreqLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    mKnockFreqLabel->position->setCoords(0.5, 0); // place position at center/top of axis rect
    mKnockFreqLabel->setText(tr("Loading..."));
    mKnockFreqLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
    mKnockFreqLabel->setPen(QPen(Qt::black)); // show black border around text
}

void MainWindow::retranslate()
{
    mMainUi->retranslateUi(this);
    mUpdateWizard->retranslate();
    mKnockGraphUi->retranslateUi(mKnockGraphWidget);
    mTasksUi->retranslateUi(mTasksWidget);

    mMainUi->tableFuel->retranslate();
    mMainUi->tableStaging->retranslate();
    mMainUi->tableAfrMap->retranslate();
    mMainUi->tableAfrTgt->retranslate();
    mMainUi->tableIgnMap->retranslate();
    mMainUi->tableKnk->retranslate();
}

void MainWindow::setLanguageEnglish()
{
    qApp->removeTranslator(&mTranslator);
    this->retranslate();
    mSettings.setValue("main/language", "English");
}

void MainWindow::setLanguageFrench()
{
    if (!mTranslator.load(":/tr/motolink_fr")) {
        qWarning() << "Failed to load translation";
        return;
    }

    qApp->installTranslator(&mTranslator);
    this->retranslate();
    mSettings.setValue("main/language", "French");
}

void MainWindow::showHelp()
{
    mHelpViewer.show();
}

void MainWindow::uiEnable()
{
    const bool toggle = true;

    mMainUi->actionConnect->setEnabled(!toggle);
    mMainUi->actionDisconnect->setEnabled(toggle);
    mMainUi->actionGet_Configuration->setEnabled(toggle);
    mMainUi->actionSend_Configuration->setEnabled(toggle);
    mMainUi->actionShow_tasks->setEnabled(toggle);
    mMainUi->actionShow_Knock_Spectrum->setEnabled(toggle);
    mMainUi->actionShow_Logs->setEnabled(toggle);
    mMainUi->bReadMtl->setEnabled(toggle);
    mMainUi->bWriteMtl->setEnabled(toggle);
}

void MainWindow::uiDisable()
{
    const bool toggle = false;

    mMainUi->actionConnect->setEnabled(!toggle);
    mMainUi->actionDisconnect->setEnabled(toggle);
    mMainUi->actionGet_Configuration->setEnabled(toggle);
    mMainUi->actionSend_Configuration->setEnabled(toggle);
    mMainUi->actionShow_tasks->setEnabled(toggle);
    mMainUi->actionShow_Knock_Spectrum->setEnabled(toggle);
    mMainUi->actionShow_Logs->setEnabled(toggle);
    mMainUi->bReadMtl->setEnabled(toggle);
    mMainUi->bWriteMtl->setEnabled(toggle);
}

void MainWindow::updateRecentFilesActions()
{
    QStringList files = mSettings.value(SETTINGS_RECENT_FILES).toStringList();
    const uint numRecentFiles = qMin(files.size(), (int)MAX_RECENT_FILES);

    for (uint i = 0; i < numRecentFiles; ++i)
    {
        QString text = QFileInfo(files[i]).fileName();
        mRecentFilesActions[i]->setText(text);
        mRecentFilesActions[i]->setData(files[i]);
        mRecentFilesActions[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MAX_RECENT_FILES; ++j)
        mRecentFilesActions[j]->setVisible(false);
}

void MainWindow::openRecenFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        this->openFile(action->data().toString());
}

void MainWindow::showFuelTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabFuel);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showAFRTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabAfrMap);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showStagingTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabStaging);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showAFRTgtTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabAfrTarget);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showIgnTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabIgnMap);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showKnockTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabKnock);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showSettingsTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabSettings);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showTasks()
{
    mTasksWidget->show();
    mTasksWidget->raise();
}

void MainWindow::showKnockGraph()
{
    mKnockGraphWidget->show();
    mKnockGraphWidget->raise();
}

void MainWindow::showLogs()
{
    mLogsWidget->show();
    mLogsWidget->raise();
}

void MainWindow::writeLogs(const QString &msg)
{
    QString time("[%1] ");
    mLogsUi->list->addItem(time.arg(QTime::currentTime().toString())+msg);
    mLogsUi->list->scrollToBottom();
}

void MainWindow::exportToMTLFile()
{
    mFile.addTable(&mFuelModel);
    mFile.addTable(&mStagingModel);
    mFile.addTable(&mAFRModel);
    mFile.addTable(&mAFRTgtModel);
    mFile.addTable(&mIgnModel);
    mFile.addTable(&mKnockModel);

    mFile.addProperty("Idle", mMainUi->sbIdle->value());
    mFile.addProperty("PitLimiter", mMainUi->sbPitLimiter->value());
    mFile.addProperty("ShiftLight", mMainUi->sbShiftLight->value());
    mFile.addProperty("RpmDiv", mMainUi->dsbRpmDiv->value());
    mFile.addProperty("SpeedDiv", mMainUi->dsbSpeedDiv->value());

    mFile.addProperty("AFRInput", mMainUi->cbAFRInput->currentIndex());

    mFile.addProperty("TPS0",
                      mMainUi->tableSensorTPS->item(0, 0)->data(Qt::EditRole));
    mFile.addProperty("TPS100",
                      mMainUi->tableSensorTPS->item(1, 0)->data(Qt::EditRole));

    mFile.addProperty("AFR0V",
                      mMainUi->tableSensorAFR->item(0, 0)->data(Qt::EditRole));
    mFile.addProperty("AFR5V",
                      mMainUi->tableSensorAFR->item(1, 0)->data(Qt::EditRole));

}

void MainWindow::importFromMTLFile()
{
    QVariant prop;

    mFile.getProperty("Idle", &prop);
    mMainUi->sbIdle->setValue(prop.toInt());

    mFile.getProperty("PitLimiter", &prop);
    mMainUi->sbPitLimiter->setValue(prop.toInt());

    mFile.getProperty("ShiftLight", &prop);
    mMainUi->sbShiftLight->setValue(prop.toInt());

    mFile.getProperty("RpmDiv", &prop);
    mMainUi->dsbRpmDiv->setValue(prop.toDouble());

    mFile.getProperty("SpeedDiv", &prop);
    mMainUi->dsbSpeedDiv->setValue(prop.toDouble());

    mFile.getProperty("AFRInput", &prop);
    mMainUi->cbAFRInput->setCurrentIndex(prop.toInt());

    mFile.getProperty("TPS0", &prop);
    mMainUi->tableSensorTPS->item(0, 0)->setData(Qt::EditRole, prop);
    mFile.getProperty("TPS100", &prop);
    mMainUi->tableSensorTPS->item(1, 0)->setData(Qt::EditRole, prop);

    mFile.getProperty("AFR0V", &prop);
    mMainUi->tableSensorAFR->item(0, 0)->setData(Qt::EditRole, prop);
    mFile.getProperty("AFR5V", &prop);
    mMainUi->tableSensorAFR->item(1, 0)->setData(Qt::EditRole, prop);
}

void MainWindow::doFastPolling()
{
    if (mMtl->isConnected())
    {
        mMtl->readSensors();
        if (mKnockGraphWidget->isVisible())
            mMtl->readKnockSpectrum();
    }
    else {
        mFastPollingTimer.stop();
    }
}

void MainWindow::doSlowPolling()
{
    if (mMtl->isConnected())
    {
        if (mTasksWidget->isVisible())
            mMtl->readMonitoring();
    }
    else {
        mSlowPollingTimer.stop();
    }
}

void MainWindow::doTablesPolling()
{
    if (mMtl->isConnected())
    {
        mMtl->readTables();
    }
    else {
        mTablesTimer.stop();
    }
}

void MainWindow::doSensorsRedraw()
{
    mMainUi->lVbat->setText(QString::number(mSensorsStruct.vAn7)+tr(" Volts"));

    mMainUi->lTpsVolts->setText(QString::number(mSensorsStruct.vAn8)+tr(" Volts"));
    mMainUi->lTpsPct->setText(QString::number(mSensorsStruct.tps)+tr("%"));

    mMainUi->lAfrVal->setText(QString::number(mSensorsStruct.afr));
    mMainUi->lAfrVolts->setText(QString::number(mSensorsStruct.vAn9)+tr(" Volts"));
    mMainUi->lRpm->setText(QString::number(mSensorsStruct.rpm)+tr(" Rpm"));
    mMainUi->lRpmHertz->setText(QString::number(mSensorsStruct.freq1)+tr(" Hertz"));
    mMainUi->lSpeedHertz->setText(QString::number(mSensorsStruct.freq2)+tr(" Hertz"));
}

void MainWindow::onSensorsReceived(sensors_data_t const * data)
{
    mSensorsStruct = *data;

    //this->setTablesCursorFromSensors(mSensorsStruct.tps, mSensorsStruct.rpm);
    //this->setTablesCursor(mSensorsStruct.row, mSensorsStruct.col);
    //mKnockModel.writeCellPeak(mSensorsStruct.tps, mSensorsStruct.rpm, QVariant(mSensorsStruct.knock_value));
    //mAFRModel.writeCellAverage(mSensorsStruct.tps, mSensorsStruct.rpm, QVariant(mSensorsStruct.afr*10.0));
}

void MainWindow::onMonitoringReceived(const monitor_t *monitoring)
{
    const quint16 maskUsage = 0x3FFF; /* Remove thread state in last 2 bits */
    const quint16 maskState = 0x8000; /* Thread state */
    QTableWidgetItem *item;

    item = mTasksUi->tableWidget->item(0, 0);
    item->setData(Qt::DisplayRole, float(monitoring->ser2 & maskUsage)/100.0);
    if (monitoring->ser2 & maskState) item->setBackgroundColor(Qt::white);
    else item->setBackgroundColor(Qt::lightGray);

    item = mTasksUi->tableWidget->item(1, 0);
    item->setData(Qt::DisplayRole, float(monitoring->bdu & maskUsage)/100.0);
    if (monitoring->bdu & maskState) item->setBackgroundColor(Qt::white);
    else item->setBackgroundColor(Qt::lightGray);

    item = mTasksUi->tableWidget->item(2, 0);
    item->setData(Qt::DisplayRole, float(monitoring->sdu & maskUsage)/100.0);
    if (monitoring->sdu & maskState) item->setBackgroundColor(Qt::white);
    else item->setBackgroundColor(Qt::lightGray);

    item = mTasksUi->tableWidget->item(3, 0);
    item->setData(Qt::DisplayRole, float(monitoring->can & maskUsage)/100.0);
    if (monitoring->can & maskState) item->setBackgroundColor(Qt::white);
    else item->setBackgroundColor(Qt::lightGray);

    item = mTasksUi->tableWidget->item(4, 0);
    item->setData(Qt::DisplayRole, float(monitoring->knock & maskUsage)/100.0);
    if (monitoring->knock & maskState) item->setBackgroundColor(Qt::white);
    else item->setBackgroundColor(Qt::lightGray);

    item = mTasksUi->tableWidget->item(5, 0);
    item->setData(Qt::DisplayRole, float(monitoring->sensors & maskUsage)/100.0);
    if (monitoring->sensors & maskState) item->setBackgroundColor(Qt::white);
    else item->setBackgroundColor(Qt::lightGray);

    item = mTasksUi->tableWidget->item(6, 0);
    item->setData(Qt::DisplayRole, float(monitoring->irq & maskUsage)/100.0);
    item->setBackgroundColor(Qt::lightGray);

    item = mTasksUi->tableWidget->item(7, 0);
    item->setData(Qt::DisplayRole, float(monitoring->idle & maskUsage)/100.0);
    if (monitoring->idle & maskState) item->setBackgroundColor(Qt::white);
    else item->setBackgroundColor(Qt::lightGray);

}

void MainWindow::onKnockSpectrumReceived(const QByteArray *data)
{
    QVector<double> x(SPECTRUM_SIZE), y(SPECTRUM_SIZE);
    QCustomPlot * plot = mKnockGraphUi->mainPlot;
    double max_val = 0;
    double max_freq = 0;

    for (uint i=4; i<SPECTRUM_SIZE; i++)
    {
        x[i] = ((FFT_FREQ*i)/SPECTRUM_SIZE);
        y[i] = (KNOCK_MAX/256.0)*(uchar)data->at(i);
        if (y[i] > max_val) {
            max_val = y[i];
            max_freq = x[i];
        }
    }

    mKnockFreqLabel->setText(QString::number(max_val, 'f', 2)+"V at "+
                             QString::number(max_freq, 'f', 2)+"Hz");

    plot->graph(0)->setData(x, y);
    //plot->yAxis->setRange(0, max_val*1.2);
    plot->replot();
}

void MainWindow::onTablesReceived(const quint8 *afr, const quint8 *knock)
{
    mAFRModel.setDataFromArray(afr);
    mKnockModel.setDataFromArray(knock);
    this->setTablesCursor(mSensorsStruct.row, mSensorsStruct.col);
}

void MainWindow::onSetTps0Pct(void)
{
    float tps = mMtl->getSensors()->vAn8;
    mMainUi->tableSensorTPS->item(0, 0)->setData(Qt::EditRole, QString::number(tps, 'f', 3));
}

void MainWindow::onSetTps100Pct(void)
{
    float tps = mMtl->getSensors()->vAn8;
    mMainUi->tableSensorTPS->item(1, 0)->setData(Qt::EditRole, QString::number(tps, 'f', 3));
}

void MainWindow::onDataChanged()
{    
    if (mFile.isLoading())
        return;

    mHasChanged = true;
    if (mMainUi->actionAutosave->isChecked())
    {
        this->saveFile();
    }

    if (mMainUi->actionAuto_Send->isChecked())
    {

    }
}

void MainWindow::onHeadersNeedSync(int section, Qt::Orientation orientation, const QVariant value)
{
    const int role = Qt::UserRole;
    mFuelModel.setHeaderData(section, orientation, value, role);
    mStagingModel.setHeaderData(section, orientation, value, role);
    mAFRModel.setHeaderData(section, orientation, value, role);
    mAFRTgtModel.setHeaderData(section, orientation, value, role);
    mIgnModel.setHeaderData(section, orientation, value, role);
    mKnockModel.setHeaderData(section, orientation, value, role);

    //mMtl->writeTablesHeaders(row, cols);
}

void MainWindow::onSimpleError(QString error)
{
    QMessageBox::critical(this, "Error", error);
}

void MainWindow::showNewVersionPopup(QString version)
{
    QMessageBox::information(this,
         tr("New version available"),
         tr("There is a new version available for download: ")+version
         +tr("<br/>You are currently using: ")+__MTL_VER__
         +"<br/><br/><a href='https://github.com/fpoussin/MotoLink/releases/latest'>"
                             +tr("Download here")+"</a>");
}

void MainWindow::setTablesCursorFromSensors(uint tps, uint rpm)
{
    int row, col;
    for (int i=0; i<mTablesModelList.size(); i++)
    {
        TableModel* tbl = mTablesModelList.at(i);
        if (tbl && tbl->getCell(tps, rpm, &row, &col))
        {
            tbl->highlightCell(row, col);
        }
    }
}

void MainWindow::setTablesCursor(uint row, uint col)
{
    for (int i=0; i<mTablesModelList.size(); i++)
    {
        TableModel* tbl = mTablesModelList.at(i);
        if (tbl)
        {
            tbl->highlightCell(row, col);
        }
    }
}

void MainWindow::onReadMtlSettings()
{

}

void MainWindow::onWriteMtlSettings()
{

}
