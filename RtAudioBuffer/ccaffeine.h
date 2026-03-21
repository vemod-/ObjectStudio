#ifndef CCAFFEINE_H
#define CCAFFEINE_H

#include <QString>
#include <QDebug>

#ifdef Q_OS_MACOS
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class CCaffeine
{
public:
    CCaffeine();
    ~CCaffeine();

    void setReason(const QString& reason);
    void enable();
    void disable();

private:

#ifdef Q_OS_MACOS
    IOPMAssertionID assertionID = kIOPMNullAssertionID;
    QString m_reason = "Caffeine";
#elif defined(Q_OS_IOS)
    QString m_reason;
#endif
};

#endif