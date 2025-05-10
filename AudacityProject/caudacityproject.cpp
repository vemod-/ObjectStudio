#include "caudacityproject.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>

AudacityBlock::AudacityBlock() : Channel(0), AliasStart(0), Rate(0), Start(0) {}

float* AudacityBlock::GetNext()
{
    return CChannelBuffer(wa.getNextRate(Rate),wa.channels(),presets.ModulationRate).channelPointer(Channel);
}

void AudacityBlock::Reset()
{
    wa.reset();
    wa.skipTo(AliasStart);
}

bool AudacityBlock::Init(const QString& Filename, ulong StartPtr, uint Channels, ulong AliasPointer,int RateOverride)
{
    if (wa.load(Filename))
    {
        Start=StartPtr;
        AliasStart=AliasPointer;
        Rate=RateOverride;
        qDebug() << wa.origRate() << presets.SampleRate;
        if (wa.origRate() != presets.SampleRate)
        {
            const double oldSampleRate=wa.origRate();
            const double newSampleRate=presets.SampleRate;
            const ldouble rateFactor=newSampleRate/oldSampleRate;
            Start*=rateFactor;
            AliasStart*=rateFactor;
        }
        if (Channels<wa.channels()) Channel=Channels;
        Reset();
        return true;
    }
    return false;
}

void AudacityBlock::skip(ulong samples)
{
    wa.skipTo(samples+AliasStart);
}

AudacityClip::AudacityClip() : Counter(0), BlockIndex(0), BufferPointer(0), Offset(0)
{
    auBuffer.makeNull();
}

AudacityClip::~AudacityClip()
{
    qDeleteAll(Blocks);
}

void AudacityClip::Reset()
{
    BlockIndex=0;
    Counter=0;
    BufferPointer=0;
    auBuffer.makeNull();
    for(AudacityBlock* b : qAsConst(Blocks)) b->Reset();
}

void AudacityClip::AddBlock(const QString& Filename,ulong Start,int RateOverride)
{
    const QString FN=CPresets::resolveFilename(Filename);
    if (!QFileInfo::exists(FN)) return;
    auto AB=new AudacityBlock();
    if (AB->Init(FN,Start,0,0,RateOverride)) Blocks.append(AB);
    qDebug() << "AddBlock" << Start << AB->Start << RateOverride;
}

void AudacityClip::AddAliasBlock(const QString& Filename,ulong Start,uint Channel,ulong AliasStart)
{
    const QString FN=CPresets::resolveFilename(Filename);
    if (!QFileInfo::exists(FN)) return;
    auto AB=new AudacityBlock();
    if (AB->Init(FN,Start,Channel,AliasStart,0)) Blocks.append(AB);
    qDebug() << "AddAliasBlock" << Start << AB->Start << AliasStart;
}

void AudacityClip::loadClip(const QDomLiteElement* xml, const QString& ProjectPath,int RateOverride)
{
    if (!xml) return;
    if (xml->matches("waveclip"))
    {
        Offset=xml->attributeValueLDouble("offset");
        if (const QDomLiteElement* sequenze = xml->elementByTag("sequence"))
        {
            for (const QDomLiteElement* block : (const QDomLiteElementList)sequenze->elementsByTag("waveblock"))
            {
                const ulong Start=block->attributeValueULong("start");
                if (const QDomLiteElement* file = block->elementByTag("simpleblockfile"))
                {
                    const QString Filename=file->attribute("filename");
                    QString FilePath=ProjectPath + "/" + Filename;
                    if (QFileInfo::exists(FilePath))
                    {
                        AddBlock(FilePath,Start,RateOverride);
                    }
                    else
                    {
                        const QString Path1=Filename.mid(0,3);
                        const QString Path2="d" + Filename.mid(3,2);
                        FilePath=ProjectPath + "/" + Path1 + "/" + Path2 + "/" + Filename;
                        AddBlock(FilePath,Start,RateOverride);
                    }
                }
                if (const QDomLiteElement* file = block->elementByTag("pcmaliasblockfile"))
                {
                    const QString Filename=file->attribute("aliasfile");
                    const uint AliasChannel=file->attributeValueUInt("aliaschannel");
                    const ulong AliasStart=file->attributeValueULong("aliasstart");
                    AddAliasBlock(Filename,Start,AliasChannel,AliasStart);
                }
            }
        }
    }
}

void AudacityClip::loadSequence(const QDomLiteElement* xml, const QString& ProjectPath,int RateOverride)
{
    if (!xml) return;
    if (xml->matches("sequence"))
    {
        Offset=0;

        for (const QDomLiteElement* block : (const QDomLiteElementList)xml->elementsByTag("waveblock"))
        {
            const ulong Start=block->attributeValueULong("start");
            if (const QDomLiteElement* file = block->elementByTag("simpleblockfile"))
            {
                const QString Filename=file->attribute("filename");
                QString FilePath=ProjectPath + "/" + Filename;
                if (QFileInfo::exists(FilePath))
                {
                    AddBlock(FilePath,Start,RateOverride);
                }
                else
                {
                    //AnsiString Path1=Filename.SubString(1,3);
                    //AnsiString Path2="d" + Filename.SubString(4,2);
                    const QString Path1=Filename.mid(1,3);
                    const QString Path2="d" + Filename.mid(4,2);
                    FilePath=ProjectPath + "/" + Path1 + "/" + Path2 + "/" + Filename;
                    AddBlock(FilePath,Start,RateOverride);
                }
            }
            if (const QDomLiteElement* file = block->elementByTag("pcmaliasblockfile"))
            {
                const QString Filename=file->attribute("aliasfile");
                const uint AliasChannel=file->attributeValueUInt("aliaschannel");
                const ulong AliasStart=file->attributeValueULong("aliasstart");
                AddAliasBlock(Filename,Start,AliasChannel,AliasStart);
            }
        }
    }
}

float* AudacityClip::GetNext()
{
    if (BlockIndex > Blocks.size()-1) return nullptr;
    AudacityBlock* AB = Blocks[BlockIndex];
    AudacityBlock* NB = (BlockIndex < Blocks.size()-1) ? Blocks[BlockIndex+1] : nullptr;
    for (uint i=0; i < Buffer.size(); i++)
    {
        if (Counter==AB->Start)
        {
            auBuffer.fromRawData(AB->GetNext());
            BufferPointer=0;
        }
        if (NB)
        {
            if (Counter==NB->Start)
            {
                AB=NB;
                auBuffer.fromRawData(AB->GetNext());
                BufferPointer=0;
                BlockIndex++;
                NB=nullptr;
                if (BlockIndex < Blocks.size()-1) NB=Blocks[BlockIndex+1];
            }
        }
        if (Counter > AB->Start)
        {
            if (BufferPointer >= Buffer.size())
            {
                auBuffer.fromRawData(AB->GetNext());
                BufferPointer=0;
                if (!NB)
                {
                    if (!auBuffer.isValid()) BlockIndex++;
                }
            }
        }
        (auBuffer.isValid()) ? Buffer.setAt(i,auBuffer.at(BufferPointer)) : Buffer.zeroAt(i);
        BufferPointer++;
        Counter++;
    }
    return Buffer.data();
}

ulong AudacityClip::milliSeconds()
{
    ulong mSecs=0;
    for(AudacityBlock* b : qAsConst(Blocks))
    {
        mSecs=qMax<ulong>(presets.samplesTomSecs(b->end())+ulong(Offset*1000),mSecs);
    }
    return mSecs;
}

void AudacityClip::skip(const ulong64 samples)
{
    Reset();
    Counter=samples;//ulong(mSecs*ldouble(presets.SamplesPermSec));
    for (int i=0;i<Blocks.size();i++)
    {
        AudacityBlock* b=Blocks.at(i);
        if (Counter >= b->Start)
        {
            if (Counter<(b->end()))
            {
                b->skip(Counter-b->Start);
                BlockIndex=i;
                return;
            }
        }
    }
}

AudacityTrack::AudacityTrack()
{
    Time=0;
    ClipIndex=0;
    Playing=false;
    Mute=false;
    Solo=false;
    Gain=0;
    Channel=0;
    Linked=false;
    Offset=0;
    Rate=0;
    Pan=0;
    FactorL=0;
    FactorR=0;
}

AudacityTrack::~AudacityTrack()
{
    qDeleteAll(Clips);
}

void AudacityTrack::Reset()
{
    Playing=false;
    Time=0;
    ClipIndex=0;
    for(AudacityClip* c : qAsConst(Clips)) c->Reset();
}

void AudacityTrack::loadTrack(const QDomLiteElement* xml, const QString& ProjectPath)
{
    if (!xml) return;
    if (xml->matches("wavetrack"))
    {
        Channel = xml->attributeValueInt("channel");
        Name = xml->attribute("name");
        Linked = xml->attributeValueBool("linked");
        Offset=xml->attributeValueLDouble("offset");
        //Mute = StrToInt("0"+AnsiString(xmldoc->GetAttribute("mute")));
        //Solo = StrToInt("0"+AnsiString(xmldoc->GetAttribute("solo")));
        Rate = xml->attributeValueInt("rate");
        Gain = float(xml->attributeValue("gain"));
        Pan = float(xml->attributeValue("pan"));

        FactorL = (Pan > 0) ? Gain * (1.f - Pan) : Gain;
        FactorR = (Pan < 0) ? Gain * (1.f + Pan) : Gain;

        for (const QDomLiteElement* clip : (const QDomLiteElementList)xml->elementsByTag("waveclip"))
        {
            auto AC=new AudacityClip();
            AC->loadClip(clip,ProjectPath,Rate);
            Clips.append(AC);
        }
        for(const QDomLiteElement* clip : (const QDomLiteElementList)xml->elementsByTag("sequence"))
        {
            auto AC=new AudacityClip();
            AC->loadSequence(clip,ProjectPath,Rate);
            Clips.append(AC);
        }
    }
}

float* AudacityTrack::GetNext()
{
    if (ClipIndex > Clips.size()-1) return nullptr;
    AudacityClip* AC=Clips[ClipIndex];
    if (Time < AC->Offset + Offset)
    {
        Time+=ldouble(presets.ModulationTime*0.001);
        return nullptr;
    }
    if (ClipIndex < Clips.size()-1)
    {
        AudacityClip* NC=Clips[ClipIndex+1];
        if (Time >= NC->Offset + Offset)
        {
            ClipIndex++;
            AC=NC;
        }
    }
    Time+=ldouble(presets.ModulationTime*0.001);
    return AC->GetNext();
}

ulong AudacityTrack::milliSeconds()
{
    ulong mSecs=0;
    for(AudacityClip* c : qAsConst(Clips)) mSecs=qMax<ulong>(c->milliSeconds()+ulong(Offset*1000),mSecs);
    return mSecs;
}

void AudacityTrack::skip(const ulong64 samples)
{
    Reset();
    ClipIndex=0;
    Time = presets.samplesTomSecs(samples) * 0.001l;
    for (int i=0;i<Clips.size();i++)
    {
        AudacityClip* c=Clips.at(i);
        if (Time >= c->Offset + Offset)
        {
            if (Time < c->milliSeconds()*0.001l)
            {
                c->skip(samples - presets.mSecsToSamples((c->Offset + Offset)*1000));
                ClipIndex=i;
                return;
            }
        }
    }
}

void inline CAudacityProject::updateDeviceParameter(const CParameter* /*p*/)
{
    ModFactor = m_Parameters[pnVolume]->PercentValue;
}

bool CAudacityProject::loadFile(const QString& ProjectFile)
{
    Loading=true;
    qDeleteAll(Tracks);
    Tracks.clear();
    const QDomLiteDocument doc(ProjectFile);
    const QString projname = doc.documentElement->attribute("projname");
    const QString OrigPath=QFileInfo(ProjectFile).absolutePath() + "/" + projname;
    const QDomLiteElementList tracks = (const QDomLiteElementList)doc.documentElement->elementsByTag("wavetrack");
    qDebug() << projname << OrigPath << tracks.size();
    Time=0;
    m_MilliSeconds=0;
    for (const QDomLiteElement* track : tracks)
    {
        auto AT=new AudacityTrack();
        AT->loadTrack(track,OrigPath);
        m_MilliSeconds=qMax<ulong>(AT->milliSeconds(),m_MilliSeconds);
        Tracks.append(AT);
    }
    Loading=false;
    return true;
}

CAudacityProject::CAudacityProject()
{
    ModFactor=0;
    Time=0;
    m_MilliSeconds=0;
    //Playing=false;
    Loading=false;
}

CAudacityProject::~CAudacityProject()
{
    qDeleteAll(Tracks);
}

void CAudacityProject::execute(const bool Show)
{
    if (Show)
    {
        if (!openFile(selectFile("Audacity projects (*.aup)"))) QDesktopServices::openUrl(QUrl("file://"+filename()));
    }
}

void CAudacityProject::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoOut(jnOut);
    addParameterVolume();
    addFileParameter();
    updateDeviceParameter();
    Time=0;
    m_MilliSeconds=0;
    //Playing=false;
    Loading=false;
}

void CAudacityProject::play(const bool FromStart)
{
    if (FromStart)
    {
        if (m_Initialized)
        {
            Time=0;
            for(AudacityTrack* t : qAsConst(Tracks)) t->Reset();
        }
    }
    //Playing=true;
    IDevice::play(FromStart);
}
/*
void CAudacityProject::pause()
{
    //Playing=false;
    IDevice::pause();
}
*/
ulong CAudacityProject::milliSeconds() const
{
    return qMax<ulong>(m_MilliSeconds, IDevice::milliSeconds());
}

ulong64 CAudacityProject::samples() const
{
    return qMax<ulong64>(presets.mSecsToSamples(m_MilliSeconds), IDevice::samples());
}

void CAudacityProject::skip(const ulong64 samples)
{
    for(AudacityTrack* t : qAsConst(Tracks)) t->skip(samples);
    IDevice::skip(samples);
}

void CAudacityProject::process()
{
    CStereoBuffer* OutBuffer=StereoBuffer(jnOut);
    OutBuffer->zeroBuffer();
    for(AudacityTrack* AT : qAsConst(Tracks))
    {
        if (!AT->Playing)
        {
            if (AT->Offset <= Time)  AT->Playing=true;
        }
        if (AT->Playing)
        {
            CMonoBuffer auBuffer(AT->GetNext());
            if (auBuffer.isValid())
            {
                if (AT->Linked)
                {
                    OutBuffer->addLeftBuffer(auBuffer.data(),AT->FactorL*ModFactor);
                }
                else if (AT->Channel==1)
                {
                    OutBuffer->addRightBuffer(auBuffer.data(),AT->FactorR*ModFactor);
                }
                else
                {
                    OutBuffer->addLeftBuffer(auBuffer.data(),AT->FactorL*ModFactor);
                    OutBuffer->addRightBuffer(auBuffer.data(),AT->FactorR*ModFactor);
                }
            }
        }
    }
    Time+=ldouble(presets.ModulationTime*0.001);
}

CAudioBuffer* CAudacityProject::getNextA(const int ProcIndex)
{
    if (!m_Playing) return nullptr;//&m_NullBufferStereo;
    if (Loading) return nullptr;//&m_NullBufferStereo;
    return IDevice::getNextA(ProcIndex);
    /*
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    return m_AudioBuffers[ProcIndex];
*/
}
