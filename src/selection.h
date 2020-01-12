#ifndef SELECTION_H
#define SELECTION_H

#include <QObject>

#ifdef Q_OS_WIN
class QTimer;
class QMimeData;
#endif

class Selection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Selection)

public:
    ~Selection() override;

    static Selection *instance();

signals:
    void requestedSelectionAvailable(const QString &selection);

public slots:
    void requestSelection();

protected:
    Selection();

private slots:
    void getSelection();

#ifdef Q_OS_WIN
private:
    QScopedPointer<QMimeData> m_originalClipboardData;
    QTimer *m_maxSelectionDelay;
#endif
};

#endif // SELECTION_H
