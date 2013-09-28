#include "csf2playerform.h"
#include "ui_csf2playerform.h"
#include "csf2player.h"

CSF2PlayerForm::CSF2PlayerForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CSF2PlayerForm)
{
    ui->setupUi(this);
    ui->VolDial->setKnobStyle(QSynthKnob::AluminiumStyle);
    ui->VolDial->setNotchStyle(QSynthKnob::LEDNotch);
    connect(ui->LoadButton,SIGNAL(clicked()),this,SLOT(OpenClick()));
    //connect(ui->TestButton,SIGNAL(pressed()),this,SLOT(TestMouseDown()));
    //connect(ui->TestButton,SIGNAL(released()),this,SLOT(TestMouseUp()));
    connect(ui->BankList,SIGNAL(currentIndexChanged(int)),this,SLOT(ChangeBank()));
    connect(ui->PresetList,SIGNAL(currentIndexChanged(int)),this,SLOT(ChangePreset()));
    connect(ui->PatchButton,SIGNAL(toggled(bool)),this,SLOT(PatchToggled(bool)));
    connect(ui->VolDial,SIGNAL(valueChanged(int)),this,SLOT(VolChanged(int)));
}

CSF2PlayerForm::~CSF2PlayerForm()
{
    delete ui;
}

void CSF2PlayerForm::FillPresetList(int Bank,int Preset)
{
    CSF2Player* m_DM=(CSF2Player*)m_Device;
    ui->BankList->clear();
    //ui->PresetList->setSortingEnabled(true);
    //ui->BankList->setInsertPolicy(QComboBox::InsertAlphabetically);
    foreach (int i,m_DM->SF2Device.banks.keys())
    {
        ui->BankList->addItem(QString("000" + QString::number(i)).right(3));
        if (i==Bank)
        {
            ui->BankList->setCurrentIndex(ui->BankList->count()-1);
            m_DM->SF2Device.setBank(i);
            //ui->BankList->setCurrentIndex(i);
        }
    }
    ui->BankList->model()->sort(0);
    /*
    int LastIndex=-1;
    for (short i=0 ; i<m_DM->SF2Device.presetcount(); i++)
    {
        if (m_DM->SF2Device.banknumber(i)>LastIndex)
        {
            LastIndex=m_DM->SF2Device.banknumber(i);
            ui->BankList->addItem(QString::number(m_DM->SF2Device.banknumber(i)));
            if (Bank==LastIndex)
            {
                ui->BankList->setCurrentIndex(ui->BankList->count()-1);
                m_DM->SF2Device.setBank(LastIndex);
                ui->BankList->setCurrentIndex(LastIndex);
            }
        }
    }
    */
    ui->PresetList->clear();
    if (ui->BankList->count()>0)
    {
        FillPresetList2(Preset);
    }
    else
    {
        m_DM->SF2Device.setBank(0);
    }
    ui->PatchButton->setChecked(m_DM->GetParameterValue("Patch Change"));
}

void CSF2PlayerForm::FillPresetList2(int Preset)
{
    CSF2Player* m_DM=(CSF2Player*)m_Device;
    ui->PresetList->clear();
    //ui->PresetList->setSortingEnabled(true);
    //ui->PresetList->setInsertPolicy(QComboBox::InsertAlphabetically);
    QString SelText;
    foreach (int i,m_DM->SF2Device.banks[m_DM->SF2Device.currentBank(0)].presets.keys())
    {
        QString PresetNum="000" + QString::number(i);
        PresetNum=PresetNum.right(3)  + " " + m_DM->SF2Device.banks[m_DM->SF2Device.currentBank(0)].presets[i].name;
        ui->PresetList->addItem(PresetNum);
        if (i==Preset)
        {
            SelText=PresetNum;
            m_DM->SF2Device.setPreset(Preset);
        }
    }
    ui->PresetList->model()->sort(0);
    /*
    //ListBox2->Sorted=true;
    QString SelText;
    for (short i=0 ; i<m_DM->SF2Device.presetcount(); i++)
    {
        if (m_DM->SF2Device.currentBank(0)==m_DM->SF2Device.banknumber(i))
        {
            QString PresetNum="000" + QString::number(m_DM->SF2Device.presetnumber(i));
            PresetNum=PresetNum.right(3);
            ui->PresetList->addItem(PresetNum + " " + m_DM->SF2Device.presetname(i));
            if (m_DM->SF2Device.presetnumber(i)==Preset)
            {
                SelText=PresetNum + " " + m_DM->SF2Device.presetname(i);
                m_DM->SF2Device.setPreset(Preset);
            }
        }
    }
    */
    if (ui->PresetList->count()==0)
    {
        m_DM->SF2Device.setPreset(0);
    }
    else
    {
        for (int i=0;i<ui->PresetList->count();i++)
        {
            if (ui->PresetList->itemText(i)==SelText)
            {
                ui->PresetList->setCurrentIndex(i);
                break;
            }
        }
    }
}

const QString CSF2PlayerForm::CustomSave()
{
    CSF2Player* m_DM=(CSF2Player*)m_Device;

    QDomLiteElement xml("Custom");

    QString Relpath=QDir().relativeFilePath(m_DM->FileName());

    xml.setAttribute("File",Relpath);
    xml.setAttribute("Bank",m_DM->SF2Device.currentBank(0));
    xml.setAttribute("Preset",m_DM->SF2Device.currentPreset(0));
    return xml.toString();
}

void CSF2PlayerForm::CustomLoad(const QString& XML)
{
    CSF2Player* m_DM=(CSF2Player*)m_Device;

    QDomLiteElement xml;
    xml.fromString(XML);
    if (xml.tag=="Custom")
    {
        QString Path = CPresets::ResolveFilename(QFileInfo(xml.attribute("File")).absoluteFilePath());
        if (QFileInfo(m_DM->FileName())==QFileInfo(Path)) return;
        if (QFileInfo(Path).exists())
        {
            ui->LoadButton->setText("...");
            if (m_DM->SF2Device.loadFile(Path))
            {
                int Bank=xml.attributeValue("Bank");
                int Preset=xml.attributeValue("Preset");
                FillPresetList(Bank,Preset);
                m_DM->SetFilename(Path);
                ui->LoadButton->setText(QFileInfo(Path).completeBaseName());
                //this->Caption= OpenDialog1->FileName;
                //CurrentPlayType=(PlayType)StrToInt(xmldoc->GetAttribute(WideString("PlayType")));
                //CurrentBank=StrToInt(xmldoc->GetAttribute(WideString("PlayIndex")));
                //m_DM->SF2Generator->SelectPlay(CurrentPlayType,CurrentPlayIndex);
                return;
            }
            //ShowMessage("Could not open");
        }
    }
}
//---------------------------------------------------------------------------
void CSF2PlayerForm::OpenClick()
{
    CSF2Player* m_DM=(CSF2Player*)m_Device;
    QString fn=m_Device->OpenFile(SF2File::SF2Filter);
    if (QFileInfo(m_DM->FileName())==QFileInfo(fn)) return;
    ui->LoadButton->setText("...");
    if (m_DM->SF2Device.loadFile(fn))
    {
        m_DM->SetFilename(QFileInfo(fn).absoluteFilePath());
        ui->LoadButton->setText(QFileInfo(fn).completeBaseName());
        FillPresetList(0,0);
        //this->Caption= OpenDialog1->FileName;
        //CurrentBank=0;
        return;
    }
    //ShowMessage("Could not open");

}

void CSF2PlayerForm::TestMouseDown()
{
    /*
    CSF2Player* m_DM=(CSF2Player*)m_Device;
    m_DM->SF2Device.NoteOn(0,ui->PitchSlider->value(),ui->VolSlider->value());
    */
}
//---------------------------------------------------------------------------

void CSF2PlayerForm::TestMouseUp()
{
    /*
    CSF2Player* m_DM=(CSF2Player*)m_Device;
    m_DM->SF2Device.NoteOff(0,ui->PitchSlider->value());
    */
}
//---------------------------------------------------------------------------

void CSF2PlayerForm::ChangeBank()
{
    CSF2Player* m_DM=(CSF2Player*)m_Device;
    if (ui->BankList->currentIndex()==-1) return;
    int BankIndex=ui->BankList->currentText().toInt();
    m_DM->SF2Device.setBank(BankIndex);
    FillPresetList2(0);
}
//---------------------------------------------------------------------------

void CSF2PlayerForm::ChangePreset()
{
    CSF2Player* m_DM=(CSF2Player*)m_Device;
    if (ui->PresetList->currentIndex()==-1) return;
    QString Presetnum=ui->PresetList->currentText();
    int PresetIndex=Presetnum.left(3).toInt();
    m_DM->SF2Device.setPreset(PresetIndex);
}

void CSF2PlayerForm::SetPatchResponse(bool value)
{
    ui->PatchButton->blockSignals(true);
    ui->PatchButton->setChecked(value);
    ui->PatchButton->blockSignals(false);
    ui->BankList->setEnabled(!value);
    ui->PresetList->setEnabled(!value);
}

void CSF2PlayerForm::setVolume(int value)
{
    ui->VolDial->blockSignals(true);
    ui->VolDial->setValue(value);
    ui->VolDial->blockSignals(false);
}

void CSF2PlayerForm::PatchToggled(bool value)
{
    m_Device->SetParameterValue("Patch Change",(int)value);
    m_Device->UpdateHost();
    ui->BankList->setEnabled(!value);
    ui->PresetList->setEnabled(!value);
}

void CSF2PlayerForm::VolChanged(int value)
{
    m_Device->SetParameterValue("Volume",value);
    m_Device->UpdateHost();
}

void CSF2PlayerForm::SetProgram(const int Bank, const int Preset)
{
    FillPresetList(Bank,Preset);
}
