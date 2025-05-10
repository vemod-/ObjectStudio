#include "ceffectrack.h"
#include <QLayout>
#include <QPushButton>
#include <QDrag>
#include "mouseevents.h"
#include "cparametersmenu.h"
#include "caddins.h"

CEffectRackForm::CEffectRackForm(IDevice* Device,QWidget* Parent) :
    CSoftSynthsForm(Device,false,Parent)
{
    auto ly=new QVBoxLayout(this);
    ly->setContentsMargins(0,0,0,0);
    ly->setSpacing(0);
    m_Rack = new CRackContainer(this);
    ly->addWidget(m_Rack);
    m_Rack->Init(&m_DeviceList);
    Device->addTickerDevice(&m_DeviceList);
    Device->setDeviceParent(&m_DeviceList);
    m_DeviceList.setHost(this);
    m_Toolbar = new QWidget(this);
    ly->addWidget(m_Toolbar);
    auto tbly = new QHBoxLayout(m_Toolbar);
    tbly->setContentsMargins(0,0,0,0);
    tbly->setSpacing(0);
    auto btnAdd = new QPushButton("+",m_Toolbar);
    btnAdd->setFlat(true);
    btnAdd->setStyleSheet("color:white;background-image:url(:/Black Aluminium Tile.jpg);font-size:24px;");
    tbly->addWidget(btnAdd);
    m_Toolbar->setFixedHeight(btnAdd->height());
    connect(btnAdd,&QPushButton::clicked,this,&CEffectRackForm::addDevice);
    PluginsPopup=new QSignalMenu("New Device",this);
    connect(PluginsPopup,SIGNAL(menuClicked(QString)),this,SLOT(PluginMenuClicked(QString)));
    connect(m_Rack,&CRackContainer::mousePress,this,&CEffectRackForm::rackMousePressed);
    //connect(m_Rack,&CRackContainer::automationRequested,this,&CEffectRackForm::showAutomation);
    setMinimumWidth(800);

    DropEvent* ev = new DropEvent();
    QWidget* rackWidget = (QWidget*)m_Rack->widget();
    rackWidget->setAcceptDrops(true);
    rackWidget->installEventFilter(ev);
    connect(ev,&DropEvent::Drop,this,&CEffectRackForm::rackDrop);
    connect(ev,&DropEvent::DragEnter,this,&CEffectRackForm::rackDragEnter);
    connect(this,&CEffectRackForm::controlChanged,m_Rack,&CRackContainer::updateControl,Qt::QueuedConnection);
}

void CEffectRackForm::rackDrop(QDropEvent* e)
{
    QByteArray b(e->mimeData()->data("application/x-dnditemdata"));
    qDebug() << "Drop" << e->position() << QString::fromLatin1(b);
    int deviceIndex = m_DeviceList.indexOfDevice(m_DeviceList.device(QString::fromLatin1(b)));
    int newIndex = e->position().y() / 112;
    int move = newIndex - deviceIndex;
    qDebug() << "move" << move;
    if (m_DeviceList.moveDevice(deviceIndex, move))
    {
        e->setAccepted(true);
        m_Rack->moveDevice(deviceIndex, move);
        updateConnections();
    }
}

void CEffectRackForm::rackDragEnter(QDragEnterEvent* e)
{
    qDebug() << "DragEnter" << e->position();
    e->acceptProposedAction();
}

void CEffectRackForm::rackMousePressed(IDevice* d, QPoint p)
{
    if (m_DeviceList.deviceCount() < 1) return;
    qDebug() << d->deviceID() << p;
    const int index = m_DeviceList.indexOfDevice(d);
    QWidget* rackWidget = m_Rack->widget();
    qDebug() << rackWidget->rect();
    QPixmap pm = rackWidget->grab().scaled(rackWidget->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    pm.setDevicePixelRatio(1);
    QList<QPixmap> l;
    for (int i = 0; i < m_DeviceList.deviceCount(); i++)
    {
        l.append(pm.copy(0,i*122,width(),122));
    }

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemdata", QByteArray(d->deviceID().toLatin1()));
    QDrag* drag = new QDrag(rackWidget);
    drag->setMimeData(mimeData);
    drag->setPixmap(l[index]);
    drag->setHotSpot(rackWidget->mapFromGlobal(p) - QPoint(0,112 * index));
    Qt::DropActions a = drag->exec(Qt::MoveAction);
    qDebug() << drag->target();
    if ((a == Qt::IgnoreAction) && (!rackWidget->rect().contains(mapFromGlobal(QCursor::pos())))) removeDevice(d);
}

void CEffectRackForm::init(CInJack* in, COutJack* out)
{
    QMutexLocker locker(&mutex);
    insideIn = in;
    insideOut = out;
    m_DeviceList.addJack(insideIn);
    m_DeviceList.addJack(insideOut);
    setMaximumHeight(m_Toolbar->height());
}

void CEffectRackForm::addDevice()
{
    PluginsPopup->clear();
    const QStringList plugs=CAddIns::addInNames();
    for (const QString& p : plugs) PluginsPopup->addAction(p,p);
    PluginsPopup->popup(pos());
}

void CEffectRackForm::removeDevice(IDevice* d)
{
    QMutexLocker locker(&mutex);
    m_DeviceList.removeDevice(d);
    m_Rack->removeDevice(d);
    delete d;
    setMaximumHeight((112*m_DeviceList.deviceCount())+m_Toolbar->height());
    if (m_DeviceList.deviceCount() == 0) {
        setMinimumHeight(m_Toolbar->height());
    }
    else {
        setMinimumHeight(112 + m_Toolbar->height());
    }
    adjustSize();
    updateConnections();
}

void CEffectRackForm::PluginMenuClicked(QString AddInName)
{
    QMutexLocker locker(&mutex);
    const int MenuIndex=CAddIns::indexOf(AddInName);
    if (MenuIndex<0) return;
    IDevice* D=m_DeviceList.createDevice(instancefn(MenuIndex),m_DeviceList.findFreeIndex(AddInName),parentWidget());
    m_Rack->addDevice(D);
    setMaximumHeight((112*m_DeviceList.deviceCount())+m_Toolbar->height());
    setMinimumHeight(112 + m_Toolbar->height());
    updateConnections();
}

void CEffectRackForm::updateConnections()
{
    QMutexLocker locker(&mutex);
    QList<IDevice*> audioDevices;
    for (int i = 0; i < m_DeviceList.deviceCount(); i++)
    {
        if ((m_DeviceList.device(i)->inJack(IJack::Audio)) && (m_DeviceList.device(i)->outJack(IJack::Audio)))
        {
            audioDevices.append(m_DeviceList.device(i));
        }
    }
    for (IDevice* d :audioDevices)
    {
        m_DeviceList.disconnectJack(d->inJack(IJack::Audio));
        m_DeviceList.disconnectJack(d->outJack(IJack::Audio));
    }
    //m_DeviceList.disconnectAll();
    for (int i = 0; i < audioDevices.size() - 1; i++)
    {
        CInJack* in = audioDevices[i + 1]->inJack(IJack::Audio);
        COutJack* out = audioDevices[i]->outJack(IJack::Audio);
        m_DeviceList.connect(in->jackID(),out->jackID());
    }
    if (!audioDevices.empty())
    {
        m_DeviceList.connect(insideOut->jackID(),audioDevices.first()->inJack(IJack::Audio)->jackID());
        m_DeviceList.connect(insideIn->jackID(),audioDevices.last()->outJack(IJack::Audio)->jackID());
    }
    m_Rack->drawConnections();
}

int CEffectRackForm::deviceCount()
{
    return m_DeviceList.deviceCount();
}

void CEffectRackForm::unserializeCustom(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    m_DeviceList.clear();
    if (QDomLiteElement* Items = xml->elementByTag("Items"))
    {
        for (const QDomLiteElement* Device : (const QDomLiteElementList)Items->elementsByTag("Device"))
        {
            const QString Name=Device->attribute("Type");
            const int Index=Device->attributeValueInt("Index");
            //qDebug() << CAddIns::addInNames();
            const int MenuIndex=CAddIns::indexOf(Name);
            IDevice* d = m_DeviceList.createDevice(instancefn(MenuIndex),Index,parentWidget());
            if (d)
            {
                m_Rack->addDevice(d);
                m_DeviceList.unserializeDevice(Device,d);
            }
        }
        m_Rack->unserialize(Items);
        for (const QDomLiteElement* XMLConnection : (const QDomLiteElementList)Items->elementsByTag("Connection")) {
            QString InJack=XMLConnection->attribute("InJack");
            QString OutJack=XMLConnection->attribute("OutJack");
            m_DeviceList.connect(InJack,OutJack);
        }
    }
    setMaximumHeight((112*m_DeviceList.deviceCount())+m_Toolbar->height());
    if (m_DeviceList.deviceCount() == 0) {
        setMinimumHeight(m_Toolbar->height());
    }
    else {
        setMinimumHeight(112 + m_Toolbar->height());
    }
    updateConnections();
}

void CEffectRackForm::serializeCustom(QDomLiteElement* xml) const
{
    QDomLiteElement* Items=xml->appendChild("Items");
    for (int i = 0; i < m_DeviceList.deviceCount(); i++) {
        IDevice* d = m_DeviceList.device(i);
        QDomLiteElement* Device = Items->appendChild("Device");
        Device->setAttribute("Index",d->index());
        Device->setAttribute("Type",d->name());
        Device->setAttribute("ClassName",QString(d->name()+".dll"));
        d->serializeDevice(Device);
    }
    m_Rack->serialize(Items);
    for (int i = 0; i < m_DeviceList.inJackCount(); i++) {
        CInJack* j = m_DeviceList.inJack(i);
        for (int i = 0; i < j->outJackCount(); i++) {
            QDomLiteElement* Connection = Items->appendChild("Connection","InJack",j->jackID());
            Connection->setAttribute("OutJack",j->outJack(i)->jackID());
        }

    }
}

void CEffectRackForm::parameterChange(IDevice* device, const CParameter* parameter)
{
    if (device)
    {
        if (parameter)
        {
            //QMutexLocker locker(&mutex);
            emit controlChanged(device,parameter);
            const int d = m_DeviceList.indexOfDevice(device);
            if (d > -1) m_DeviceList.updateParameter(d,parameter);
        }
    }
}

bool CEffectRackForm::event(QEvent* e) {
    if (e->type()==QEvent::NonClientAreaMouseButtonPress)
    {
        if (dynamic_cast<QMouseEvent*>(e)->button()==Qt::RightButton)
        {
            //actionPasteParameters->setEnabled(QApplication::clipboard()->text().startsWith("<Parameters"));
            //parametersMenu->popup(mapToGlobal(dynamic_cast<QMouseEvent*>(event)->pos()));
            CParametersMenu* m = new CParametersMenu(m_Device,this,false);
            m->setAttribute(Qt::WA_DeleteOnClose,true);
            connect(m,&CParametersMenu::parametersChanged,this,&CEffectRackForm::updateConnections);
            //m->addSeparator();
            //m->addAction("UI map",ui->DesktopContainer,&CDesktopContainer::showMap);
            //m->addAction("Hide UIs",ui->DesktopContainer,&CDesktopContainer::hideUIs);
            //m->addAction("Cascade UIs",this,&CMacroBoxForm::cascadeUIs);

            m->popup(mapToGlobal(dynamic_cast<QMouseEvent*>(e)->pos()));
        }
    }
    return CSoftSynthsForm::event(e);
}

/*
void CEffectRackForm::showAutomation(IDevice* d)
{
    m_Rack->showAutomation(d, &m_DeviceList);
}
*/
CEffectRack::CEffectRack()
{
}

void CEffectRack::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoIn();
    addJackStereoOut(jnOut);
    m_Form=new CEffectRackForm(this,MainWindow);
    FORMFUNC(CEffectRackForm)->init((CInJack*)m_Jacks[1]->createInsideJack(4,this), (COutJack*)m_Jacks[0]->createInsideJack(jnInsideIn,this));
    FORMFUNC(CEffectRackForm)->updateConnections();
    updateDeviceParameter();
}
/*
void CEffectRack::tick()
{
    IDevice::tick();
}
*/
void CEffectRack::process()
{
    InBuffer=FetchAStereo(jnIn);
}

CAudioBuffer* CEffectRack::getNextA(const int ProcIndex)
{
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    if (ProcIndex==jnInsideIn) return InBuffer;
    if (ProcIndex==jnOut)
    {
        if (!InBuffer->isValid()) return nullptr;
        if (!FORMFUNC(CEffectRackForm)->deviceCount()) return InBuffer;
        return FORMFUNC(CEffectRackForm)->insideIn->getNextA();
    }
    return nullptr;
}

void CEffectRack::mixerChannelProc(CStereoBuffer* buffer) {
    if (!FORMFUNC(CEffectRackForm)->deviceCount()) return;
    if (!buffer->isValid()) return;
    InBuffer = buffer;
    m_Process = false;
    CStereoBuffer* b = (CStereoBuffer*)getNextA(jnOut);
    if (!b) return;
    buffer->writeStereoBuffer(b);
}

void inline CEffectRack::updateDeviceParameter(const CParameter* /*p*/)
{
}

