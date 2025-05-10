#include "ccompressor.h"

CCompressor::CCompressor(){}

void CCompressor::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoIn();
    addJackStereoOut(jnOut);
    addParameterPercent("Threshold",100);
    addParameterPercent("Ratio",100);
    startParameterGroup();
    addParameterTime("Attack Time");
    addParameterTime("Release Time");
    endParameterGroup();
    addParameterVolume("Volume");
    updateDeviceParameter();
}

void inline CCompressor::updateDeviceParameter(const CParameter* /*p*/)
{
    compr.set_threshold(m_Parameters[pnThreshold]->PercentValue);
    compr.set_ratio(m_Parameters[pnRatio]->PercentValue);
    compr.set_attack(m_Parameters[pnAttack]->mSec2samplesValue());
    compr.set_release(m_Parameters[pnRelease]->mSec2samplesValue());
    compr.set_output(m_Parameters[pnOutput]->PercentValue);
}

CAudioBuffer* CCompressor::getNextA(const int ProcIndex)
{
    const CStereoBuffer* InBuffer = FetchAStereo(jnIn);
    qDebug() << InBuffer << InBuffer->isValid();
    if (!InBuffer->isValid()) return nullptr;
    CStereoBuffer* OutBuffer=StereoBuffer(jnOut);
    compr.process(InBuffer->data(),InBuffer->dataR(),OutBuffer->data(),OutBuffer->dataR(),OutBuffer->size());
    return m_AudioBuffers[ProcIndex];
}
