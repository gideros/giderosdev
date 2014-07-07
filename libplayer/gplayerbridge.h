#ifndef GPLAYERBRIDGE_H
#define GPLAYERBRIDGE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>

class GPlayerBridge : public QObject
{
    Q_OBJECT
public:
    explicit GPlayerBridge(QObject *parent = 0);


public slots:
    void play(const QString &fileName);

signals:
    void quit();

public slots:

private:
    QNetworkAccessManager manager_;

private:
    void upload(const QString &projectName, const QString &localFile, const QString &remoteFileName);
    void upload(const QString &projectName, const QByteArray &localFile, const QByteArray &data, const QString &remoteFileName);
};

#endif
