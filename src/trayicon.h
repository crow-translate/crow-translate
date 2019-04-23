#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>

class AppSettings;
class MainWindow;

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

public:
    enum IconType {
        DefaultIcon,
        LightIcon,
        DarkIcon,
        CustomIcon
    };
    Q_ENUM(IconType)

    TrayIcon(MainWindow *parent = nullptr);

    void loadSettings(const AppSettings &settings);
    void showNotification(const QString &message, const QString &iconName, int interval = 10000);

    static QIcon customTrayIcon(const QString &customName);
    static QString trayIconName(IconType type);

private slots:
    void processTrayActivated(QSystemTrayIcon::ActivationReason reason);
};

#endif // TRAYICON_H
