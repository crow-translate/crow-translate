#include <QFile>
#include <QStandardPaths>
#include <QProcess>

#include "singleapplication.h"
#include "updaterwindow.h"
#include "ui_updaterwindow.h"

UpdaterWindow::UpdaterWindow(QGitTag *release, int installer, QWidget *parent) :
    QWidget (parent, Qt::Dialog),
    ui (new Ui::UpdaterWindow)
{
    ui->setupUi(this);
    this->setWindowModality(Qt::WindowModal);
    this->setAttribute(Qt::WA_DeleteOnClose);

    // Hide the download progress ui until the download begins
    ui->downloadBar->setVisible(false);
    ui->cancelDownloadButton->setVisible(false);

    // Get download information
    downloadUrl = release->assets().at(installer).url();
    downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + release->assets().at(installer).name();

    // Show release data
    ui->versionsLabel->setText("<b>"
                               + tr("Current version:")
                               + " </b>"
                               + SingleApplication::applicationVersion()
                               + "<br><b>"
                               + tr("Latest version:")
                               + " </b>"
                               + release->tagName());

    QString changelog = release->body();
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
    ui->updateStatusLabel->setText("");

    // Send request
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    downloadManager.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    reply = downloadManager.get(QNetworkRequest(downloadUrl));
#else
    QNetworkRequest request(downloadUrl);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    reply = downloadManager->get(request);
#endif

    connect(reply, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived, qint64 bytesTotal) {
        ui->downloadBar->setValue(static_cast<int>(bytesReceived * 100 / bytesTotal));
    });

    // Show download progress ui
    ui->downloadBar->setVisible(true);
    ui->cancelDownloadButton->setVisible(true);

    // Wait until download
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error()) {
        // Show error
        ui->updateStatusLabel->setText(reply->errorString());
        ui->downloadButton->setEnabled(true);
    } else {
        QFile installer(downloadPath);
        if (installer.open(QFile::WriteOnly)) {
            // Save file
            installer.write(reply->readAll());
            installer.close();
            ui->updateStatusLabel->setText(tr("Downloading is complete"));
            ui->installButton->setEnabled(true);
        } else {
            // Show error
            ui->updateStatusLabel->setText(tr("Unable to write file"));
            ui->downloadBar->setValue(0);
            ui->downloadButton->setEnabled(true);
        }
    }

    delete reply;
}

void UpdaterWindow::on_installButton_clicked()
{
    QProcess::startDetached(downloadPath);
    SingleApplication::exit();
}

void UpdaterWindow::on_updateLaterButton_clicked()
{
    this->close();
}

void UpdaterWindow::on_cancelDownloadButton_clicked()
{
    reply->abort();
    ui->downloadBar->setVisible(false);
    ui->downloadBar->setValue(0);
    ui->cancelDownloadButton->setVisible(false);
}
