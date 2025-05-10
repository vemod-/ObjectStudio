#ifndef CCHANNELEQ_H
#define CCHANNELEQ_H

#include <QFrame>
#include "cstereomixer.h"

namespace Ui {
class CChannelEQ;
}

class CChannelEQ : public QFrame
{
    Q_OBJECT

public:
    explicit CChannelEQ(QWidget *parent = nullptr);
    ~CChannelEQ();
    void init(CStereoMixerChannel* ch);
    void unserialize(const QDomLiteElement* xml);
    void serialize(QDomLiteElement* xml) const;
public slots:
    void setHi(int v);
    void setLo(int v);
    void setMid(int v);
    void setEQ(bool v);
private:
    Ui::CChannelEQ *ui;
    CStereoMixerChannel* m_Ch;
};

#endif // CCHANNELEQ_H
