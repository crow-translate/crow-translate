#include <QFile>
#include <QStandardPaths>
#include <QProcess>

#include "updaterwindow.h"
#include "ui_updaterwindow.h"

UpdaterWindow::UpdaterWindow(const QGitRelease &release, QWidget *parent) :
    QWidget(parent, Qt::Dialog),
    ui(new Ui::UpdaterWindow)
{
    ui->setupUi(this);
    this->setWindowModality(Qt::WindowModal);
    this->setAttribute(Qt::WA_DeleteOnClose);

    // Hide the download progress ui until the download begins
    ui->downloadBar->setVisible(false);
    ui->cancelDownloadButton->setVisible(false);

    // Search for Windows installer
    foreach (auto asset, release.assets()) {
        if (asset.contentType() == "application/x-ms-dos-executable") {
            downloadUrl = asset.url();
            downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + asset.name();
            break;
        }
    }

    // Load release data
    QString info(tr("A new version of Crow Translate is available! Updates add functionality and improve the stability of the application. Most often."));
    info.append("<br><b>" + tr("Current version:") + " </b>" + qApp->applicationVersion());
    info.append("<br><b>" + tr("Latest version:") + " </b>" + release.tagName());
    info.append("<br>" + tr("You can also download the release manually from this") + " <a href=\"" + downloadUrl.toString() + "\"> " + tr("link") + "</a>.");
    ui->infoLabel->setText(info);

    QString changelog = release.body();
    changelog.prepend("<b>" + tr("Changelog:") + "</b><br><br>");
    changelog.replace("### Added", "<b>Added</b><ul>");
    changelog.replace("### Changed", "</ul><b>Changed</b><ul>");
    changelog.replace("- ", "<li>");
    changelog.replace(".\n", "</li>");
    changelog.append("</ul>");
    ui->changelogTextEdit->setText(changelog);
}

UpdaterWindow::~UpdaterWindow()
{
    delete ui;
}

void UpdaterWindow::on_downloadButton_clicked()
{
    ui->downloadButton->setEnabled(false);

    // Send request
    downloadManager = new QNetworkAccessManager(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    downloadManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    downloading = downloadManager->get(QNetworkRequest(downloadUrl));
#else
    QNetworkRequest request(downloadUrl);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    downloading = downloadManager->get(request);
#endif

    connect(downloadManager, &QNetworkAccessManager::finished, this, &UpdaterWindow::finishDownload);
    connect(downloading, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived, qint64 bytesTotal) {
        ui->downloadBar->setValue(bytesReceived * 100 / bytesTotal);
    });

    // Show download progress ui
    ui->downloadBar->setVisible(true);
    ui->cancelDownloadButton->setVisible(true);
}

void UpdaterWindow::on_installButton_clicked()
{
    QProcess::startDetached(downloadPath);
    qApp->exit();
}

void UpdaterWindow::on_updateLaterButton_clicked()
{
    this->close();
}

void UpdaterWindow::on_cancelDownloadButton_clicked()
{
    downloading->abort();
    ui->downloadBar->setVisible(false);
    ui->cancelDownloadButton->setVisible(false);
}

void UpdaterWindow::finishDownload(QNetworkReply *reply)
{
    ui->installButton->setEnabled(true);

    if (reply->error()) {
        // Show error
        ui->updateStatusLabel->setText(reply->errorString());
        ui->downloadButton->setEnabled(true);
    }
    else {
        // Write file
        QFile installer(downloadPath);
        if (installer.open(QFile::WriteOnly)) {
            installer.write(reply->readAll());
            installer.close();
            ui->updateStatusLabel->setText(tr("Downloading is complete"));
        }
        else {
            ui->updateStatusLabel->setText(tr("Unable to write file"));
            ui->downloadBar->setValue(0);
            ui->downloadButton->setEnabled(true);
        }
    }

    ui->cancelDownloadButton->setEnabled(false);
    downloadManager->deleteLater();
}
