#include "caudiounithost.h"
#include "caudiounitclass.h"
#include "cvstform.h"

#undef devicename
#define devicename "AudioUnitHost"

CAudioUnitHost::CAudioUnitHost()
{
}

CAudioUnitHost::~CAudioUnitHost()
{

}

void CAudioUnitHost::Init(const int Index, void *MainWindow)
{
    m_Name=devicename;
    IDevice::Init(Index,MainWindow);
    AddJackStereoIn();
    AddJackMIDIIn();
    AddJackStereoOut(jnOut);
    AddParameterVolume();
    AddParameterMIDIChannel();
    AddParameter(ParameterType::SelectBox,"Patch Change","",0,1,0,"Off§On",0);
    m_Form=new CVSTForm(new CAudioUnitClass(CPresets::Presets.SampleRate,CPresets::Presets.ModulationRate), this,(QWidget*)MainWindow);
    VolFactor=1.0;
    CalcParams();
}

void inline CAudioUnitHost::CalcParams()
{
    VolFactor=m_ParameterValues[pnVolume]*0.01;
    IAudioPlugInHost* AU=((CVSTForm*)m_Form)->PlugIn;
    AU->setMIDIChannel(m_ParameterValues[pnMIDIChannel]);
}

void CAudioUnitHost::Play(const bool /*fromStart*/)
{
    CAudioUnitClass* AU=(CAudioUnitClass*)((CVSTForm*)m_Form)->PlugIn;
    AU->Play();
}

void CAudioUnitHost::Pause()
{
    CAudioUnitClass* AU=(CAudioUnitClass*)((CVSTForm*)m_Form)->PlugIn;
    AU->AllNotesOff();
    AU->Stop();
}

const QString CAudioUnitHost::FileName()
{
    return ((CVSTForm*)m_Form)->PlugIn->Filename();
}

void CAudioUnitHost::Execute(const bool Show)
{
    if (Show)
    {
        if (((CVSTForm*)m_Form)->PlugIn->Filename().isEmpty())
        {
            ((CVSTForm*)m_Form)->PlugIn->Popup(QCursor::pos());
        }
        else
        {
            m_Form->show();
        }
    }
    else
    {
        m_Form->hide();
    }
}

const QString CAudioUnitHost::Save()
{
    CAudioUnitClass* AU=(CAudioUnitClass*)((CVSTForm*)m_Form)->PlugIn;
    if (AU->Filename().isEmpty()) return QString();
    QDomLiteElement xml("Custom");
    xml.setAttribute("Type",QVariant::fromValue(AU->Type()));
    xml.setAttribute("Subtype",QVariant::fromValue(AU->SubType()));
    xml.setAttribute("Manufacturer",QVariant::fromValue(AU->Manufacturer()));
    xml.setAttribute("Program",QVariant::fromValue(AU->CurrentProgram()));
    QDomLiteDocument d;
    d.fromString(AU->SaveXML());
    xml.appendClone(d.documentElement);
    xml.appendChildFromString(m_Form->Save());
    return xml.toString();
}

void CAudioUnitHost::Load(const QString& XML)
{
    CAudioUnitClass* AU=(CAudioUnitClass*)((CVSTForm*)m_Form)->PlugIn;
    QDomLiteElement xml;
    xml.fromString(XML);
    if (xml.tag=="Custom")
    {
        OSType type=xml.attribute("Type").toUInt();
        OSType subtype=xml.attribute("Subtype").toUInt();
        OSType manufacturer=xml.attribute("Manufacturer").toUInt();
        if ((type != AU->Type()) | (subtype != AU->SubType()) | (manufacturer != AU->Manufacturer()))
        {
            AU->SelectAU(type,subtype,manufacturer);
        }
        int p=xml.attributeValue("Program");
        if (p > -1) AU->SetProgram(p);
        ((CVSTForm*)m_Form)->FillList(p);
        QDomLiteElement* plist=xml.elementByTag("plist");
        if (plist)
        {
            QDomLiteDocument d;
            d.fromString(AU->SaveXML());
            if (!plist->compare(d.documentElement))
            {
                d.documentElement->copy(plist);
                QString s=d.toString();
                AU->LoadXML(s.replace("<data/>","<data></data>"));
            }
        }
        QDomLiteElement* Custom=xml.elementByTag("Custom");
        if (Custom)
        {
            m_Form->Load(Custom->toString());
        }
    }
}

void CAudioUnitHost::Process()
{
    CAudioUnitClass* AU=(CAudioUnitClass*)((CVSTForm*)m_Form)->PlugIn;
    AU->DumpMIDI((CMIDIBuffer*)FetchP(jnMIDIIn),m_ParameterValues[pnPatchChange]);
    float* Buffer=FetchA(jnIn);
    if (Buffer != NULL)
    {
        CopyMemory(AU->inBufferL,Buffer,m_BufferSize*sizeof(float)*2);
    }
    else
    {
        ZeroMemory(AU->inBufferL,m_BufferSize*sizeof(float)*2);
    }
    if (AU->Process())
    {
        ((CStereoBuffer*)AudioBuffers[jnOut])->WriteBuffer(AU->outBufferL,VolFactor);
    }
    else
    {
        AudioBuffers[jnOut]->ZeroBuffer();
    }
}

const QString CAudioUnitHost::PresetName()
{
    IAudioPlugInHost* AU=((CVSTForm*)m_Form)->PlugIn;
    return AU->ProgramName();
}

const QStringList CAudioUnitHost::PresetNames()
{
    IAudioPlugInHost* AU=((CVSTForm*)m_Form)->PlugIn;
    return AU->ProgramNames();
}

void CAudioUnitHost::SetProgram(const int index)
{
    CVSTForm* f=((CVSTForm*)m_Form);
    f->SetProgram(index);
}

const void* CAudioUnitHost::Picture() const
{
    IAudioPlugInHost* AU=((CVSTForm*)m_Form)->PlugIn;
    QPixmap* pm=new QPixmap(AU->Picture());
    return (void*)pm;
}
