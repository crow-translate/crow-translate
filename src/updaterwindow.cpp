/*
 *  Copyright © 2018 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "updaterwindow.h"
#include "ui_updaterwindow.h"

#include "singleapplication.h"

#include <QFile>
#include <QStandardPaths>
#include <QProcess>

UpdaterWindow::UpdaterWindow(QGitTag *release, int installer, QWidget *parent) :
    QWidget(parent, Qt::Dialog),
    ui(new Ui::UpdaterWindow)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    m_network.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
#endif

    // Hide the download progress until download begins
    ui->downloadBar->setVisible(false);
    ui->cancelDownloadButton->setVisible(false);

    // Get download information
    m_downloadUrl = release->assets().at(installer).url();
    m_downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + release->assets().at(installer).name();

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
    changelog.replace("**Added**", "<b>Added</b><ul>");
    changelog.replace("**Changed**", "</ul><b>Changed</b><ul>");
    changelog.replace("-   ", "<li>");
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
    m_reply = m_network.get(QNetworkRequest(m_downloadUrl));
#else
    QNetworkRequest request(m_downloadUrl);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    m_reply = m_network->get(request);
#endif

    connect(m_reply, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal != 0) // May be 0 if network disabled
            ui->downloadBar->setValue(static_cast<int>(bytesReceived * 100 / bytesTotal));
    });

    // Show download progress ui
    ui->downloadBar->setVisible(true);
    ui->cancelDownloadButton->setVisible(true);

    // Wait until download ends
    QEventLoop loop;
    connect(m_reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (m_reply->error()) {
        // Show error
        ui->updateStatusLabel->setText(m_reply->errorString());
        ui->downloadBar->setVisible(false);
        ui->downloadBar->setValue(0);
        ui->cancelDownloadButton->setVisible(false);
        ui->downloadButton->setEnabled(true);
    } else {
        QFile installer(m_downloadPath);
        if (installer.open(QFile::WriteOnly)) {
            // Save file
            installer.write(m_reply->readAll());
            installer.close();
            ui->updateStatusLabel->setText(tr("Downloading is complete"));
            ui->cancelDownloadButton->setEnabled(false);
            ui->installButton->setEnabled(true);
        } else {
            // Show error
            ui->updateStatusLabel->setText(tr("Unable to write file"));
            ui->downloadBar->setVisible(false);
            ui->downloadBar->setValue(0);
            ui->cancelDownloadButton->setVisible(false);
            ui->downloadButton->setEnabled(true);
        }
    }

    delete m_reply;
}

void UpdaterWindow::on_installButton_clicked()
{
    QProcess::startDetached(m_downloadPath);
    SingleApplication::exit();
}

void UpdaterWindow::on_updateLaterButton_clicked()
{
    this->close();
}

void UpdaterWindow::on_cancelDownloadButton_clicked()
{
    m_reply->abort();
    ui->downloadBar->setVisible(false);
    ui->downloadBar->setValue(0);
    ui->cancelDownloadButton->setVisible(false);
}
