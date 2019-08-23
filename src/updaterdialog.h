/*
 *  Copyright © 2018-2019 Hennadii Chernyshchyk <genaloner@gmail.com>
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

#ifndef UPDATERDIALOG_H
#define UPDATERDIALOG_H

#include <QDialog>
#include <QUrl>

class QGitTag;
class QNetworkAccessManager;
class QNetworkReply;

namespace Ui {
class UpdaterDialog;
}

class UpdaterDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdaterDialog)

public:
    explicit UpdaterDialog(QGitTag *release, int installer, QWidget *parent = nullptr);
    ~UpdaterDialog();

private slots:
    void download();
    void cancelDownload();
    void install();

    void parseReply();

private:
    void setStatus(const QString &errorString, bool success);

    Ui::UpdaterDialog *ui;
    QNetworkAccessManager *m_network;
    QNetworkReply *m_reply;

    QUrl m_downloadUrl;
    QString m_downloadPath;
};

#endif // UPDATERDIALOG_H
