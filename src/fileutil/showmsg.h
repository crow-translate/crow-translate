/* 
 * File:   ShowMsg.h
 * Author: batista
 *
 * Created on 8 de Março de 2013, 20:45
 */

#ifndef SHOWMSG_H
#define	SHOWMSG_H
#include <QMessageBox>
#include <QtGui>
#include <QObject>
#include <QDebug>
#include <QString>

#define INFO        "info"
#define WARN        "warn"
#define ERROR       "error"

class QMessageBox;

class ShowMsg {
public:
    ShowMsg();
    virtual ~ShowMsg();
    //
    void ShowGuiMessage(QString msgtype, QString msgtitle, QString message);

    QString getMsg() const;
    void setMsg(const QString &newMsg);

    QString getMsgType() const;
    void setMsgType(const QString &newMsgType);

private:
    QMessageBox msgBox;
    QString msg;
    QString msgType;

};

#endif	/* SHOWMSG_H */

