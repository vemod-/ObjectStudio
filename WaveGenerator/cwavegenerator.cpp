#include "cwavegenerator.h"
#include <QFileInfo>

CWaveGenerator::CWaveGenerator()
{
    WF=nullptr;
    Init();
}

CWaveGenerator::~CWaveGenerator()
{
    Unref();
    qDebug() << "Exit WaveGenerator";
}

void CWaveGenerator::Unref()
{
    if (WF!=nullptr)
    {
        CSingleMap<QString,CWaveFile>::removeItem(m_Path.toLower());
        m_Path.clear();
        WF=nullptr;
    }
}

bool CWaveGenerator::load(const QString& path, uint SampleRate, uint BufferSize)
{
    QMutexLocker locker(&mutex);
    m_Size=0;
    Unref();
    m_Path=QFileInfo(path).absoluteFilePath();
    m_BufferSize=BufferSize;
    if (!QFileInfo::exists(m_Path)) return false;
    WF=CSingleMap<QString,CWaveFile>::addItem(m_Path.toLower());
    if (WF->refCount==1)
    {
        if (!WF->load(m_Path,SampleRate))
        {
            Unref();
            return false;
        }
    }
    Init();
    m_Audio.initZero(BufferSize,WF->data.channels());
    m_Size=WF->data.size();
    LP.End=m_Size;
    return true;
}

void inline CWaveGenerator::Init()
{
    m_Pointer=0;
    m_Finished=true;
    m_Size=0;
    LP.reset();
    m_Position=0;
    m_OrigFreq=440;
    m_SampleState=ssSilent;
}

void CWaveGenerator::finishBuffer(const uint fromPtr)
{
    m_Finished=true;
    for (uint c=0; c < m_Audio.channels(); c++)
    {
        zeroMemory(m_Audio.dataPointer(fromPtr,c),m_BufferSize-fromPtr);
    }
}

float* CWaveGenerator::getNext()
{
    if (m_Finished) return nullptr;
    const uint len=uint(qMin<ldouble>(m_Size-m_Pointer,m_BufferSize));
    if (len < m_BufferSize) finishBuffer(len);
    if (len==0) return nullptr;
    m_Audio.copy(WF->data,ulong64(m_Pointer),len);
    m_Pointer+=len;
    return m_Audio.data();
}

float* CWaveGenerator::getNextSpeed(const double Speed)
{
    if (m_Finished) return nullptr;
    if (isOne(Speed) || (isZero(Speed))) return getNext();
    for (uint i = 0; i < m_BufferSize; i++)
    {
        if (m_Pointer >= m_Size)
        {
            finishBuffer(i);
            break;
        }
        m_Audio.setX(i,WF->data,ulong64(m_Pointer));
        m_Pointer+=ldouble(Speed);
    }
    return m_Audio.data();
}

float* CWaveGenerator::getNextRate(const double RateOverride)
{
    return (m_Finished) ? nullptr : getNextSpeed(RateOverride/WF->frequency);
}

float* CWaveGenerator::getNextFreq(const double Frequency)
{
    if (m_SampleState==ssSilent) return nullptr;
    double OverrideFactor=Frequency/m_OrigFreq;
    if (isZero(OverrideFactor)) OverrideFactor=1;
    float XFadeVol=0;
    for (uint i = 0; i < m_BufferSize; i++)
    {
        float Vol=1.0;
        switch (LP.LoopType)
        {
        case ltForward:
            if (m_SampleState==ssStarting)
            {
                m_Position=LP.Start;
                m_SampleState = (LP.LoopStart<LP.LoopEnd) ? ssLooping : ssEnding;
            }
            else if (m_SampleState==ssLooping)
            {
                while (m_Position>LP.LoopEnd) m_Position-=LP.LoopEnd-LP.LoopStart;
            }
            else
            {
                if ((m_Position >= LP.End) | (m_Position >= m_Size)) m_SampleState=ssSilent;
            }
            if (m_Position<0)
            {
                m_Audio.zeroAtX(i);//m_Audio[i+(m_BufferSize*c)]=0;
            }
            else if ((m_Position >= LP.End) | (m_Position >= m_Size))
            {
                m_SampleState=ssSilent;
                m_Audio.zeroAtX(i);//m_Audio[i+(m_BufferSize*c)]=0;
            }
            else
            {
                m_Audio.setX(i,WF->data,ulong64(m_Position));
                //for (int c=0; c < m_Audio.channels(); c++) m_Audio.set(i,c,WF->data.get(m_Position,c));//m_Audio[i+(m_BufferSize*c)]=*(m_Buffer+(ulong64)m_Position+(m_Size*c));
            }
            m_Position+=ldouble(OverrideFactor);
            break;
        case ltAlternate:
            if (m_SampleState==ssStarting)
            {
                m_Position=LP.Start;
                m_SampleState = (LP.LoopStart<LP.LoopEnd) ? ssLooping : ssEnding;
            }
            else if (m_SampleState==ssLooping)
            {
                if (AlternateDirection==1) {
                    if (m_Position>=LP.LoopEnd) AlternateDirection=-1;
                }
                else if (AlternateDirection==-1) {
                    if (m_Position<=LP.LoopStart) AlternateDirection=1;
                }
            }
            else
            {
                if (AlternateDirection==-1)
                {
                    if (m_Position<=LP.LoopStart) AlternateDirection=1;
                }
                if ((m_Position >= LP.End) | (m_Position >= m_Size)) m_SampleState=ssSilent;
            }
            if (m_Position<0)
            {
                m_Audio.zeroAtX(i);//m_Audio[i+(m_BufferSize*c)]=0;
            }
            else if ((m_Position >= LP.End) | (m_Position >= m_Size))
            {
                m_SampleState=ssSilent;
                m_Audio.zeroAtX(i);//m_Audio[i+(m_BufferSize*c)]=0;
            }
            else
            {
                m_Audio.setX(i,WF->data,ulong64(m_Position));
                //for (int c=0; c < m_Audio.channels(); c++) m_Audio.set(i,c,WF->data.get(m_Position,c));//m_Audio[i+(m_BufferSize*c)]=*(m_Buffer+(ulong64)m_Position+(m_Size*c));
            }
            m_Position+=ldouble(OverrideFactor*AlternateDirection);
            break;
        case ltXFade:
            if (m_SampleState==ssStarting)
            {
                m_Position=LP.Start;
                XFadePosition=LP.Start;
                m_SampleState = (LP.LoopStart<LP.LoopEnd) ? ssLooping : ssEnding;
            }
            else if (m_SampleState==ssLooping)
            {
                if (!isZero(XFadeFactor))
                {
                    if (!XFadeStarted)
                    {
                        if (m_Position>XFadeEnd) XFadeStarted=true;
                    }
                    if (XFadeStarted)
                    {
                        XFadeVol=0;
                        if (m_Position>XFadeEnd && m_Position<LP.LoopEnd)
                        {
                            const double diff=double(LP.LoopEnd-m_Position);
                            XFadeVol=float(LP.XFade-diff)*XFadeFactor;
                            XFadePosition=LP.LoopStart-diff;
                        }
                        if (m_Position<XFadeStart && m_Position>LP.LoopStart)
                        {
                            const double diff=double(m_Position-LP.LoopStart);
                            XFadeVol=float(LP.XFade-diff)*XFadeFactor;
                            XFadePosition=LP.LoopEnd+diff;
                        }
                        Vol=1.f-XFadeVol;
                    }
                    while (m_Position>LP.LoopEnd) m_Position-=LP.LoopEnd-LP.LoopStart;
                }
                else
                {
                    while (m_Position>LP.LoopEnd) m_Position-=LP.LoopEnd-LP.LoopStart;
                }
            }
            else
            {
                if (!isZero(XFadeFactor))
                {
                    if (XFadeStarted)
                    {
                        XFadeVol=0;
                        if (m_Position>XFadeEnd && m_Position<LP.LoopEnd)
                        {
                            const double diff=double(LP.LoopEnd-m_Position);
                            XFadeVol=float(LP.XFade-diff)*XFadeFactor;
                            XFadePosition=LP.LoopStart-diff;
                        }
                        if (m_Position<XFadeStart && m_Position>LP.LoopStart)
                        {
                            const double diff=double(m_Position-LP.LoopStart);
                            XFadeVol=float(LP.XFade-diff)*XFadeFactor;
                            XFadePosition=LP.LoopEnd+diff;
                        }
                        if (m_Position<(LP.LoopEnd+ulong64(LP.XFade)) && m_Position>LP.LoopEnd)
                        {
                            const double diff=double(m_Position-LP.LoopEnd);
                            XFadeVol=float(LP.XFade-diff)*XFadeFactor;
                            XFadePosition=LP.LoopStart+diff;
                        }
                        Vol=1.f-XFadeVol;
                    }
                }
                if ((m_Position >= LP.End) | (m_Position >= m_Size)) m_SampleState=ssSilent;
            }
            if (m_Position<0)
            {
                m_Audio.zeroAtX(i);//m_Audio[i+(m_BufferSize*c)]=0;
            }
            else if ((m_Position >= LP.End) | (m_Position >= m_Size))
            {
                m_SampleState=ssSilent;
                m_Audio.zeroAtX(i);//m_Audio[i+(m_BufferSize*c)]=0;
            }
            else
            {
                m_Audio.setX(i,WF->data,ulong64(m_Position),Vol);
                //for (int c=0; c < m_Audio.channels(); c++) m_Audio.set(i,c,WF->data.get(m_Position,c)*Vol);//m_Audio[i+(m_BufferSize*c)]=*(m_Buffer+(ulong64)m_Position+(m_Size*c))*Vol;
            }

            if (XFadeFactor > 0)
            {
                if (XFadeVol > 0)
                {
                    if ((XFadePosition >= 0) && (XFadePosition < m_Size))
                    {
                        m_Audio.addX(i,WF->data,ulong64(XFadePosition),XFadeVol);
                        //for (int c=0; c < m_Audio.channels(); c++) m_Audio.set(i,c,WF->data.get(XFadePosition,c)*XFadeVol);//m_Audio[i+(m_BufferSize*c)]+=*(m_Buffer+(ulong64)XFadePosition+(m_Size*c))*XFadeVol;
                    }
                }
            }
            m_Position+=ldouble(OverrideFactor);
        }
    }
    return m_Audio.data();
}

void CWaveGenerator::reset()
{
    m_Pointer=0;
    m_Finished=false;
    m_SampleState=ssStarting;
    AlternateDirection=1;
    XFadeFactor = (LP.XFade) ? XFadeFactor=(1.f/LP.XFade)*0.5f : 0;
    XFadeStart=LP.LoopStart+ulong64(LP.XFade);
    XFadeEnd=LP.LoopEnd-ulong64(LP.XFade);

    m_OrigFreq=MIDIkey2Freq(LP.MIDIKey,440.0,LP.MIDICents);
    XFadeStarted=false;
}

void CWaveGenerator::release()
{
    if (m_SampleState != ssSilent)
    {
        m_SampleState=ssEnding;
    }
}

void CWaveGenerator::skipTo(const ulong64 Ptr)
{
    m_Pointer=Ptr;
}


