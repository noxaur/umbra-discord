#include <QTest>
#include "core/cache.h"
#include "core/types/discord_types.h"

class TestCache : public QObject
{
    Q_OBJECT
private slots:
    void testUpsertAndGetGuild();
    void testDeleteGuild();
    void testUpsertAndGetChannel();
    void testUpsertAndGetMessage();
    void testGuildChannels();
};

void TestCache::testUpsertAndGetGuild()
{
    Cache cache;
    Guild guild;
    guild.id = Snowflake("123456789");
    guild.name = "Test Guild";
    guild.ownerId = Snowflake("987654321");
    guild.unavailable = false;

    cache.upsertGuild(guild);

    auto result = cache.guild(Snowflake("123456789"));
    QVERIFY2(result.has_value(), "Guild should be found after upsert");
    QCOMPARE(result->id.toString(), QString("123456789"));
    QCOMPARE(result->name, QString("Test Guild"));
}

void TestCache::testDeleteGuild()
{
    Cache cache;
    Guild guild;
    guild.id = Snowflake("111222333");
    guild.name = "Delete Me";
    guild.ownerId = Snowflake("444555666");

    cache.upsertGuild(guild);
    QVERIFY(cache.guild(Snowflake("111222333")).has_value());

    QList<Guild> allBefore = cache.allGuilds();
    QVERIFY(!allBefore.isEmpty());

    Guild guild2;
    guild2.id = Snowflake("111222333");
    guild2.name = "Replaced";
    guild2.ownerId = Snowflake("444555666");
    guild2.unavailable = true;
    cache.upsertGuild(guild2);

    auto result = cache.guild(Snowflake("111222333"));
    QVERIFY(result.has_value());
    QCOMPARE(result->unavailable, true);
}

void TestCache::testUpsertAndGetChannel()
{
    Cache cache;
    Channel channel;
    channel.id = Snowflake("777888999");
    channel.name = "general";
    channel.type = ChannelType::Text;
    channel.guildId = Snowflake("123456789");

    cache.upsertChannel(channel);

    auto result = cache.channel(Snowflake("777888999"));
    QVERIFY2(result.has_value(), "Channel should be found after upsert");
    QCOMPARE(result->name, QString("general"));
    QCOMPARE(result->type, ChannelType::Text);
}

void TestCache::testUpsertAndGetMessage()
{
    Cache cache;
    Message msg;
    msg.id = Snowflake("100200300");
    msg.channelId = Snowflake("777888999");
    msg.author.id = Snowflake("444555666");
    msg.author.username = "TestUser";
    msg.content = "Hello world";
    msg.timestamp = QDateTime::currentDateTime();

    cache.insertMessage(msg);

    auto result = cache.message(Snowflake("777888999"), Snowflake("100200300"));
    QVERIFY2(result.has_value(), "Message should be found after insert");
    QCOMPARE(result->content, QString("Hello world"));
    QCOMPARE(result->author.username, QString("TestUser"));
}

void TestCache::testGuildChannels()
{
    Cache cache;

    Channel ch1;
    ch1.id = Snowflake("111");
    ch1.name = "general";
    ch1.type = ChannelType::Text;
    ch1.guildId = Snowflake("999");

    Channel ch2;
    ch2.id = Snowflake("222");
    ch2.name = "random";
    ch2.type = ChannelType::Text;
    ch2.guildId = Snowflake("999");

    Channel ch3;
    ch3.id = Snowflake("333");
    ch3.name = "other";
    ch3.type = ChannelType::Text;
    ch3.guildId = Snowflake("888");

    cache.upsertChannel(ch1);
    cache.upsertChannel(ch2);
    cache.upsertChannel(ch3);

    QList<Channel> channels = cache.guildChannels(Snowflake("999"));
    QCOMPARE(channels.size(), 2);

    QList<Channel> otherChannels = cache.guildChannels(Snowflake("888"));
    QCOMPARE(otherChannels.size(), 1);
}
QTEST_MAIN(TestCache)
#include "test_cache.moc"
