#ifndef CSTEREOCHANNELWIDGET_H
#define CSTEREOCHANNELWIDGET_H

#include <QFrame>
#include "cstereomixer.h"

namespace Ui {
class CStereoChannelWidget;
}

class CStereoChannelWidget : public QFrame
{
    Q_OBJECT
    
public:
    explicit CStereoChannelWidget(QWidget *parent = 0);
    ~CStereoChannelWidget();
    void init(CStereoMixerChannel* ch, const QString& Name);
    void checkPeak();
    void resetPeak();
    void soloButton(bool pressed);
    void muteButton(bool pressed);
    void serialize(QDomLiteElement* xml);
    void unserialize(QDomLiteElement* xml);
    void setSender(const QString& s);
private:
    Ui::CStereoChannelWidget *ui;
    QString m_Name;
protected:
    CStereoMixerChannel* m_Ch;
private slots:
    void setVolume(int Vol);
signals:
    void soloTriggered(bool v);
};

#endif // CSTEREOCHANNELWIDGET_H
