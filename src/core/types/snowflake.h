#ifndef SNOWFLAKE_H
#define SNOWFLAKE_H

#include <QString>
#include <QDateTime>
#include <QHash>

class Snowflake
{
public:
    Snowflake() = default;
    explicit Snowflake(const QString &value) : m_value(value) {}

    QString toString() const { return m_value; }

    QDateTime timestamp() const
    {
        qint64 ms = (m_value.toULongLong() >> 22) + 1420070400000;
        return QDateTime::fromMSecsSinceEpoch(ms);
    }

    bool operator<(const Snowflake &other) const { return m_value < other.m_value; }
    bool operator==(const Snowflake &other) const { return m_value == other.m_value; }
    bool operator!=(const Snowflake &other) const { return !(*this == other); }

private:
    QString m_value;
};

inline uint qHash(const Snowflake &key, uint seed = 0)
{
    return qHash(key.toString(), seed);
}

#endif // SNOWFLAKE_H
