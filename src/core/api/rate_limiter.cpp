#include "rate_limiter.h"
#include <QRegularExpression>

RateLimiter::RateLimiter(QObject *parent)
    : QObject(parent)
{
    m_globalTimer = new QTimer(this);
    m_globalTimer->setSingleShot(true);
    connect(m_globalTimer, &QTimer::timeout, this, [this]() {
        m_globalLimited = false;
        emit bucketReady(QStringLiteral("global"));
    });
}

RateLimiter::~RateLimiter()
{
}

QString RateLimiter::bucketKey(const QString &endpoint) const
{
    QString path = endpoint;
    int queryIdx = path.indexOf(QLatin1Char('?'));
    if (queryIdx >= 0)
        path = path.left(queryIdx);

    static const QRegularExpression majorParamRegex(
        QStringLiteral(R"((^/(?:channels|guilds|webhooks)/)(\d+)(.*))"));
    auto match = majorParamRegex.match(path);
    if (match.hasMatch())
        return match.captured(0);

    return path;
}

bool RateLimiter::canProceed(const QString &bucket)
{
    if (m_globalLimited)
        return false;

    auto it = m_buckets.constFind(bucket);
    if (it != m_buckets.constEnd())
    {
        if (it->remaining <= 0 && it->releaseTimer && it->releaseTimer->isActive())
            return false;

        if (it->remaining > 0)
            m_buckets[bucket].remaining--;
    }

    return true;
}

void RateLimiter::recordResponse(const QString &bucket, int limit, int remaining, qint64 resetAfterMs, bool isGlobal)
{
    if (isGlobal && remaining <= 0)
    {
        m_globalLimited = true;
        m_globalTimer->start(resetAfterMs);
        return;
    }

    if (bucket.isEmpty())
        return;

    auto &b = m_buckets[bucket];
    b.limit = limit;
    b.remaining = remaining;
    b.resetAfterMs = resetAfterMs;
    b.isGlobal = isGlobal;

    if (remaining <= 0 && resetAfterMs > 0)
    {
        if (!b.releaseTimer)
        {
            b.releaseTimer = new QTimer(this);
            b.releaseTimer->setSingleShot(true);
            connect(b.releaseTimer, &QTimer::timeout, this, [this, bucket]() {
                auto it = m_buckets.find(bucket);
                if (it != m_buckets.end())
                    it->remaining = it->limit;
                emit bucketReady(bucket);
            });
        }
        b.releaseTimer->start(resetAfterMs);
    }
}

void RateLimiter::handle429(qint64 retryAfterMs, bool isGlobal)
{
    if (isGlobal)
    {
        m_globalLimited = true;
        m_globalTimer->start(retryAfterMs);
    }
}

qint64 RateLimiter::retryAfterMs(const QString &bucket)
{
    if (m_globalLimited && m_globalTimer->isActive())
        return m_globalTimer->remainingTime();

    auto it = m_buckets.constFind(bucket);
    if (it != m_buckets.constEnd())
    {
        if (it->releaseTimer && it->releaseTimer->isActive())
            return it->releaseTimer->remainingTime();
    }

    return 0;
}
