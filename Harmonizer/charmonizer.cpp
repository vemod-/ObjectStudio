#include "charmonizer.h"

CHarmonizer::CHarmonizer() : PD(presets.SampleRate), PS(presets.SampleRate)
{
    PS.setPolyphony(8);
    PD.setMaxDetectFrequency(3000);
    PD.setPitchRecordsPerSecond(40);
}

void CHarmonizer::updateDeviceParameter(const CParameter* /*p*/)
{
    PS.setOverSampling(1 << m_Parameters[pnOversampling]->Value);
    glider.setGlide(m_Parameters[pnGlide]->Value);
    if (m_Parameters[pnNote]->Value != m_oldValue)
    {
        m_Parameters[pnNote1]->setValue(m_Matrix[m_Parameters[pnNote]->Value].shift[0]);
        m_Parameters[pnNote2]->setValue(m_Matrix[m_Parameters[pnNote]->Value].shift[1]);
        m_Parameters[pnNote3]->setValue(m_Matrix[m_Parameters[pnNote]->Value].shift[2]);
        m_oldValue = m_Parameters[pnNote]->Value;
    }
    m_Matrix[m_Parameters[pnNote]->Value].shift[0]=m_Parameters[pnNote1]->Value;
    m_Matrix[m_Parameters[pnNote]->Value].shift[1]=m_Parameters[pnNote2]->Value;
    m_Matrix[m_Parameters[pnNote]->Value].shift[2]=m_Parameters[pnNote3]->Value;
}

void CHarmonizer::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    startParameterGroup("Harmonize",Qt::blue);
    addParameterSelect("Note","All§C§C#§D§D#§E§F§F#§G§G#§A§A#§B");
    addParameterTranspose("Note 1");
    addParameterTranspose("Note 2");
    addParameterTranspose("Note 3");
    endParameterGroup();
    startParameterGroup();
    addParameterTune();
    addParameterOffOn("AutoTune");
    endParameterGroup();
    addParameterPercent("Glide");
    addParameterSelect("Oversampling","1§2§4§8§16§32",3);
    addParameterPercent("Effect",50);
    updateDeviceParameter();
}

CAudioBuffer* CHarmonizer::getNextA(const int /*ProcIndex*/)
{
    const CMonoBuffer* inBuffer=FetchAMono(jnIn);
    if (!inBuffer->isValid()) return nullptr;
    if (m_Parameters[pnEffect]->Value == 0)
    {
        m_AudioBuffers[jnOut]->writeBuffer(inBuffer);
    }
    else
    {
        PD.ProcessBuffer(inBuffer->data(),presets.ModulationRate);
        const CPitchDetect::PitchRecord r = PD.CurrentPitchRecord();
        int c = r.MidiCents*m_Parameters[pnAutotune]->Value;
        if (r.MidiKey > 0)
        {
            const int mc = (r.MidiKey*100) + (r.MidiCents*m_Parameters[pnAutotune]->Value);
            if (m_Parameters[pnGlide]->Value)
            {
                if (mc != m_lastMIDICent) glider.setTargetCent(c);
                c = glider.currentCent();
            }
            m_lastKey = r.MidiKey;
            m_lastMIDICent = mc;
        }
        if (m_Parameters[pnNote]->Value > 0)
        {
            for (int i=0;i<3;i++)
            {
                const int t = m_Matrix[(m_lastKey % 12)+1].shift[i];
                if (t)
                {
                    vol[i]=1;
                    s[i] = cent2Factor((t*100)+c+tune2Cent(m_Parameters[pnTune]->PercentValue));
                }
                else
                {
                    vol[i]=0;
                }
            }
        }
        else
        {
            for (int i=0;i<3;i++)
            {
                const int t = m_Matrix[0].shift[i];
                if (t)
                {
                    vol[i]=1;
                    s[i] = cent2Factor((t*100)+c+tune2Cent(m_Parameters[pnTune]->PercentValue));
                }
                else
                {
                    vol[i]=0;
                }
            }
        }
        if (m_Parameters[pnEffect]->Value == 100)
        {
            m_AudioBuffers[jnOut]->writeBuffer(PS.process(s,vol,presets.ModulationRate, inBuffer->data()));
        }
        else
        {
            m_AudioBuffers[jnOut]->writeBuffer(PS.process(s,vol,presets.ModulationRate, inBuffer->data()),m_Parameters[pnEffect]->PercentValue);
            m_AudioBuffers[jnOut]->addBuffer(inBuffer,m_Parameters[pnEffect]->DryValue);
        }
    }
    return m_AudioBuffers[jnOut];
}

void CHarmonizer::serializeCustom(QDomLiteElement* xml) const
{
    QDomLiteElement* shifts = xml->appendChild("Shifts");
    for (const shiftMatrix& m : m_Matrix)
    {
        QDomLiteElement* s=shifts->appendChild("Shift");
        s->setAttribute("Note1",m.shift[0]);
        s->setAttribute("Note2",m.shift[1]);
        s->setAttribute("Note3",m.shift[2]);
    }
}

void CHarmonizer::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    QMutexLocker locker(&mutex);
    if (const QDomLiteElement* shifts = xml->elementByTag("Shifts"))
    {
        int i=0;
        for (const QDomLiteElement* d : (const QDomLiteElementList)shifts->elementsByTag("Shift"))
        {
            m_Matrix[i].shift[0]=d->attributeValueInt("Note1");
            m_Matrix[i].shift[1]=d->attributeValueInt("Note2");
            m_Matrix[i].shift[2]=d->attributeValueInt("Note3");
            i++;
        }
    }
    updateDeviceParameter(nullptr);
}

