#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <QtCore/QCoreApplication>
#include<QTextStream>
#include <QFile>
#include <QDir>
#include <QString>
#include <QObject>
#include "showmsg.h"

class ShowMsg;
class FileUtil : public QObject
{
    Q_OBJECT
public:
    explicit FileUtil();

    bool validDir();
    bool validStrDir(QString &str);
    bool validStrFile(QString &str);
    bool validStrAudioFileName(QString &str);


    QString getWorkSpace() const;
    void setWorkSpace(const QString &newWorkspace);

    QString getTmp_audiofn() const;
    void setTmp_audiofn(const QString &newTmp_audiofn);

    QString getTmp_textfn() const;
    void setTmp_textfn(const QString &newTmp_textfn);

    ShowMsg *msg;
    ShowMsg *getMsg() const;
    void setMsg(ShowMsg *newMsg);


private:
    QString workSpace;
    QString tmp_audiofn;
    QString tmp_textfn;

};

#endif // FILEUTIL_H
