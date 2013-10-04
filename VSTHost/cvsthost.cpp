#include "cvsthost.h"
#include "cvsthostclass.h"
#include <QFileDialog>
#include "cvstform.h"

#undef devicename
#define devicename "VSTHost"
#define BufferCount 8

CVSTHost::CVSTHost()
{
}

CVSTHost::~CVSTHost()
{
    qDebug() << "Exit CVSTHost";
}

void CVSTHost::Init(const int Index, void *MainWindow)
{
    m_Name=devicename;
    IDevice::Init(Index,MainWindow);
    AddJackStereoIn();
    AddJackMIDIIn();
    for (int i=0;i<BufferCount/2;i++) AddJackStereoOut(jnOut+i,"Out " + QString::number(i));
    AddParameterVolume();
    AddParameterMIDIChannel();
    AddParameter(ParameterType::SelectBox,"Patch Change","",0,1,0,"Off§On",0);
    VolFactor=1.0;
    OldBuffers=0;
    m_Form=new CVSTForm(new CVSTHostClass(CPresets::Presets.SampleRate,CPresets::Presets.ModulationRate), this,(QWidget*)MainWindow);

    CalcParams();
}

void inline CVSTHost::CalcParams()
{
    VolFactor=m_ParameterValues[pnVolume]*0.01;
    IAudioPlugInHost* VST=((CVSTForm*)m_Form)->PlugIn;
    VST->setMIDIChannel(m_ParameterValues[pnMIDIChannel]);
}

void CVSTHost::Pause()
{
    IAudioPlugInHost* VST=((CVSTForm*)m_Form)->PlugIn;
    VST->AllNotesOff();
}

const QString CVSTHost::FileName()
{
    IAudioPlugInHost* VST=((CVSTForm*)m_Form)->PlugIn;
    return VST->Filename();
}

void CVSTHost::Execute(const bool Show)
{
    if (Show)
    {
        IAudioPlugInHost* VST=((CVSTForm*)m_Form)->PlugIn;
        if (VST->Filename().isEmpty())
        {
            VST->Popup(QCursor::pos());
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

void CVSTHost::UpdateHost()
{
    m_Host->ParameterChange();
}

const QString CVSTHost::Save()
{
    IAudioPlugInHost* VST=((CVSTForm*)m_Form)->PlugIn;
    if (VST->Filename().isEmpty()) return QString();
    QDomLiteElement xml("Custom");
    QString Relpath=QDir(CPresets::Presets.VSTPath).relativeFilePath(VST->Filename());
    xml.setAttribute("File",Relpath);
    xml.appendChildFromString(VST->SaveXML());
    xml.appendChildFromString(m_Form->Save());
    return xml.toString();
}

void CVSTHost::Load(const QString& XML)
{
    QDomLiteElement xml;
    xml.fromString(XML);
    if (xml.tag=="Custom")
    {
        QString CurrentPath = xml.attribute("File");
        if (!CurrentPath.isEmpty())
        {
            CurrentPath = CPresets::ResolveFilename(QDir(CPresets::Presets.VSTPath).absoluteFilePath(CurrentPath));
            if (QFileInfo(CurrentPath).exists())
            {
                CVSTHostClass* VST=(CVSTHostClass*)((CVSTForm*)m_Form)->PlugIn;
                if (VST->Load(CurrentPath))
                {
                    QDomLiteElement* Custom=xml.elementByTag("Settings");
                    if (Custom) VST->LoadXML(Custom->toString());
                    Custom=xml.elementByTag("Custom");
                    if (Custom) m_Form->Load(Custom->toString());
                    ((CVSTForm*)m_Form)->FillList(VST->CurrentProgram());
                    return;
                }
                qDebug() << "Could not open " + CurrentPath;
            }
        }
    }
}

void CVSTHost::Process()
{
    CVSTHostClass* VST=(CVSTHostClass*)((CVSTForm*)m_Form)->PlugIn;
    VST->DumpMIDI((CMIDIBuffer*)FetchP(jnMIDIIn),m_ParameterValues[pnPatchChange]);
    float* Buffer=FetchA(jnIn);
    if (VST->NumInputs()>1)
    {
        if (Buffer)
        {
            CopyMemory(VST->InBuffers[0],Buffer,m_BufferSize*sizeof(float));
            CopyMemory(VST->InBuffers[1],Buffer+m_BufferSize,m_BufferSize*sizeof(float));
        }
        else
        {
            ZeroMemory(VST->InBuffers[0],m_BufferSize*sizeof(float));
            ZeroMemory(VST->InBuffers[1],m_BufferSize*sizeof(float));
        }
    }
    else if (VST->NumInputs())
    {
        if (Buffer)
        {
            CopyMemory(VST->InBuffers[0],Buffer,m_BufferSize*sizeof(float));
        }
        else
        {
            ZeroMemory(VST->InBuffers[0],m_BufferSize*sizeof(float));
        }
    }
    for (int i=VST->NumOutputs()/2;i<OldBuffers/2;i++)
    {
        AudioBuffers[jnOut + i]->ZeroBuffer();
    }
    OldBuffers=VST->NumOutputs();
    if (VST->Process())
    {
        for (int i=0;i<VST->NumOutputs()/2;i++)
        {
            ((CStereoBuffer*)AudioBuffers[jnOut+i])->FromMono(VST->OutBuffers[i*2],VST->OutBuffers[(i*2)+1],VolFactor);
        }
    }
    else
    {
        for (int i=0;i<VST->NumOutputs()/2;i++)
        {
            ((CStereoBuffer*)AudioBuffers[jnOut+i])->ZeroBuffer();
        }
    }
}

const QString CVSTHost::PresetName()
{
    IAudioPlugInHost* VST=((CVSTForm*)m_Form)->PlugIn;
    return VST->ProgramName();
}

const QStringList CVSTHost::PresetNames()
{
    IAudioPlugInHost* VST=((CVSTForm*)m_Form)->PlugIn;
    return VST->ProgramNames();
}

void CVSTHost::SetProgram(const int index)
{
    ((CVSTForm*)m_Form)->SetProgram(index);
}

const void* CVSTHost::Picture() const
{
    IAudioPlugInHost* VST=((CVSTForm*)m_Form)->PlugIn;
    QPixmap* pm=new QPixmap(VST->Picture());
    return (void*)pm;
}
