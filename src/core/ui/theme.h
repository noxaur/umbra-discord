#ifndef THEME_H
#define THEME_H

#include <QString>
#include <QHash>
#include <QObject>

struct Theme {
    QString name;

    // Surface ladder
    QString canvas;
    QString surface;
    QString surfaceElevated;
    QString surfaceCard;
    QString buttonFg;

    // Text colors
    QString ink;
    QString body;
    QString charcoal;
    QString mute;
    QString ash;
    QString stone;
    QString onDark;
    QString onDarkMute;

    // Borders
    QString hairline;
    QString hairlineSoft;
    QString hairlineStrong;

    // Accents
    QString accentBlue;
    QString accentBlueSoft;
    QString accentRed;
    QString accentRedSoft;
    QString accentGreen;
    QString accentGreenSoft;
    QString accentYellow;
    QString accentYellowSoft;

    // Interactive
    QString primary;
    QString primaryPressed;
    QString onPrimary;

    // Rounded scale (pixels)
    int roundedXS = 4;
    int roundedSM = 6;
    int roundedMD = 8;
    int roundedLG = 10;
    int roundedXL = 16;

    // Font
    QString fontFamily = "Inter, \"SF Pro Text\", \"Segoe UI\", \"Helvetica Neue\", Arial, sans-serif";

    // Factory methods
    static Theme raycast();
    static Theme discord();

    // QSS generation
    QString generateQSS() const;
    QString generateMessageCSS() const;
};

// Theme manager
class ThemeManager : public QObject {
    Q_OBJECT
public:
    static ThemeManager &instance();
    const Theme &currentTheme() const;
    void setTheme(const QString &name);
    void setTheme(const Theme &theme);
    QString regenerateQSS() const;
    QString regenerateMessageCSS() const;

signals:
    void themeChanged(const Theme &theme);

private:
    ThemeManager();
    Theme m_current;
    QHash<QString, Theme> m_themes;
};

#endif // THEME_H
