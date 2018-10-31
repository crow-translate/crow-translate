#ifndef UPDATERWINDOW_H
#define UPDATERWINDOW_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "qgitrelease.h"

namespace Ui {
class UpdaterWindow;
}

class UpdaterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UpdaterWindow(const QGitRelease &release, QWidget *parent = nullptr);
    ~UpdaterWindow();

private slots:
    void on_downloadButton_clicked();
    void on_installButton_clicked();
    void on_updateLaterButton_clicked();
    void on_cancelDownloadButton_clicked();

    void finishDownload(QNetworkReply *downloading);

private:
    Ui::UpdaterWindow *ui;
    QNetworkAccessManager *downloadManager;
    QNetworkReply *downloading;

    QUrl downloadUrl;
    QString downloadPath;
};

#endif // UPDATERWINDOW_H
