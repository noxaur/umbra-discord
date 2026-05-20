#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H

#include <QHash>
#include <QString>
#include <QTimer>
#include <QObject>
#include <QQueue>
#include <functional>

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
    void queueRequest(const QString &bucket, std::function<void()> callback);
    void recordResponse(const QString &bucket, int limit, int remaining, qint64 resetAfterMs, bool isGlobal);
    void handle429(qint64 retryAfterMs, bool isGlobal);
    qint64 retryAfterMs(const QString &bucket);
    QString bucketKey(const QString &endpoint) const;

    static constexpr int kMaxQueueSize = 100;

signals:
    void bucketReady(const QString &bucket);

private:
    void processQueue();

    QHash<QString, Bucket> m_buckets;
    bool m_globalLimited = false;
    QTimer *m_globalTimer = nullptr;

    struct QueuedRequest
    {
        QString bucket;
        std::function<void()> callback;
    };
    QQueue<QueuedRequest> m_queue;
    bool m_processing = false;
};

#endif // RATE_LIMITER_H
