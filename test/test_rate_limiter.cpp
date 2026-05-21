#include <QTest>
#include "core/api/rate_limiter.h"

class TestRateLimiter : public QObject
{
    Q_OBJECT
private slots:
    void testInitialBucketTracking();
    void testUpdateRateLimitHeaders();
    void testIsRateLimited();
};

void TestRateLimiter::testInitialBucketTracking()
{
    RateLimiter limiter;
    QString bucket = limiter.bucketKey("/api/channels/123/messages");
    QVERIFY2(!bucket.isEmpty(), "Bucket key should not be empty for valid endpoint");
    QVERIFY2(bucket.contains("123"), "Bucket key should contain the major parameter");

    QString bucket2 = limiter.bucketKey("/api/guilds/456/channels");
    QVERIFY2(!bucket2.isEmpty(), "Bucket key should not be empty for guild endpoint");
    QVERIFY2(bucket2.contains("456"), "Bucket key should contain the guild ID");
}

void TestRateLimiter::testUpdateRateLimitHeaders()
{
    RateLimiter limiter;
    QString bucket = "/api/channels/123/messages";

    limiter.recordResponse(bucket, 10, 5, 1000, false);

    bool canGo = limiter.canProceed(bucket);
    QVERIFY2(canGo, "Should be able to proceed when remaining > 0");
}

void TestRateLimiter::testIsRateLimited()
{
    RateLimiter limiter;
    QString bucket = "/api/channels/123/messages";

    limiter.recordResponse(bucket, 10, 0, 5000, false);

    QTRY_VERIFY_WITH_TIMEOUT(!limiter.canProceed(bucket), 100);

    limiter.handle429(1000, true);
    bool canProceed = limiter.canProceed("any");
    QVERIFY2(!canProceed, "Should not proceed when globally rate limited");
}
QTEST_MAIN(TestRateLimiter)
#include "test_rate_limiter.moc"
