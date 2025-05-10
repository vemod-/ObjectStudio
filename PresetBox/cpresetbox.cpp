#include "cpresetbox.h"
#include <QVBoxLayout>

CPresetBox::CPresetBox()
{
}

CPresetBox::~CPresetBox()
{
    qDebug() << "~CPresetBox";
    if (m_Initialized)
    {
        qDeleteAll(JacksCreated);
    }
}

void CPresetBox::init(const int Index, QWidget* MainWindow)
{
    QMutexLocker locker(&mutex);
    m_Name=devicename;
    IDevice::init(Index,MainWindow);

    addJackStereoOut(0);
    addJackDualMonoOut(1);
    addJackMIDIOut(3);
    addJackModulationOut(4,"Modulation Out");
    addJackModulationOut(5,"Frequency Out");
    addJackModulationOut(6,"Trigger Out");
    addJackStereoIn();
    addJackDualMonoIn();
    addJackMIDIIn();
    addJackModulationIn("Modulation In");
    addJackModulationIn("Frequency In");
    addJackModulationIn("Trigger In");
    addParameterMIDIChannel();
    addParameter(CParameter::Numeric,"Preset","",1,MaxPresets,0,"",1);

    m_Form=new CMacroBoxForm(this,MainWindow);
    CDesktopContainer* d=form()->DesktopContainer;

    auto l = dynamic_cast<QVBoxLayout*>(m_Form->layout());
    l->removeWidget(d);
    buttonPanel = new QSynthButtonPanel(m_Form);
    buttonPanel->setFixedHeight(100);
    buttonPanel->init(MaxPresets,"Preset");
    CDesktopComponent::connect(buttonPanel,&QSynthButtonPanel::valueChanged,m_Form,qOverload<QString,int>(&CSoftSynthsForm::setParameter));
    l->addWidget(buttonPanel);
    l->addWidget(d);

    for (QDomLiteElement& e : presetList) e.tag="Preset";

    for (uint i=0;i<m_Jacks.size();i++)
    {
        IJack* J=m_Jacks[i];
        IJack* J1=d->Desktop->addJack(J->createInsideJack(int(i),this),0);
        JacksCreated.append(J1);
        (J->isOutJack()) ? InsideJacks.append(dynamic_cast<CInJack*>(J1)) : InsideJacks.append(dynamic_cast<CInJack*>(J));
    }
    currentIndex = -1;
    updateDeviceParameter();
}

CAudioBuffer* CPresetBox::getNextA(const int ProcIndex)
{
    return InsideJacks[ProcIndex]->getNextA();
}

CMIDIBuffer* CPresetBox::getNextP(const int ProcIndex)
{
    return InsideJacks[ProcIndex]->getNextP();
}

float CPresetBox::getNext(const int ProcIndex)
{
    return InsideJacks[ProcIndex]->getNext();
}

void CPresetBox::savePreset(const int index)
{
    if (currentIndex > -1)
    {
        QMutexLocker locker(&mutex);
        QDomLiteElement& XMLPreset = presetList[index];
        XMLPreset.clearChildren();
        for (int i = 0; i < deviceList()->deviceCount(); i++)
        {
            IDevice* d = deviceList()->device(i);
            QDomLiteElement* Device = XMLPreset.appendChild("Device","ID",d->deviceID());
            d->serializeParameters(Device);
        }
    }
}

void CPresetBox::loadPreset(const int index)
{
    QMutexLocker locker(&mutex);
    const QDomLiteElement& XMLPreset = presetList[index];
    {
        for (int i = 0; i < XMLPreset.childCount(); i++)
        {
            const QDomLiteElement* XMLDevice = XMLPreset.childElement(i);
            if (i < deviceList()->deviceCount())
            {
                IDevice* d = deviceList()->device(i);
                d->unserializeParameters(XMLDevice);
                deviceList()->updateParameter(i);
                desktopContainer()->showParameters(d);
            }
        }
        desktopComponent()->DrawConnections();
    }
}

void CPresetBox::updateDeviceParameter(const CParameter* /*p*/)
{
    if (currentIndex != preset())
    {
        QMutexLocker locker(&mutex);
        savePreset(currentIndex);
        loadPreset(preset());
        buttonPanel->setValue(preset()+1);
        currentIndex = preset();
    }
}

void CPresetBox::serializeCustom(QDomLiteElement* xml) const
{
    QDomLiteElement* XMLPresets = xml->appendChild("Presets");
    for (const QDomLiteElement& e : presetList) XMLPresets->appendChild(e.clone());
}

void CPresetBox::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    QMutexLocker locker(&mutex);
    for (QDomLiteElement& e : presetList) e.clearChildren();
    if (const QDomLiteElement* XMLpresets = xml->elementByTag("Presets"))
    {
        uint i = 0;
        for (const QDomLiteElement* Preset : (const QDomLiteElementList)XMLpresets->elementsByTag("Preset"))
        {
            if (i < MaxPresets) presetList[i++].copy(Preset);
        }
    }
    currentIndex = -1;
    updateDeviceParameter();
    form()->fillList(currentProgram());
}
