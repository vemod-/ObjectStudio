#include "csf2playerform.h"
#include "ui_csf2playerform.h"
#include "csf2player.h"

CSF2PlayerForm::CSF2PlayerForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CSF2PlayerForm)
{
    ui->setupUi(this);
    ui->label->setShadowColor(QColor(255,255,255,200));
    ui->label->setTextColor(QColor(0,0,0,200));
    ui->label->setEffect(EffectLabel::Raised);
    ui->PatchButton->setButtonStyle(QToggleButton::TouchStyle);
    ui->VolDial->setKnobStyle(QSynthKnob::AluminiumStyle);
    ui->VolDial->setNotchStyle(QSynthKnob::LEDNotch);
    connect(ui->LoadButton,&QLCDLabel::leftClicked,this,&CSF2PlayerForm::OpenClick);
    connect(ui->BankList,&QLCDDropDown::currentIndexChanged,this,&CSF2PlayerForm::ChangeBank);
    connect(ui->PresetList,&QLCDDropDown::currentIndexChanged,this,&CSF2PlayerForm::ChangePreset);
    parameters[CSF2Player::pnPatchChange]->connectToWidget(ui->PatchButton,&QToggleButton::toggled,&QToggleButton::setChecked);
    parameters[CSF2Player::pnVolume]->connectToWidget(ui->VolDial,&QSynthKnob::valueChanged,&QSynthKnob::setValue);
    m_TimerID=startTimer(200);
}

CSF2PlayerForm::~CSF2PlayerForm()
{
    killTimer(m_TimerID);
    m_TimerID=0;
    delete ui;
}

void CSF2PlayerForm::FillBankList(int Bank,int Preset)
{
    ui->BankList->blockSignals(true);
    ui->PresetList->blockSignals(true);
    ui->BankList->clear();
    ui->BankList->setStringList(SF2DEVICE->bankCaptions());
    ui->BankList->setCurrentText(SF2DEVICE->bankCaption(Bank));
    ui->PresetList->clear();
    ui->BankList->blockSignals(false);
    ui->PresetList->blockSignals(false);
    FillPresetList(Bank,Preset);
}

void CSF2PlayerForm::FillPresetList(int Bank, int Preset)
{
    ui->BankList->blockSignals(true);
    ui->PresetList->blockSignals(true);
    ui->PresetList->clear();
    ui->PresetList->setStringList(SF2DEVICE->presetCaptions(Bank));
    ui->PresetList->setCurrentText(SF2DEVICE->presetCaption(Bank,Preset));
    ui->BankList->blockSignals(false);
    ui->PresetList->blockSignals(false);
}

void CSF2PlayerForm::serializeCustom(QDomLiteElement* xml) const
{
    xml->setAttribute("Bank",SF2DEVICE->currentBank(-1));
    xml->setAttribute("Preset",SF2DEVICE->currentPreset(-1));
}

void CSF2PlayerForm::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    QMutexLocker locker(&mutex);
    if (!m_Device->filename().isEmpty())
    {
        const int Bank=xml->attributeValueInt("Bank");
        const int Preset=xml->attributeValueInt("Preset");
        FillBankList(Bank,Preset);
        SF2DEVICE->setBankPreset(Bank,Preset);
        SetPatchResponse();
        ui->LoadButton->setText(QFileInfo(m_Device->filename()).completeBaseName());
    }
}
//---------------------------------------------------------------------------
void CSF2PlayerForm::OpenClick()
{
    QMutexLocker locker(&mutex);
    if (m_Device->openFile(m_Device->selectFile(SF2File::SF2Filter)))
    {
        ui->LoadButton->setText(QFileInfo(m_Device->filename()).completeBaseName());
        FillBankList(0,0);
        SF2DEVICE->setBankPreset(0,0);
        SetPatchResponse();
    }
}

void CSF2PlayerForm::TestMouseDown()
{
    /*
    SF2DEVICE->NoteOn(0,ui->PitchSlider->value(),ui->VolSlider->value());
    */
}
//---------------------------------------------------------------------------

void CSF2PlayerForm::TestMouseUp()
{
    /*
    SF2DEVICE->NoteOff(0,ui->PitchSlider->value());
    */
}
//---------------------------------------------------------------------------

void CSF2PlayerForm::ChangeBank(int /*index*/)
{
    if (ui->BankList->currentIndex() < 0) return;
    const int bank=ui->BankList->currentText().toInt();
    FillPresetList(bank,0);
    SF2DEVICE->setBankPreset(bank,0);
}
//---------------------------------------------------------------------------

void CSF2PlayerForm::ChangePreset(int /*index*/)
{
    if (ui->PresetList->currentIndex() < 0) return;
    const int bank=ui->BankList->currentText().toInt();
    const int preset=ui->PresetList->currentText().left(3).toInt();
    SF2DEVICE->setBankPreset(bank,preset);
}

void CSF2PlayerForm::SetPatchResponse()
{
    ui->BankList->setEnabled(!ui->PatchButton->isChecked());
    ui->PresetList->setEnabled(!ui->PatchButton->isChecked());
}

void CSF2PlayerForm::SetProgram(const int Program)
{
    CSF2Device* d=SF2DEVICE;
    FillBankList(d->banknumber(Program),d->presetnumber(Program));
    d->setBankPreset(Program);
}

void CSF2PlayerForm::timerEvent(QTimerEvent *)
{
    if (!m_TimerID) return;
    const int p=SF2DEVICE->currentPreset(-1);
    const int b=SF2DEVICE->currentBank(-1);
    const int bank=ui->BankList->currentText().toInt();
    int preset=ui->PresetList->currentText().left(3).toInt();
    if (bank != b)
    {
        preset=p;
        FillPresetList(b,p);
        ui->BankList->blockSignals(true);
        ui->BankList->setCurrentText(SF2DEVICE->bankCaption(b));
        ui->BankList->blockSignals(false);
        ui->PresetList->blockSignals(true);
        ui->PresetList->setCurrentText(SF2DEVICE->presetCaption(b,p));
        ui->PresetList->blockSignals(false);
        m_Device->updateHostParameter();
    }
    if (preset != p)
    {
        ui->PresetList->blockSignals(true);
        ui->PresetList->setCurrentText(SF2DEVICE->presetCaption(b,p));
        ui->PresetList->blockSignals(false);
        m_Device->updateHostParameter();
    }
}
