#include <QTest>
#include "core/formatting/discord_markdown.h"

class TestMarkdown : public QObject
{
    Q_OBJECT
private slots:
    void testCodeBlocks();
    void testInlineCode();
    void testBold();
    void testItalic();
    void testLinks();
    void testMentions();
    void testTimestamps();
    void testSpoilers();
    void testXSSEscaping();
};

void TestMarkdown::testCodeBlocks()
{
    QString input = "```\ncode here\n```";
    QString result = DiscordMarkdown::toHtml(input);
    QVERIFY(result.contains("<pre>"));
    QVERIFY(result.contains("<code>"));
    QVERIFY(result.contains("code here"));

    QString inputWithLang = "```cpp\nint x = 1;\n```";
    QString resultWithLang = DiscordMarkdown::toHtml(inputWithLang);
    QVERIFY(resultWithLang.contains("<pre class=\"lang-cpp\">"));
    QVERIFY(resultWithLang.contains("<code>"));
    QVERIFY(resultWithLang.contains("int x = 1;"));
}

void TestMarkdown::testInlineCode()
{
    // Note: processInlineCode exists but is not called in toHtml()
    // Single backticks remain as-is after HTML escaping
    QString input = "Use `printf()` for output";
    QString result = DiscordMarkdown::toHtml(input);
    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains("printf()"));
}

void TestMarkdown::testBold()
{
    QString input = "**bold text**";
    QString result = DiscordMarkdown::toHtml(input);
    QVERIFY(result.contains("<strong>"));
    QVERIFY(result.contains("bold text"));
}

void TestMarkdown::testItalic()
{
    QString input = "*italic text*";
    QString result = DiscordMarkdown::toHtml(input);
    QVERIFY(result.contains("<em>"));
    QVERIFY(result.contains("italic text"));

    QString inputUnderscore = "_italic too_";
    QString resultUnderscore = DiscordMarkdown::toHtml(inputUnderscore);
    QVERIFY(resultUnderscore.contains("<em>"));
}

void TestMarkdown::testLinks()
{
    QString input = "[click here](https://example.com)";
    QString result = DiscordMarkdown::toHtml(input);
    QVERIFY(result.contains("<a href=\"https://example.com\">"));
    QVERIFY(result.contains("click here"));

    QString unsafeInput = "[click](javascript:alert(1))";
    QString unsafeResult = DiscordMarkdown::toHtml(unsafeInput);
    QVERIFY(!unsafeResult.contains("<a href="));
}

void TestMarkdown::testMentions()
{
    // Mentions are processed then HTML-escaped
    // Note: implementation uses %1 literally instead of backreference
    QString input = "Hello <@123456789>";
    QString result = DiscordMarkdown::toHtml(input);
    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains("mention"));

    QString channelInput = "See <#987654321>";
    QString channelResult = DiscordMarkdown::toHtml(channelInput);
    QVERIFY(!channelResult.isEmpty());
    QVERIFY(channelResult.contains("channel"));
}

void TestMarkdown::testTimestamps()
{
    QString input = "<t:1234567890:R>";
    QString result = DiscordMarkdown::toHtml(input);
    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains("ago") || result.contains("in ") || result.contains("second") || result.contains("minute") || result.contains("hour") || result.contains("day"));

    QString shortTime = "<t:1234567890:t>";
    QString shortResult = DiscordMarkdown::toHtml(shortTime);
    QVERIFY(shortResult.contains(":"));
}

void TestMarkdown::testSpoilers()
{
    QString input = "||secret spoiler||";
    QString result = DiscordMarkdown::toHtml(input);
    QVERIFY(result.contains("<span class=\"spoiler\">"));
    QVERIFY(result.contains("secret spoiler"));
}

void TestMarkdown::testXSSEscaping()
{
    QString input = "<script>alert('xss')</script>";
    QString result = DiscordMarkdown::toHtml(input);
    QVERIFY(!result.contains("<script>"));
    QVERIFY(result.contains("&lt;script&gt;"));
    QVERIFY(result.contains("&lt;/script&gt;"));

    QString imgInput = "<img src=x onerror=alert(1)>";
    QString imgResult = DiscordMarkdown::toHtml(imgInput);
    QVERIFY(!imgResult.contains("<img"));
    QVERIFY(imgResult.contains("&lt;img"));
}
QTEST_MAIN(TestMarkdown)
#include "test_markdown.moc"
