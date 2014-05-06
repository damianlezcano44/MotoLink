#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QCloseEvent>
#include <QString>
#include <QUsb>
#include <QUndoStack>
#include <QTranslator>
#include <QSettings>
#include <QTextBrowser>
#include <QAction>
#include <QUndoView>

#include "hrc.h"
#include "updatewizard.h"
#include "helpviewer.h"
#include "motolink.h"
#include "bootloader.h"
#include "commands.h"

#define MAX_RECENT_FILES 5
#define SETTINGS_RECENT_FILES "main/recent_files"
#define SETTINGS_LANGUAGE "main/language"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void Quit(void);
    void closeEvent(QCloseEvent *event);
    void openFile(void);
    void openFile(const QString &filename);
    void saveFile(void);
    void saveFileAs(void);
    void connectToEcu(void);
    void disconnectFromEcu(void);
    void showAbout(void);
    void showUpdateDialog(void);
    void importHrc(void);
    void exportHrc(void);


private slots:
    void setLanguageEnglish(void);
    void setLanguageFrench(void);
    void showHelp(void);
    void uiEnable(void);
    void uiDisable(void);
    void updateRecentFilesActions(void);
    void openRecenFile(void);
    void itemChanged (QStandardItem * item);
    void itemActivated (const QModelIndex & index);

private:

    void setupDefaults(void);
    void setupConnections(void);
    void setupTabShortcuts(void);
    void setupSettings(void);
    void makeDefaultModel(void);
    void makeCellColors(QStandardItemModel *model);
    void retranslate(void);
    QColor NumberToColor(float value, float maxValue, bool greenIsNegative = false);

    Ui::MainWindow *mUi;
    QTranslator mTranslator;
    QSettings mSettings;
    QStandardItemModel mDefaultModel;
    QString mCurrentFile;
    quint8 mNumCol;
    quint8 mNumRow;
    bool mHasChanged;
    Hrc mHrc;
    QUsb mUsb;
    Motolink mMtl;
    Bootloader mBtl;
    UpdateWizard mUpdateWizard;
    HelpViewer mHelpViewer;
    QUndoStack mUndoStack; /* TODO */
    QUndoView mUndoView;
    QStringList mRecentFiles;
    QAction *mRecentFilesActions[MAX_RECENT_FILES];
};

#endif // MAINWINDOW_H
