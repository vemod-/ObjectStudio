#ifndef CCHANNELGAIN_H
#define CCHANNELGAIN_H

#include <QFrame>
#include "cstereomixer.h"

namespace Ui {
class CChannelGain;
}

class CChannelGain : public QFrame
{
    Q_OBJECT

public:
    explicit CChannelGain(QWidget *parent = nullptr);
    ~CChannelGain();
    void init(CStereoMixerChannel* ch);
    void unserialize(const QDomLiteElement* xml);
    void serialize(QDomLiteElement* xml) const;
public slots:
    void setGain(int v);
    void setLoCut(bool v);
private:
    Ui::CChannelGain *ui;
    CStereoMixerChannel* m_Ch;
};

#endif // CCHANNELGAIN_H
