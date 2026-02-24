#include "cobjectcomposer.h"
#include "cobjectcomposerform.h"

CObjectComposer::CObjectComposer()
{

}

CObjectComposer::~CObjectComposer()
{
    delete m_Form;
    m_Form = nullptr;
}

void CObjectComposer::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addParameterVolume("Volume");
    addParameterPercent("Humanize");

    m_Form = new CObjectComposerForm(this,MainWindow);
    updateDeviceParameter();
}

void CObjectComposer::initWithFile(const QString& path) {
    FORMFUNC(CObjectComposerForm)->initWithFile(path);
}

CAudioBuffer* CObjectComposer::getNextA(const int ProcIndex)
{
    m_AudioBuffers[ProcIndex]->writeBuffer(FORMFUNC(CObjectComposerForm)->insideIn->getNextA(),m_Parameters[pnVolume]->PercentValue);
    return m_AudioBuffers[ProcIndex];
}

void inline CObjectComposer::updateDeviceParameter(const CParameter* /*p*/)
{
    FORMFUNC(CObjectComposerForm)->UpdateVol(m_Parameters[pnVolume]->Value);
    FORMFUNC(CObjectComposerForm)->setMIDI2wavParameter("Humanize",m_Parameters[pnHumanize]->Value);
}
