#ifndef UPDATERWINDOW_H
#define UPDATERWINDOW_H

#include "qgittag.h"

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Ui {
class UpdaterWindow;
}

class UpdaterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UpdaterWindow(QGitTag *release, int installer, QWidget *parent = nullptr);
    ~UpdaterWindow();

private slots:
    // UI
    void on_downloadButton_clicked();
    void on_installButton_clicked();
    void on_updateLaterButton_clicked();
    void on_cancelDownloadButton_clicked();

private:
    Ui::UpdaterWindow *ui;

    QNetworkAccessManager m_network{this};
    QNetworkReply *m_reply;

    QUrl m_downloadUrl;
    QString m_downloadPath;
};

#endif // UPDATERWINDOW_H
