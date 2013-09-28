#include "csf2player.h"
#include "csf2playerform.h"

#define devicename "SF2Player"

CSF2Player::CSF2Player()
{
    m_Loading=false;
}

void CSF2Player::Init(const int Index,void* MainWindow)
{
    m_Name=devicename;
    IDevice::Init(Index,MainWindow);
    AddJackMIDIIn();
    AddJackStereoOut(jnOut);
    AddParameterMIDIChannel();
    AddParameterVolume();
    AddParameterTranspose();
    AddParameter(ParameterType::SelectBox,"Patch Change","",0,1,0,"Off§On",0);
    LastTrigger=0;
    LastFreq=0;
    m_Form=new CSF2PlayerForm(this,(QWidget*)MainWindow);
    VolumeFactor=1.0*(1.0/sqrtf(SF2Device::sf2voices));
    SF2Device.reset();
    CalcParams();
}

float* CSF2Player::GetNextA(const int ProcIndex)
{
    if (m_Loading) return NULL;
    if (m_Process)
    {
        m_Process=false;
        Process();
    }
    return AudioBuffers[ProcIndex]->Buffer;
}

void CSF2Player::Process()
{
    SF2Device.parseMIDI((CMIDIBuffer*)FetchP(jnIn));
    bool First=true;
    CStereoBuffer* OutBuffer=(CStereoBuffer*)AudioBuffers[jnOut];
    for (int i1=0;i1<SF2Device.voiceCount();i1++)
    {
        float* BufferL=SF2Device.getNext(i1);
        if (BufferL)
        {
            float volL=VolumeFactor*SF2Device.volL(SF2Device.voiceChannel(i1));
            float volR=VolumeFactor*SF2Device.volR(SF2Device.voiceChannel(i1));
            if (First)
            {
                First=false;
                OutBuffer->WriteBuffer(BufferL,volL,volR);
            }
            else
            {
                OutBuffer->AddBuffer(BufferL,volL,volR);
            }
        }
    }
    if (First) OutBuffer->ZeroBuffer();
}

void CSF2Player::Pause()
{
    SF2Device.allNotesOff();
    //Reset();
}

void CSF2Player::Play(const bool FromStart)
{
    if (FromStart) SF2Device.reset();
}

void inline CSF2Player::CalcParams()
{
    CSF2PlayerForm* f=(CSF2PlayerForm*)m_Form;
    float oldVol=VolumeFactor;
    VolumeFactor=(float)m_ParameterValues[pnVolume]*0.01*(1.0/sqrtf(SF2Device::sf2voices));
    if (oldVol != VolumeFactor) f->setVolume(m_ParameterValues[pnVolume]);
    bool oldPatch=SF2Device.patchResponse;
    SF2Device.patchResponse=m_ParameterValues[pnPatchChange];
    if (oldPatch != SF2Device.patchResponse) f->SetPatchResponse(m_ParameterValues[pnPatchChange]);
    SF2Device.setTranspose(m_ParameterValues[pnTranspose]);
    SF2Device.setChannel(m_ParameterValues[pnMIDIChannel]);
}

void CSF2Player::SetFilename(const QString &FileName)
{
    m_FileName=FileName;
}

void CSF2Player::Load(const QString &XML)
{
    m_Loading=true;
    if (m_Form) m_Form->Load(XML);
    m_Loading=false;
}

void CSF2Player::SetProgram(const int Bank, const int Preset)
{
    CSF2PlayerForm* f=(CSF2PlayerForm*)m_Form;
    f->SetProgram(Bank,Preset);
}
