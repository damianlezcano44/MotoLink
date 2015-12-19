#include "update.h"

Update::Update(QObject *parent) :
    QObject(parent)
{
    mReleasesUrl.setUrl("https://api.github.com/repos/fpoussin/motolink/releases");
    connect(&mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onResult(QNetworkReply*)));
    mCurrentVersion = __MTL_VER__;
    mNewVersion = mCurrentVersion;
}

void Update::getLatestVersion()
{
    QNetworkRequest request;
    request.setUrl(mReleasesUrl);
    mNetworkManager.get(request);
}


void Update::onResult(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError
            || reply->size() < 1)
        return;

    QString data = (QString) reply->readAll();

    delete reply;

    QScriptEngine engine;
    QScriptValue result = engine.evaluate(data);

    if (!result.isArray())
        return;

    QScriptValueIterator it(result);
    while (it.hasNext())
    {
        it.next();
        QScriptValue release = it.value();

        QString version = release.property("tag_name").toString();
        version.remove(QRegExp("[a-zA-Z]"));

        if (version.isEmpty()) {
            qDebug() << tr("Unable to get latest version") << version;
            return;
        }

        mNewVersion = version;
        if (mCurrentVersion.compare(mNewVersion) < 0)
        {
            qDebug() << QString(tr("New version available: ")) + version;
            emit newVersionAvailable(version);
        }
        else
        {
            qDebug() << QString(tr("Using latest version: ")) + version;
        }

        break; // Only latest release
    }
}
