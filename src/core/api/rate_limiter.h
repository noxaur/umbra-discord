#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H

#include <QHash>
#include <QString>
#include <QTimer>
#include <QObject>

class RateLimiter : public QObject
{
    Q_OBJECT

public:
    struct Bucket
    {
        int limit = 0;
        int remaining = 0;
        qint64 resetAfterMs = 0;
        QTimer *releaseTimer = nullptr;
        bool isGlobal = false;
    };

    explicit RateLimiter(QObject *parent = nullptr);
    ~RateLimiter();

    bool canProceed(const QString &bucket);
    void recordResponse(const QString &bucket, int limit, int remaining, qint64 resetAfterMs, bool isGlobal);
    void handle429(qint64 retryAfterMs, bool isGlobal);
    qint64 retryAfterMs(const QString &bucket);
    QString bucketKey(const QString &endpoint) const;

signals:
    void bucketReady(const QString &bucket);

private:
    QHash<QString, Bucket> m_buckets;
    bool m_globalLimited = false;
    QTimer *m_globalTimer = nullptr;
};

#endif // RATE_LIMITER_H
