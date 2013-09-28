#ifndef CCHANNELWIDGET_H
#define CSF2CHANNELWIDGET_H

#include <QFrame>
#include <cstereomixer.h>
#include <cdevicecontainer.h>
#include <qsynthknob.h>
#include <qsignalmenu.h>

namespace Ui {
class CSF2ChannelWidget;
}

class CSF2ChannelWidget : public QFrame
{
    Q_OBJECT
    
public:
    explicit CSF2ChannelWidget(QWidget *parent = 0);
    ~CSF2ChannelWidget();
    void Init(CStereoMixerChannel* ch, CDeviceContainer* SF2, short MIDIChannel, QString Name);
    void loadSF(QString filename);
    void checkPreset();
    void checkPeak();
    void resetPeak();
    void soloButton(bool pressed);
    const QString Save();
    void Load(const QString& XML);
private:
    void buildPresetMenu();
    Ui::CSF2ChannelWidget *ui;
    CStereoMixerChannel* m_Ch;
    CDeviceContainer* m_Instrument;
    short m_MIDIChannel;
    short preset;
    short bank;
    QList<QSynthKnob*> Effect;
    QSignalMapper* mapper;
    QSignalMenu* instrumentMenu;
    QAction* showUIAction;
    QList<QSignalMenu*> presetMenus;
    QSignalMenu* bankMenu;
    QAction* togglePatchChangeAction;
private slots:
    void setVolume(int Vol);
    void setPan(int Pan);
    void setMute(bool Mute);
    void setBypass(bool Bypass);
    void setEffect(int effNumber);
    void loadDialog();
    void selectInstrument(QString instrument);
    void selectProgram(int program);
    void togglePatchChange();
    void showBankMenu();
signals:
    void solo();
protected:
    void showEvent(QShowEvent *);
};

#endif // CCHANNELWIDGET_H
