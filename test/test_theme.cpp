#include <QTest>
#include "core/ui/theme.h"

class TestTheme : public QObject
{
    Q_OBJECT
private slots:
    void testRaycastQSSNotEmpty();
    void testDiscordQSSNotEmpty();
    void testRaycastMessageCSSNotEmpty();
    void testDiscordMessageCSSNotEmpty();
};

void TestTheme::testRaycastQSSNotEmpty()
{
    Theme theme = Theme::raycast();
    QString qss = theme.generateQSS();
    QVERIFY2(!qss.isEmpty(), "Raycast QSS should not be empty");
    QVERIFY2(qss.contains("background-color") || qss.contains("background"), "Raycast QSS should contain background styling");
    QVERIFY2(qss.contains("QWidget") || qss.contains("QMainWindow"), "Raycast QSS should style Qt widgets");
}

void TestTheme::testDiscordQSSNotEmpty()
{
    Theme theme = Theme::discord();
    QString qss = theme.generateQSS();
    QVERIFY2(!qss.isEmpty(), "Discord QSS should not be empty");
    QVERIFY2(qss.contains("background-color") || qss.contains("background"), "Discord QSS should contain background styling");
    QVERIFY2(qss.contains("QWidget") || qss.contains("QMainWindow"), "Discord QSS should style Qt widgets");
}

void TestTheme::testRaycastMessageCSSNotEmpty()
{
    Theme theme = Theme::raycast();
    QString css = theme.generateMessageCSS();
    QVERIFY2(!css.isEmpty(), "Raycast message CSS should not be empty");
    QVERIFY2(css.contains(".message") || css.contains("message"), "Raycast message CSS should contain message styling");
}

void TestTheme::testDiscordMessageCSSNotEmpty()
{
    Theme theme = Theme::discord();
    QString css = theme.generateMessageCSS();
    QVERIFY2(!css.isEmpty(), "Discord message CSS should not be empty");
    QVERIFY2(css.contains(".message") || css.contains("message"), "Discord message CSS should contain message styling");
}
QTEST_MAIN(TestTheme)
#include "test_theme.moc"
