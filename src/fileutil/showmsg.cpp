
#include "showmsg.h"

ShowMsg::ShowMsg() {
}

ShowMsg::~ShowMsg() {
}

/**
 * Show a type of message.
 * E.G. ShowGuiMessage("info", "Message", "Everything is AlRight!!");
 * @param msgtype
 * @param msgtitle
 * @param message
 */
void ShowMsg::ShowGuiMessage(QString msgtype, QString msgtitle, QString message) {

    if (msgtype == INFO) {
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle(msgtitle);
        msgBox.setText(message);
        //
        msgBox.exec();
    } else if (msgtype == WARN) {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(msgtitle);
        msgBox.setText(message);
        //
        msgBox.exec();
    } else if (msgtype == ERROR) {
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(msgtitle);
        msgBox.setText(message);
        //
        msgBox.exec();
    }
}

QString ShowMsg::getMsg() const
{
    return msg;
}

void ShowMsg::setMsg(const QString &newMsg)
{
    msg = newMsg;
}

QString ShowMsg::getMsgType() const
{
    return msgType;
}

void ShowMsg::setMsgType(const QString &newMsgType)
{
    msgType = newMsgType;
}
