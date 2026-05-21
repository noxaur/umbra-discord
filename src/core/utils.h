#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QColor>
#include <QPixmap>

inline bool isSafeUrl(const QString &url)
{
    QString lower = url.toLower().trimmed();
    return lower.startsWith("http://") || lower.startsWith("https://") || lower.startsWith("/");
}

inline QColor hashColor(const QString &id)
{
    uint h = 0;
    for (const QChar &c : id) {
        h = h * 31 + c.unicode();
    }
    static const QColor palette[] = {
        QColor("#57c1ff"), QColor("#ff6161"), QColor("#59d499"),
        QColor("#ffc533"), QColor("#c78dff"), QColor("#ff8c42"),
        QColor("#42d9a8"), QColor("#f472b6"), QColor("#60a5fa"),
        QColor("#a78bfa"), QColor("#34d399"), QColor("#fbbf24"),
    };
    return palette[h % 12];
}

QPixmap generateGuildAvatar(const QString &name, const QColor &color, int size = 32);

#endif // UTILS_H
