#include "csf2generator.h"

bool CSF2Generator::load(const QString& Path)
{
    QMutexLocker locker(&mutex);
    endNote();
    MidiBank=0;
    MidiPreset=0;
    m_PitchWheel=0;
    m_PortamentoStep=0;
    finished=true;
    ID=0;
    channel=0;
    QString m_Path = QFileInfo(Path).absoluteFilePath();
    Ready=false;
    Unref();
    if (!QFileInfo::exists(m_Path)) return false;
    SFFile=CSingleMap<QString, CSF2File>::addItem(m_Path.toLower());
    if (SFFile->refCount==1)
    {
        if (!SFFile->load(m_Path))
        {
            Unref();
            return false;
        }
    }
    return true;
}

CSF2Generator::CSF2Generator() :
    ISoundGenerator()
{
    Ready=false;
    SFFile=nullptr;
    MidiBank=0;
    MidiPreset=0;
}

CSF2Generator::~CSF2Generator()
{
    Unref();
    //qDeleteAll(Osc);
}

void CSF2Generator::Unref()
{
    QMutexLocker locker(&mutex);
    if (SFFile!=nullptr)
    {
        CSingleMap<QString, CSF2File>::removeItem(SFFile->path.toLower());
        SFFile=nullptr;
    }
}

void CSF2Generator::setPitchWheel(const int cent)
{
    ISoundGenerator::setPitchWheel(cent);
    for (CSFOscillator& O : Osc) O.setPitchWheel(cent);
}

void CSF2Generator::addPortamento(const int steps)
{
    ISoundGenerator::addPortamento(steps);
    for (CSFOscillator& O : Osc) O.setTranspose(m_PortamentoStep);
}

void CSF2Generator::setAftertouch(const int value)
{
    ISoundGenerator::setAftertouch(value);
    const float val=(value*0.001f)+1;
    for (CSFOscillator& O : Osc) O.setAftertouch(val);
}

void CSF2Generator::resetPortamento()
{
    ISoundGenerator::resetPortamento();
    for (CSFOscillator& O : Osc) O.setTranspose(0);
}

void CSF2Generator::startNote(const short MidiNote, const short MidiVelo)
{
    Ready=false;
    if (!SFFile) return;
    PlayEnd=false;
    finished=false;
    std::vector<sfData> OscData = SFFile->SF2Enabler.Nav(ushort(bankPresetNumber(MidiBank,MidiPreset)),ushort(MidiNote),ushort(MidiVelo));
    if (OscData.empty()) return;
    /*
    Debug("Osc count " + QString::number(OscCount) +
        " Offset " + QString::number(SFFile->Offset) +
        " Size " + QString::number(SFFile->SFData->Size) +
        " MIDI Bank " + QString::number(MidiBank) +
        " MIDI Preset " + QString::number(MidiPreset) +
        " SF Bank " + QString::number(SFFile->BankID)
    );
    */
    //OscFactor=mixFactor(OscCount);
    Osc.resize(OscData.size());//for (ulong i=Osc.size();i<OscCount;i++) Osc.push_back(new CSFOscillator);
    for (uint i=0;i<OscData.size();i++)
    {
        CSFOscillator& O=Osc[i];
        O.Silent=true;
        if (SFFile->sampleCount > 0)
        {
            O.setTune(m_Tune);
            O.Init(&OscData[i],MidiNote,MidiVelo);
            O.setPitchWheel(m_PitchWheel);
            O.setTranspose(m_PortamentoStep);
            O.Start();
        }
    }
    Ready=true;
}

void CSF2Generator::endNote()
{
    PlayEnd=true;
    ID=0;
    for (CSFOscillator& O : Osc) O.Finish();
}

float* CSF2Generator::getNext()
{
    if ((!Ready) | finished | (SFFile==nullptr)) return nullptr;
    ushort SilentCount=0;
    for (CSFOscillator& O : Osc)
    {
        if (!O.Silent) break;
        if (++SilentCount==Osc.size())
        {
            finished=true;
            return nullptr;
        }
    }
    Buffer.zeroBuffer();
    for (CSFOscillator& O : Osc)
    {
        if (!O.Silent)
        {
            O.Modulate();
            //const char SmpMd = O.SampleMode & 3;
            const bool DoLoop = ((O.SampleMode & 0x01) | (!PlayEnd & (O.SampleMode & 0x02)));
            if (O.Stereo==CSFOscillator::StereoL)
            {
                if (DoLoop)
                {
                    for (uint s=0;s<Buffer.size();s++)
                    {
                        O.Loop();
                        Buffer.addAtL(s,O.UpdatePos(SFFile));
                    }
                }
                else //if (SmpMd==0)
                {
                    for (uint s=0;s<Buffer.size();s++)
                    {
                        if (O.NoLoop()) break;
                        Buffer.addAtL(s,O.UpdatePos(SFFile));
                    }
                }
            }
            else if (O.Stereo==CSFOscillator::StereoR)
            {
                if (DoLoop)
                {
                    for (uint s=0;s<Buffer.size();s++)
                    {
                        O.Loop();
                        Buffer.addAtR(s,O.UpdatePos(SFFile));
                    }
                }
                else //if (SmpMd==0)
                {
                    for (uint s=0;s<Buffer.size();s++)
                    {
                        if (O.NoLoop()) break;
                        Buffer.addAtR(s,O.UpdatePos(SFFile));
                    }
                }
            }
            else
            {
                const float LeftPan=float(O.LeftPan);
                const float RightPan=float(O.RightPan);
                if (DoLoop)
                {
                    for (uint s=0;s<Buffer.size();s++)
                    {
                        O.Loop();
                        const float a=O.UpdatePos(SFFile);
                        Buffer.addAt(s,a*LeftPan,a*RightPan);
                    }
                }
                else //if (SmpMd==0)
                {
                    for (uint s=0;s<Buffer.size();s++)
                    {
                        if (O.NoLoop()) break;
                        const float a=O.UpdatePos(SFFile);
                        Buffer.addAt(s,a*LeftPan,a*RightPan);
                    }
                }
            }
        }
    }
    return Buffer.data();
}

int CSF2Generator::banknumber(const int program) const
{
    return (!SFFile) ? 0 : SFFile->programHeaders[uint(program)].wPresetBank;
}

int CSF2Generator::presetnumber(const int program) const
{
    return  (!SFFile) ? 0 : SFFile->programHeaders[uint(program)].wPresetNum;
}

const QString CSF2Generator::bankPresetName(const int program) const
{
    return  (!SFFile) ? QString() : SFFile->programHeaders[uint(program)].achPresetName;
}

const QString CSF2Generator::presetName(const int bank, const int preset) const
{
    return (!SFFile) ? QString() : SFFile->banks[bank].presets[preset].presetName;
}

int CSF2Generator::bankPresetNumber(const int bank, const int preset) const
{
    return  (!SFFile) ? 0 : SFFile->banks[bank].presets[preset].programNumber;
}

const QList<int> CSF2Generator::bankNumbers() const
{
    return  (!SFFile) ? QList<int>() : SFFile->banks.keys();
}

const QList<int> CSF2Generator::presetNumbers(const int bank) const
{
    return  (!SFFile) ? QList<int>() : SFFile->banks[bank].presets.keys();
}
