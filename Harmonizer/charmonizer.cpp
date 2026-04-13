#include "charmonizer.h"

CHarmonizer::CHarmonizer() : PD(presets.SampleRate), PS(presets.SampleRate,presets.ModulationRate,3)
{
    PD.setMaxDetectFrequency(3000);
    PD.setPitchRecordsPerSecond(10);
}

void CHarmonizer::updateDeviceParameter(const CParameter* /*p*/)
{
    PS.setOverSampling(1 << m_Parameters[pnOversampling]->Value);
    PD.setGlide(m_Parameters[pnGlide]->Value);
    PD.setDetectSlack(m_Parameters[pnSlack]->Value);
    PD.setDetectLevelThreshold(m_Parameters[pnThreshold]->PercentValue);
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
    addParameterTune();
    startParameterGroup();
    addParameterOffOn("AutoTune");
    addParameterPercent("Glide");
    addParameter(CParameter::Numeric,"Slack","Cents",0,100,0,"",2);
    addParameterPercent("Threshold",10);
    endParameterGroup();
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
        const CYIN::PitchRecord r = PD.CurrentPitchRecord();
        int target = (m_Parameters[pnAutotune]->Value) ? PD.correctionCents() : 0;
        if (r.MidiKey > 0) m_lastKey = r.MidiKey;
        if (m_Parameters[pnNote]->Value > 0)
        {
            for (int i=0;i<3;i++)
            {
                const int t = m_Matrix[(m_lastKey % 12)+1].shift[i];
                if (t)
                {
                    vol[i]=m_Parameters[pnEffect]->PercentValue;
                    s[i] = cent2Factor((t * 100) + target + tune2Cent(m_Parameters[pnTune]->PercentValue));
                }
                else
                {
                    vol[i] = 0;
                    s[i] = 0;
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
                    vol[i]=m_Parameters[pnEffect]->PercentValue;
                    s[i] = cent2Factor((t * 100) + target + tune2Cent(m_Parameters[pnTune]->PercentValue));
                }
                else
                {
                    vol[i] = 0;
                    s[i] = 0;
                }
            }
        }
        if (m_Parameters[pnEffect]->Value == 100)
        {
            PS.process(s,vol,inBuffer->data(),m_AudioBuffers[jnOut]->data());
        }
        else
        {
            PS.process(s,vol,inBuffer->data(),m_AudioBuffers[jnOut]->data());
            m_AudioBuffers[jnOut]->addBuffer(inBuffer->data(),m_Parameters[pnEffect]->DryValue);
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

