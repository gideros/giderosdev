#include <gplayerbridge.h>
#include <QDomDocument>
#include <QFile>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QJsonDocument>
#include <gmd5.h>
#include <QJsonArray>
#include <QJsonObject>
#include <stdint.h>
#include <QEventLoop>
#include <QDir>
#include <QFileInfo>
#include <gprojectproperties.h>
#include <gdependencygraph.h>
#include <QStack>
#include <gplatformutil.h>
#include <gutil.h>
#include <QHttpMultiPart>

GPlayerBridge::GPlayerBridge(QObject *parent) :
    QObject(parent)
{
}

static uint8_t nibbleFromChar(char c)
{
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 255;
}

static void hexStringToBytes(const char str[33], uint8_t bytes[16])
{
    int i;

    for (i = 0; i < 16; ++i)
        bytes[i] = (nibbleFromChar(str[i * 2]) << 4) | nibbleFromChar(str[i * 2 + 1]);
}

void GPlayerBridge::play(const QString &fileName)
{
    QDomDocument doc;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
        return;

    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }

    file.close();

    QFileInfo fileInfo(fileName);
    QDir projectDir = fileInfo.dir();
    QString projectName = fileInfo.baseName();

    GProjectProperties properties_;
    QList<QPair<QString, QString> > fileList_;
    GDependencyGraph dependencyGraph_;

    // read properties
    {
        QDomElement root = doc.documentElement();

        properties_.clear();

        QDomElement properties = root.firstChildElement("properties");

        // graphics options
        if (!properties.attribute("scaleMode").isEmpty())
            properties_.scaleMode = properties.attribute("scaleMode").toInt();
        if (!properties.attribute("logicalWidth").isEmpty())
            properties_.logicalWidth = properties.attribute("logicalWidth").toInt();
        if (!properties.attribute("logicalHeight").isEmpty())
            properties_.logicalHeight = properties.attribute("logicalHeight").toInt();
        QDomElement imageScales = properties.firstChildElement("imageScales");
        for(QDomNode n = imageScales.firstChild(); !n.isNull(); n = n.nextSibling())
        {
            QDomElement scale = n.toElement();
            if(!scale.isNull())
                properties_.imageScales.push_back(QPair<QString, double>(scale.attribute("suffix"), scale.attribute("scale").toDouble()));
        }
        if (!properties.attribute("orientation").isEmpty())
            properties_.orientation = properties.attribute("orientation").toInt();
        if (!properties.attribute("fps").isEmpty())
            properties_.fps = properties.attribute("fps").toInt();

        // iOS options
        if (!properties.attribute("retinaDisplay").isEmpty())
            properties_.retinaDisplay = properties.attribute("retinaDisplay").toInt();
        if (!properties.attribute("autorotation").isEmpty())
            properties_.autorotation = properties.attribute("autorotation").toInt();

        // input options
        if (!properties.attribute("mouseToTouch").isEmpty())
            properties_.mouseToTouch = properties.attribute("mouseToTouch").toInt() != 0;
        if (!properties.attribute("touchToMouse").isEmpty())
            properties_.touchToMouse = properties.attribute("touchToMouse").toInt() != 0;
        if (!properties.attribute("mouseTouchOrder").isEmpty())
            properties_.mouseTouchOrder = properties.attribute("mouseTouchOrder").toInt();

        // export options
        if (!properties.attribute("architecture").isEmpty())
            properties_.architecture = properties.attribute("architecture").toInt();
        if (!properties.attribute("assetsOnly").isEmpty())
            properties_.assetsOnly = properties.attribute("assetsOnly").toInt() != 0;
        if (!properties.attribute("iosDevice").isEmpty())
            properties_.iosDevice = properties.attribute("iosDevice").toInt();
        if (!properties.attribute("packageName").isEmpty())
            properties_.packageName = properties.attribute("packageName");
        if (!properties.attribute("encryptCode").isEmpty())
            properties_.encryptCode = properties.attribute("encryptCode").toInt() != 0;
        if (!properties.attribute("encryptAssets").isEmpty())
            properties_.encryptAssets = properties.attribute("encryptAssets").toInt() != 0;
    }

    // populate file list and dependency graph
    {
        fileList_.clear();
        dependencyGraph_.clear();
        QList<QPair<QString, QString> > dependencies_;

        QStack<QDomNode> stack;
        stack.push(doc.documentElement());

        std::vector<QString> dir;

        while (stack.empty() == false)
        {
            QDomNode n = stack.top();
            QDomElement e = n.toElement();
            stack.pop();

            if (n.isNull() == true)
            {
                dir.pop_back();
                continue;
            }

            QString type = e.tagName();

            if (type == "file")
            {
                QString fileName = e.hasAttribute("source") ? e.attribute("source") : e.attribute("file");
                QString name = QFileInfo(fileName).fileName();

                QString n;
                for (std::size_t i = 0; i < dir.size(); ++i)
                    n += dir[i] + "/";
                n += name;

                fileList_.push_back(QPair<QString, QString>(n, fileName));

                if (QFileInfo(fileName).suffix().toLower() == "lua")
                {
                    bool excludeFromExecution = e.hasAttribute("excludeFromExecution") && e.attribute("excludeFromExecution").toInt();
                    dependencyGraph_.addCode(fileName, excludeFromExecution);
                }

                continue;
            }

            if (type == "folder")
            {
                QString name = e.attribute("name");
                dir.push_back(name);

                QString n;
                for (std::size_t i = 0; i < dir.size(); ++i)
                    n += dir[i] + "/";

                stack.push(QDomNode());
            }

            if (type == "dependency")
            {
                QString from = e.attribute("from");
                QString to = e.attribute("to");

                dependencies_.push_back(QPair<QString, QString>(from, to));
            }

            QDomNodeList childNodes = n.childNodes();
            for (int i = 0; i < childNodes.size(); ++i)
                stack.push(childNodes.item(i));
        }

        for (int i = 0; i < dependencies_.size(); ++i)
            dependencyGraph_.addDependency(dependencies_[i].first,dependencies_[i].second);
    }


    QMap<QString, QString> localToRemote;
    QMap<QString, QString> remoteToLocal;

    for (int i = 0; i < fileList_.size(); ++i)
    {
        const QString &remote = fileList_[i].first;
        const QString &local = fileList_[i].second;

        localToRemote[local] = remote;
        remoteToLocal[remote] = local;
    }

    // load md5 file and update it
    GMD5 localFileMap;
    {
        projectDir.mkdir(".tmp");
        std::string md5filename = projectDir.filePath(".tmp/md5.bin").toStdString();
        localFileMap.load(md5filename.c_str());

        bool updated = false;

        // delete unused files
        for (GMD5::iterator iter = localFileMap.begin(); iter != localFileMap.end();)
        {
            if (!localToRemote.contains(QString::fromStdString(iter->first)))
            {
                localFileMap.erase(iter++);
                updated = true;
            }
            else
            {
                ++iter;
            }
        }

        // update files with different time stamp
        for (int i = 0; i < fileList_.size(); ++i)
        {
            std::string filename = fileList_[i].second.toStdString();
            std::string fullpath = projectDir.filePath(fileList_[i].second).toStdString();

            time_t time = gFileLastModifiedTime(fullpath.c_str());

            GMD5::iterator iter = localFileMap.find(filename);

            if (iter == localFileMap.end() || time != iter->second.first)
            {
                if (localFileMap.update(filename.c_str(), fullpath.c_str()))
                    updated = true;
            }
        }

        if (updated)
            localFileMap.save(md5filename.c_str());
    }

    // get remote file map
    GMD5 remoteFileMap;
    {
        QString url = "http://localhost:15000/list/" + projectName;
        QNetworkReply *reply = manager_.get(QNetworkRequest(QUrl(url)));

        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        delete reply;

        QJsonArray array = doc.array();
        for (int i = 0; i < array.size(); ++i)
        {
            QJsonObject obj = array[i].toObject();

            std::vector<uint8_t> md5v(16);

            std::string file = obj.value("file").toString().toStdString();
            time_t t = (time_t)obj.value("age").toDouble();
            hexStringToBytes(obj.value("md5").toString().toUtf8().constData(), &md5v[0]);

            remoteFileMap[file] = std::make_pair(t, md5v);
        }
    }

    // delete unused files
    for (GMD5::iterator iter = remoteFileMap.begin(); iter != remoteFileMap.end(); ++iter)
    {
        QString remoteFile = QString::fromStdString(iter->first);
        std::string localFile = remoteToLocal[remoteFile].toStdString();

        if (localFileMap.find(localFile) == localFileMap.end())
        {
            QString url = "http://localhost:15000/delete/" + projectName + "/" + remoteFile;

            QNetworkReply *reply = manager_.get(QNetworkRequest(QUrl(url)));

            QEventLoop loop;
            connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
            loop.exec();
        }
    }

    // upload properties.gideros
    {
        QJsonObject properties;
        properties["scaleMode"] = properties_.scaleMode;
        properties["logicalWidth"] = properties_.logicalWidth;
        properties["logicalHeight"] = properties_.logicalHeight;

        QJsonArray imageScales;
        for (int i = 0; i < properties_.imageScales.size(); ++i)
        {
            QJsonObject imageScale;
            imageScale["suffix"] = properties_.imageScales[i].first;
            imageScale["scale"] = properties_.imageScales[i].second;
            imageScales.append(imageScale);
        }
        properties["imageScales"] = imageScales;

        properties["orientation"] = properties_.orientation;
        properties["fps"] = properties_.fps;

        properties["retinaDisplay"] = properties_.retinaDisplay;
        properties["autorotation"] = properties_.autorotation;

        properties["mouseToTouch"] = properties_.mouseToTouch;
        properties["touchToMouse"] = properties_.touchToMouse;
        properties["mouseTouchOrder"] = properties_.mouseTouchOrder;


        QJsonArray luaFiles;

        std::vector<std::pair<QString, bool> > topologicalSort = dependencyGraph_.topologicalSort();
        for (std::size_t i = 0; i < topologicalSort.size(); ++i)
            if (topologicalSort[i].second == false)
                luaFiles.append(localToRemote[topologicalSort[i].first]);

        QJsonObject root;
        root["properties"] = properties;
        root["luaFiles"] = luaFiles;

        QJsonDocument doc;
        doc.setObject(root);

        upload(projectName, "properties.gideros", doc.toJson(), "");
    }

    // upload files with different md5 and younger age
    for (GMD5::iterator iter = localFileMap.begin(); iter != localFileMap.end(); ++iter)
    {
        QString fileName = QString::fromStdString(iter->first);
        QString remoteFileName = localToRemote[fileName];
        QString fullPath = projectDir.filePath(fileName);

        GMD5::iterator iter2 = remoteFileMap.find(remoteFileName.toStdString());

        bool send = false;

        if (iter == remoteFileMap.end())
        {
            send = true;
        }
        else
        {
            time_t localAge = time(NULL) - iter->second.first;
            time_t remoteAge = iter2->second.first;
            const std::vector<uint8_t> &localMD5 = iter->second.second;
            const std::vector<uint8_t> &remoteMD5 = iter2->second.second;

            if (localAge < remoteAge || localMD5 != remoteMD5)
                send = true;
        }

        if (send)
            upload(projectName, fullPath, QFileInfo(remoteFileName).path());
    }


    // send play
    {
        QString url = "http://localhost:15000/play/" + projectName;

        QNetworkReply *reply = manager_.get(QNetworkRequest(QUrl(url)));

        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
    }

    emit quit();
}

void GPlayerBridge::upload(const QString &projectName, const QString &localFile, const QString &remoteDir)
{
    printf("uploading %s\n", QFileInfo(localFile).fileName().toUtf8().constData());

    QHttpMultiPart multiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(localFile).fileName() + "\""));

    QFile file(localFile);

    if (!file.open(QIODevice::ReadOnly))
    {
        printf("file not found\n");
        return;
    }

    filePart.setBodyDevice(&file);

    multiPart.append(filePart);

    QString url = "http://localhost:15000/upload/" + projectName + "/" + remoteDir;

    QNetworkReply *reply = manager_.post(QNetworkRequest(QUrl(url)), &multiPart);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}


void GPlayerBridge::upload(const QString &projectName, const QByteArray &localFile, const QByteArray &data, const QString &remoteDir)
{
    QHttpMultiPart multiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(localFile).fileName() + "\""));

    filePart.setBody(data);

    multiPart.append(filePart);

    QString url = "http://localhost:15000/upload/" + projectName + "/" + remoteDir;

    QNetworkReply *reply = manager_.post(QNetworkRequest(QUrl(url)), &multiPart);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}

