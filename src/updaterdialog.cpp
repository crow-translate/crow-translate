/*
 *  Copyright © 2018-2022 Hennadii Chernyshchyk <genaloner@gmail.com>
 *
 *  This file is part of Crow Translate.
 *
 *  Crow Translate is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crow Translate is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "updaterdialog.h"
#include "ui_updaterdialog.h"

#include "qgittag.h"

#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QStandardPaths>

UpdaterDialog::UpdaterDialog(QGitTag *release, int installer, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdaterDialog)
    , m_network(new QNetworkAccessManager(this))
    , m_installerFile(new QFile(this))
{
    ui->setupUi(this);

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    m_network->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
#endif

    // Hide the download progress until download begins
    ui->downloadBar->setVisible(false);
    ui->cancelDownloadButton->setVisible(false);

    // Get download information
    m_downloadUrl = release->assets().at(installer).url();
    m_downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + QDir::separator() + release->assets().at(installer).name();

    // Show release data
    ui->currentVersionLabel->setText(QCoreApplication::applicationVersion());
    ui->availableVersionLabel->setText(release->tagName());

    QString changelog = release->body();
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    changelog.prepend(QStringLiteral("### %1\n").arg(tr("Changelog:")));
    ui->changelogTextEdit->setMarkdown(changelog);
#else
    changelog.prepend(QStringLiteral("<b>%1</b><br><br>").arg(tr("Changelog:")));
    changelog.replace(QStringLiteral("**Added**"), QStringLiteral("<b>Added</b><ul>"));
    changelog.replace(QStringLiteral("**Changed**"), QStringLiteral("</ul><b>Changed</b><ul>"));
    changelog.replace(QStringLiteral("**Removed**"), QStringLiteral("</ul><b>Removed</b><ul>"));
    changelog.replace(QStringLiteral("-   "), QStringLiteral("<li>"));
    changelog.replace(QStringLiteral(".\n"), QStringLiteral("</li>"));
    changelog.append(QStringLiteral("</ul>"));
    ui->changelogTextEdit->setText(changelog);
#endif
}

UpdaterDialog::~UpdaterDialog()
{
    delete ui;
}

void UpdaterDialog::download()
{
    ui->downloadButton->setEnabled(false);
    ui->updateStatusLabel->clear();
    ui->downloadBar->setVisible(true);
    ui->cancelDownloadButton->setVisible(true);

    // Prepere downloading file
    m_installerFile->setFileName(m_downloadPath);
    if (!m_installerFile->open(QFile::WriteOnly)) {
        setStatus(tr("Unable to write file"), false);
        return;
    }

    // Send request
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    m_reply = m_network->get(QNetworkRequest(m_downloadUrl));
#else
    QNetworkRequest request(m_downloadUrl);
    request->setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    m_reply = m_network->get(request);
#endif

    connect(m_reply, &QNetworkReply::readyRead, this, &UpdaterDialog::saveDownloadedData);
    connect(m_reply, &QNetworkReply::finished, this, &UpdaterDialog::showDownloadStatus);
    connect(m_reply, &QNetworkReply::downloadProgress, [this](qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal != 0) // May be 0 on error
            ui->downloadBar->setValue(static_cast<int>(bytesReceived * 100 / bytesTotal));
    });
}

void UpdaterDialog::cancelDownload()
{
    m_reply->abort();
    ui->downloadBar->setVisible(false);
    ui->downloadBar->setValue(0);
    ui->cancelDownloadButton->setVisible(false);
}

void UpdaterDialog::install()
{
    QProcess::startDetached(m_downloadPath, {});
    QCoreApplication::exit();
}

void UpdaterDialog::showDownloadStatus()
{
    m_installerFile->close();
    m_reply->deleteLater();
    if (m_reply->error()) {
        setStatus(m_reply->errorString(), false);
        return;
    }

    setStatus(tr("Downloading is complete"), true);
}

void UpdaterDialog::saveDownloadedData()
{
    m_installerFile->write(m_reply->readAll());
}

void UpdaterDialog::setStatus(const QString &errorString, bool success)
{
    ui->updateStatusLabel->setText(errorString);

    if (success) {
        ui->cancelDownloadButton->setEnabled(false);
        ui->installButton->setEnabled(true);
    } else {
        ui->downloadBar->setVisible(false);
        ui->downloadBar->reset();
        ui->cancelDownloadButton->setVisible(false);
        ui->downloadButton->setEnabled(true);
    }
}
