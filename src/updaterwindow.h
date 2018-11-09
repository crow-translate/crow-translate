#ifndef UPDATERWINDOW_H
#define UPDATERWINDOW_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "qgittag.h"

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

    QNetworkAccessManager downloadManager{this};
    QNetworkReply *reply;

    QUrl downloadUrl;
    QString downloadPath;
};

#endif // UPDATERWINDOW_H
