#ifndef CSF2CHANNELWIDGET_H
#define CSF2CHANNELWIDGET_H

#include <QFrame>
#include <cstereomixer.h>
#include <cdevicecontainer.h>
#include <qsignalmenu.h>
#include "cchannelvol.h"
#include "cchanneleffects.h"

namespace Ui {
class CSF2ChannelWidget;
}

class CSF2ChannelWidget : public QFrame
{
    Q_OBJECT
    
public:
    explicit CSF2ChannelWidget(QWidget *parent = nullptr);
    ~CSF2ChannelWidget();
    void init(CStereoMixerChannel* ch, const QString& Name, CDeviceContainer* SF2=nullptr, short MIDIChannel=-1);
    void load(const QString& filename);
    void checkPreset();
    void checkEffects();
    void checkPeak();
    void checkAll();
    void resetPeak();
    void soloButton(bool pressed);
    void muteButton(bool pressed);
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    void setSender(const QString& s);
    CChannelVol* volSlider;
    CChannelEffects* effectsPanel;
    void toggleEffectRack();
    QString ID;
private:
    void buildPresetMenu();
    Ui::CSF2ChannelWidget *ui;
    CStereoMixerChannel* m_Ch;
    CDeviceContainer* m_Instrument;
    short m_MIDIChannel;
    short preset;
    short bank;
    QSignalMenu* instrumentMenu;
    QAction* showUIAction;
    QList<QSignalMenu*> presetMenus;
    QSignalMenu* bankMenu;
    QAction* togglePatchChangeAction;
    QSignalMenu* transposeMenu;
    QString m_Name;
private slots:
    void setVolume(int Vol);
    void loadDialog();
    void selectProgram(int program);
    void togglePatchChange();
    void setTranspose(int transpose);
    void showBankMenu();
    void selectInstrument(QString instrument);
signals:
    void soloTriggered(bool v);
};

#endif // CCHANNELWIDGET_H
