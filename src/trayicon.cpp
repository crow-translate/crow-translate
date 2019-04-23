#include "trayicon.h"
#include "appsettings.h"
#include "singleapplication.h"
#include "mainwindow.h"

#include <QDBusInterface>
#include <QFileInfo>

TrayIcon::TrayIcon(MainWindow *parent) :
    QSystemTrayIcon(parent)
{
    connect(this, &TrayIcon::activated, this, &TrayIcon::processTrayActivated);
}

void TrayIcon::loadSettings(const AppSettings &settings)
{
    const IconType iconType = settings.trayIconType();

    QIcon icon;
    if (iconType == CustomIcon) {
        const QString customIconName = settings.customIconPath();
        icon = customTrayIcon(customIconName);
        if (icon.isNull()) {
            const QString defaultIconName = trayIconName(DefaultIcon);
            showNotification(tr("The specified icon '%1' for the Crow Translate is invalid. The default icon will be used.").arg(customIconName), defaultIconName);
            icon = QIcon(defaultIconName);
        }
    } else {
        icon = QIcon::fromTheme(trayIconName(iconType));
    }
    setIcon(icon);

    const bool trayIconVisible = settings.isTrayIconVisible();
    setVisible(trayIconVisible);
    SingleApplication::setQuitOnLastWindowClosed(!trayIconVisible);
}

void TrayIcon::showNotification(const QString &message, const QString &iconName, int interval)
{
    QDBusInterface notify("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantList notifyArguments;
    notifyArguments << SingleApplication::applicationName(); // Set program name
    notifyArguments << QVariant(QVariant::UInt);
    notifyArguments << iconName; // Icon
    notifyArguments << SingleApplication::applicationName(); // Title
    notifyArguments << message; // Body
    notifyArguments << QStringList();
    notifyArguments << QVariantMap();
    notifyArguments << interval; // Show interval
    notify.callWithArgumentList(QDBus::AutoDetect, "Notify", notifyArguments);
}

QIcon TrayIcon::customTrayIcon(const QString &customName)
{
    if (QIcon::hasThemeIcon(customName))
        return QIcon::fromTheme(customName);
    if (QFileInfo::exists(customName))
        return QIcon(customName);

    return QIcon();
}

QString TrayIcon::trayIconName(TrayIcon::IconType type)
{
    switch (type) {
    case DefaultIcon:
        return QStringLiteral("crow-translate-tray");
    case DarkIcon:
        return QStringLiteral("crow-translate-tray-dark");
    case LightIcon:
        return QStringLiteral("crow-translate-tray-light");
    default:
        return QString();
    }
}

void TrayIcon::processTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    auto *mainWindow = qobject_cast<MainWindow *>(parent());

    if (reason != QSystemTrayIcon::Trigger)
        return;

    if (!mainWindow->isVisible())
        mainWindow->activate();
    else
        mainWindow->hide();
}
