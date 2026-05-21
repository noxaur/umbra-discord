#include <QTest>
#include "core/types/snowflake.h"

class TestSnowflake : public QObject
{
    Q_OBJECT
private slots:
    void testParseValidSnowflake();
    void testInvalidSnowflake();
    void testSnowflakeComparison();
};

void TestSnowflake::testParseValidSnowflake()
{
    Snowflake sf("123456789012345678");
    QCOMPARE(sf.toString(), QString("123456789012345678"));

    QDateTime ts = sf.timestamp();
    QVERIFY(ts.isValid());
    QVERIFY(ts.date().year() > 2014);
}

void TestSnowflake::testInvalidSnowflake()
{
    Snowflake empty("");
    QVERIFY(empty.toString().isEmpty());

    Snowflake defaultSf;
    QVERIFY(defaultSf.toString().isEmpty());
}

void TestSnowflake::testSnowflakeComparison()
{
    Snowflake older("100000000000000000");
    Snowflake newer("200000000000000000");
    Snowflake same("100000000000000000");

    QVERIFY2(older < newer, "Older snowflake should be less than newer");
    QVERIFY2(!(newer < older), "Newer should not be less than older");
    QVERIFY2(older == same, "Same values should be equal");
    QVERIFY2(older != newer, "Different values should not be equal");
}
QTEST_MAIN(TestSnowflake)
#include "test_snowflake.moc"
