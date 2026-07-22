#include "ceffectrack.h"
#include <QLayout>
#include <QPushButton>
#include <QDrag>
#include "cparametersmenu.h"
#include "caddins.h"
#include "qdprpixmap.h"

CEffectRackForm::CEffectRackForm(IDevice* Device,QWidget* Parent) :
    CSoftSynthsForm(Device,false,Parent)
{
    auto ly=new QVBoxLayout(this);
    ly->setContentsMargins(0,0,0,0);
    ly->setSpacing(0);
    m_Rack = new CParametersContainer(this);
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
    QFont f = btnAdd->font();
    f.setPixelSize(24);
    btnAdd->setFont(f);
    QPalette pal = btnAdd->palette();
    pal.setColor(QPalette::ButtonText,Qt::white);
    btnAdd->setPalette(pal);
    btnAdd->setFlat(true);
    QDPRPixmap::setWidgetBackground(btnAdd,":/Black Aluminium Tile.jpg",QPalette::Button);
    btnAdd->setAutoFillBackground(true);
    tbly->addWidget(btnAdd);
    m_Toolbar->setFixedHeight(btnAdd->height());
    connect(btnAdd,&QPushButton::clicked,this,&CEffectRackForm::addDevice);
    PluginsPopup=new QSignalMenu("New Device",this);
    connect(PluginsPopup,SIGNAL(menuClicked(QString)),this,SLOT(PluginMenuClicked(QString)));
    //connect(m_Rack,&CParametersContainer::mousePressed,this,&CEffectRackForm::rackMousePressed);
    connect(m_Rack,&CParametersContainer::sizeChanged,this,&CEffectRackForm::rackSizeChanged);
    setMinimumWidth(800);
    //setAcceptDrops(true);
    connect(this,&CEffectRackForm::controlChanged,m_Rack,&CParametersContainer::updateControl,Qt::QueuedConnection);
    connect(this,&CEffectRackForm::connectionsChanged,m_Rack,&CParametersContainer::updateConnections,Qt::QueuedConnection);
    connect(m_Rack,&CParametersContainer::devicesReordered,this,&CEffectRackForm::reorderDevices);
    connect(m_Rack,&CParametersContainer::deviceRemoved,this,&CEffectRackForm::removeDevice);
}

CEffectRackForm::~CEffectRackForm(){
    m_Rack->clear();
    m_DeviceList.removeJack(insideIn);
    m_DeviceList.removeJack(insideOut);
    m_DeviceList.clear();
}

void CEffectRackForm::rackSizeChanged(){
    setMaximumHeight(m_Toolbar->height() + m_Rack->maximumHeight());
    setMinimumHeight(m_Toolbar->height() + m_Rack->minimumHeight());
    setGeometry(geometry().left(),geometry().top(),geometry().width(),m_Rack->height() + m_Toolbar->height());
    updateGeometry();
}
/*
void CEffectRackForm::rackMousePressed(IDevice* d, QPoint globalPos)
{
    if (m_DeviceList.deviceCount() < 1) return;
    const int index = m_DeviceList.indexOfDevice(d);
    QPoint viewPos = m_Rack->mapFromGlobal(globalPos);
    QPointF scenePos = m_Rack->mapToScene(viewPos);
    QPixmap pm;
    m_Rack->parametersPixmap(d, &pm);
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemdata", QByteArray(d->deviceID().toLatin1()));
    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pm);
    QPoint hotspot = scenePos.toPoint() - QPoint(0, 112 * index);
    drag->setHotSpot(hotspot);
    Qt::DropActions a = drag->exec(Qt::MoveAction);
    if (a == Qt::IgnoreAction) {
        QPoint viewCursor = m_Rack->mapFromGlobal(QCursor::pos());
        if (!m_Rack->rect().contains(viewCursor)) removeDevice(d);
    }
}
*/
void CEffectRackForm::init(CInJack* in, COutJack* out)
{
    QMutexLocker locker(&mutex);
    insideIn = in;
    insideOut = out;
    m_DeviceList.addJack(insideIn);
    m_DeviceList.addJack(insideOut);
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
    updateConnections();
}

void CEffectRackForm::PluginMenuClicked(QString AddInName)
{
    QMutexLocker locker(&mutex);
    const int MenuIndex=CAddIns::indexOf(AddInName);
    if (MenuIndex<0) return;
    IDevice* D = m_DeviceList.createDevice(instancefn(MenuIndex),m_DeviceList.findFreeIndex(AddInName),parentWidget());
    m_Rack->addDevice(D);
    updateConnections();
}

void CEffectRackForm::reorderDevices(int deviceIndex, int move) {
    if (m_DeviceList.moveDevice(deviceIndex, move)) {
        m_Rack->moveDevice(deviceIndex, move);
        updateConnections();
    }
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
    m_Rack->updateConnections();
}

int CEffectRackForm::deviceCount()
{
    return m_DeviceList.deviceCount();
}

void CEffectRackForm::unserializeCustom(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    for (IDevice* d : *m_DeviceList.devices()) m_Rack->removeDevice(d);
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
            if (d) {
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
    setMaximumHeight((m_Rack->unitHeight()*m_DeviceList.deviceCount())+m_Toolbar->height());
    if (m_DeviceList.deviceCount() == 0) {
        setMinimumHeight(m_Toolbar->height());
    }
    else {
        setMinimumHeight(m_Rack->unitHeight() + m_Toolbar->height());
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
        const int d = m_DeviceList.indexOfDevice(device);
        if (parameter)
        {
            emit controlChanged(device,parameter);
        }
        else {
            m_Rack->showParameters(device);
        }
        if (d > -1) m_DeviceList.updatePolyParameter(d,parameter);
    }
}

void CEffectRackForm::updateDeviceJacks() {
    emit connectionsChanged();
}

void CEffectRackForm::closeAutomation(IDevice* /*device*/) {
}

bool CEffectRackForm::event(QEvent* e) {
    if (e->type()==QEvent::NonClientAreaMouseButtonPress)
    {
        if (dynamic_cast<QMouseEvent*>(e)->button()==Qt::RightButton)
        {
            CParametersMenu* m = new CParametersMenu(m_Device,this,false);
            //m->setAttribute(Qt::WA_DeleteOnClose,true);
            connect(m,&CParametersMenu::parametersChanged,this,&CEffectRackForm::updateConnections);
            m->popup(dynamic_cast<QMouseEvent*>(e)->globalPosition().toPoint());
        }
    }
    return CSoftSynthsForm::event(e);
}
/*
void CEffectRackForm::dragEnterEvent(QDragEnterEvent *e) {
    if (e->mimeData()->hasFormat("application/x-dnditemdata")) {
        e->acceptProposedAction();
    }
}

void CEffectRackForm::dropEvent(QDropEvent *e) {
    QByteArray b(e->mimeData()->data("application/x-dnditemdata"));
    int deviceIndex =
        m_DeviceList.indexOfDevice(
            m_DeviceList.device(QString::fromLatin1(b)));

    QPointF scenePos = m_Rack->mapToScene(e->position().toPoint());
    int newIndex = scenePos.y() / 112;//m_Rack->unitHeight();

    int move = newIndex - deviceIndex;

    if (m_DeviceList.moveDevice(deviceIndex, move)) {
        e->acceptProposedAction();
        m_Rack->moveDevice(deviceIndex, move);
        updateConnections();
    }
}
*/
CEffectRack::CEffectRack()
{
    delete m_Form;
}

void CEffectRack::init(const int Index, QWidget* MainWindow)
{
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackStereoIn();
    addJackStereoOut(stereoout);
    m_Form=new CEffectRackForm(this,MainWindow);
    FORMFUNC(CEffectRackForm)->init((CInJack*)m_Jacks[1]->createInsideJack(4,this), (COutJack*)m_Jacks[0]->createInsideJack(jnInsideIn,this));
    FORMFUNC(CEffectRackForm)->updateConnections();
    updateDeviceParameter();
}

void CEffectRack::process()
{
    InBuffer = FetchAStereo(stereoin);
}

CAudioBuffer* CEffectRack::getNextA(const int ProcIndex)
{
    if (m_Process)
    {
        m_Process=false;
        process();
    }
    if (ProcIndex==jnInsideIn) return InBuffer;
    if (ProcIndex==stereoout)
    {
        if (!FORMFUNC(CEffectRackForm)->deviceCount()) return InBuffer;
        if (!FORMFUNC(CEffectRackForm)->insideIn->outJackCount()) return nullptr;
        //if (!InBuffer) return nullptr;
        //if (!InBuffer->isValid()) return nullptr;
        return FORMFUNC(CEffectRackForm)->insideIn->getNextA();
    }
    return nullptr;
}

void CEffectRack::mixerChannelProc(CStereoBuffer* buffer) {
    if (!FORMFUNC(CEffectRackForm)->deviceCount()) return;
    if (!buffer->isValid()) return;
    InBuffer = buffer;
    m_Process = false;
    CStereoBuffer* b = (CStereoBuffer*)getNextA(stereoout);
    if (!b) return;
    buffer->writeStereoBuffer(b);
}

void inline CEffectRack::updateDeviceParameter(const CParameter* /*p*/)
{
}

