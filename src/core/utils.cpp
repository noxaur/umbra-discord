#include "core/utils.h"
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QRegularExpression>

QPixmap generateGuildAvatar(const QString &name, const QColor &color, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);

    QString initials = name.trimmed();
    if (initials.isEmpty()) {
        initials = "?";
    } else if (initials.length() == 1) {
        initials = initials.toUpper();
    } else {
        QStringList words = initials.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (words.size() >= 2) {
            initials = QString(words.first().at(0)) + QString(words.last().at(0));
            initials = initials.toUpper();
        } else {
            initials = initials.left(2).toUpper();
        }
    }

    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(size * 0.4);
    painter.setFont(font);
    painter.setPen(Qt::white);

    QRect textRect(0, 0, size, size);
    painter.drawText(textRect, Qt::AlignCenter, initials);

    return pixmap;
}
