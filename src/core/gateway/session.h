#ifndef SESSION_H
#define SESSION_H

#include <QString>

struct Session
{
    QString sessionId;
    int lastSeq = 0;
    QString resumeGatewayUrl;
    bool canResume = false;
};

#endif // SESSION_H
