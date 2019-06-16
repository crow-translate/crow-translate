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

#ifndef UPDATERWINDOW_H
#define UPDATERWINDOW_H

#include <QWidget>
#include <QUrl>

class QGitTag;
class QNetworkAccessManager;
class QNetworkReply;

namespace Ui {
class UpdaterWindow;
}

class UpdaterWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(UpdaterWindow)

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
    QNetworkAccessManager *m_network;
    QNetworkReply *m_reply;

    QUrl m_downloadUrl;
    QString m_downloadPath;
};

#endif // UPDATERWINDOW_H
