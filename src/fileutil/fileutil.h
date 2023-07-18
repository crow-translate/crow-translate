#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <QtCore/QCoreApplication>
#include<QTextStream>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QFile>
#include <QDir>
#include <QString>
#include <QDateTime>
#include <QObject>
#include "showmsg.h"
#include "qonlinetranslator.h"
#include "qonlinetts.h"

class ShowMsg;

class FileUtil : public QObject
{
    Q_OBJECT
public:
    explicit FileUtil();
    bool writeDir();
    bool validDir();
    bool validStrDir(QString &str);
    bool validStrFile(QString &str);
    bool validStrAudioFileName(QString &str);
    bool validFile();
    void writeTxtFile(const QString &text);
    void writeMp3File(const QString &strurl, const QString &oper, int seq);

    QString getAudioFile() const;
    void setAudioFile(const QString &newAudiofile);
    QString getTextFile() const;
    void setTextFile(const QString &newFiletext);
    QString getWorkSpace() const;
    void setWorkSpace(const QString &newWorkspace);

    QString getTmp_audiofn() const;
    void setTmp_audiofn(const QString &newTmp_audiofn);

    QString getTmp_textfn() const;
    void setTmp_textfn(const QString &newTmp_textfn);

    ShowMsg *msg;
    ShowMsg *getMsg() const;
    void setMsg(ShowMsg *newMsg);

    QString getAudioExt() const;
    void setAudioExt(const QString &newAudioExt);

    QString getTextFileExt() const;
    void setTextFileExt(const QString &newTextFileExt);

private:
    QString workSpace;
    QString audioFile;
    QString audioExt;
    QString textFile;
    QString textFileExt;
    QString tmp_audiofn;
    QString tmp_textfn;

    QNetworkAccessManager manager;

};

#endif // FILEUTIL_H
