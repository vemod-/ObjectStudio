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
    CCaffeine() {
        qDebug() << "caffeine construct";
        enable();
    }
    ~CCaffeine(){
        disable();
    }
    void setReason(const QString& reason)
    {
#ifdef Q_OS_MACOS
        IOPMAssertionSetProperty(assertionID, CFSTR("Name"), reason.toCFString());
#elif defined(Q_OS_IOS)
        m_reason = reason;
#endif
    }
    void enable();
    void disable();
private:
#ifdef Q_OS_MACOS
    IOPMAssertionID assertionID;
#elif defined(Q_OS_IOS)
    QString m_reason;
#endif
};

#endif // CCAFFEINE_H
