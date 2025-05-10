#ifndef CMIXERWIDGET_H
#define CMIXERWIDGET_H

#include <QWidget>
#include <cmasterwidget.h>
#include <QGridLayout>
#include <QDomLite>

    #include <csf2channelwidget.h>
    #define CHANNELWIDGET CSF2ChannelWidget

namespace Ui {
class CMixerWidget;
}

class CMixerWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit CMixerWidget(QWidget *parent = nullptr);
    ~CMixerWidget();
    CHANNELWIDGET* appendChannel(int index=-1);
    void removeChannel(int index=-1);
    void showMaster(CStereoMixer *mx, QList<CDeviceContainer*>* effects=nullptr);
    void hideMaster();
    void clear();
    QList<CHANNELWIDGET*> channels;
    CMasterWidget* master;
    void resetPeak();
    void unserialize(const QDomLiteElement* xml);
    void serialize(QDomLiteElement* xml) const;
public slots:
    void start();
    void stop();
private slots:
    void peak();
    void setSoloChannel(int channel);
private:
    Ui::CMixerWidget *ui;
    QTimer peakTimer;
    QGridLayout* lo;
    int timercounter;
};

#endif // CMIXERWIDGET_H
