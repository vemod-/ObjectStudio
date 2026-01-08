#include "ccustomjacksdialog.h"
#include "ui_ccustomjacksdialog.h"
#include "cdevicelist.h"

CCustomJacksDialog::CCustomJacksDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CCustomJacksDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,&CCustomJacksDialog::acceptDialog);
    connect(ui->ApplyButton,&QPushButton::clicked,this,&CCustomJacksDialog::applyDialog);
    connect(ui->AddJackButton,&QToolButton::clicked,this,&CCustomJacksDialog::addJackClicked);
    connect(ui->DeleteJackButton,&QToolButton::clicked,this,&CCustomJacksDialog::removeJackClicked);
}

CCustomJacksDialog::~CCustomJacksDialog()
{
    delete ui;
}

void CCustomJacksDialog::fill(QList<CDesktopComponent*>* desktops, CDesktopComponent* desktop, CCustomJackList *p, IDevice *d) {
    m_Desktops = desktops;
    m_Desktop = desktop;
    m_CustomJacks = p;
    m_ParentDevice = d;
    ui->DeleteJackButton->setEnabled(false);
    for (int i = 0; i < ui->JacksList->topLevelItemCount(); i++) {
        ui->JacksList->topLevelItem(i)->setExpanded(true);
        ui->JacksList->topLevelItem(i)->setFlags(ui->JacksList->topLevelItem(i)->flags() & ~Qt::ItemIsSelectable);
    }
    m_CustomJacks->serialize(&m_xml,d);
    for (const QDomLiteElement* c : (const QDomLiteElementList)m_xml.elementsByTag("Jack")) {
        addElementToList(c);
    }
    connect(ui->JacksList,&QTreeWidget::itemClicked,this,&CCustomJacksDialog::selectCustomJack);
    connect(ui->JacksList,&QTreeWidget::itemChanged,this,&CCustomJacksDialog::editJackName);
    connect(ui->JacksList,&QTreeWidgetEx::itemsReordered,this,&CCustomJacksDialog::reorderJacks);
    connect(ui->JacksList,&QTreeWidget::currentItemChanged,this,&CCustomJacksDialog::itemSelectionChanged);
    ui->JacksList->setCurrentItem(outJacksItem());
}

void CCustomJacksDialog::selectCustomJack(QTreeWidgetItem *){
}

void CCustomJacksDialog::itemSelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem* ) {
    qDebug() << current->text(0) << current->text(1);
    if (!current->parent() && (current->childCount() > 0)) {
        QTimer::singleShot(0, ui->JacksList, [this, current](){
            if (current->childCount()) {
                ui->JacksList->setCurrentItem(current->child(0));
            }
            else {
                if (current == inJacksItem()) {
                    if (outJacksItem()->childCount()) ui->JacksList->setCurrentItem(outJacksItem()->child(0));
                }
                else if (current == outJacksItem()) {
                    if (inJacksItem()->childCount()) ui->JacksList->setCurrentItem(inJacksItem()->child(0));
                }
            }
        });
    }
}

void CCustomJacksDialog::editJackName(QTreeWidgetItem *i){
    QString caption = i->text(1);
    if (!caption.isEmpty()) currentCustomJack()->setAttribute("Alias",i->text(1));
}

void CCustomJacksDialog::reorderJacks(){
    QDomLiteElementList newOrder;
    for (int i = 0; i < ui->JacksList->topLevelItemCount(); i++) {
        for (int j = 0; j < ui->JacksList->topLevelItem(i)->childCount(); j++) {
            QDomLiteElement* e = customJackElement(ui->JacksList->topLevelItem(i)->child(j)->data(1,34).toString());
            newOrder.append(e);
        }
    }
    for (int i = 0; i < newOrder.count(); i++) {
        m_xml.exchangeChild(i,newOrder.at(i));
    }
}

void CCustomJacksDialog::addJackClicked(){
    const QList<IDevice*>* devices = m_Desktop->deviceList()->devices();
    QMenu* menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    if (devices->size() == 0) {
        menu->setEnabled(false);
        return;
    }
    for (const IDevice* d : *devices) {
        QSignalMenu* deviceMenu = new QSignalMenu(d->deviceID(),menu);
        menu->addMenu(deviceMenu);
        connect(deviceMenu,qOverload<QString>(&QSignalMenu::menuClicked),this,&CCustomJacksDialog::addCustomJack);
        for (int i = 0; i < d->jackCount(); i++) {
            QAction* a = deviceMenu->addAction(d->jack(i)->caption(),d->jackID(i));
            for (int j = 0; j < m_ParentDevice->jackCount(); j++) {
                if (m_Desktop->JacksCreated[j]->isConnectedTo(d->jack(i))) a->setEnabled(false);
            }
        }
    }
    menu->popup(cursor().pos());
}

void CCustomJacksDialog::addCustomJack(QString id){
    IJack* jack = m_Desktop->deviceList()->jack(id);
    if (jack) {
        QDomLiteElement* c = m_xml.appendChild("Jack","Name",jack->jackID());
        c->setAttribute("AttachMode",jack->attachMode);
        c->setAttribute("Direction",jack->direction);
        ui->JacksList->setCurrentItem(addElementToList(c));
    }
}

void CCustomJacksDialog::removeJackClicked(){
    if (ui->JacksList->currentItem()->parent()) {
        m_xml.removeChild(currentCustomJack());
        ui->JacksList->currentItem()->parent()->removeChild(ui->JacksList->currentItem());
    }
    if (outJacksItem()->childCount() + inJacksItem()->childCount() == 0) ui->DeleteJackButton->setEnabled(false);

}

void CCustomJacksDialog::acceptDialog(){
    applyDialog();
    accept();
}

void CCustomJacksDialog::applyDialog(){
    m_CustomJacks->unserialize(&m_xml,m_ParentDevice,m_Desktops);
}

QTreeWidgetItem* CCustomJacksDialog::addElementToList(const QDomLiteElement *xml) {
    QString caption = xml->attribute("Alias");
    if (caption.isEmpty()) caption = xml->attribute("Name");
    QTreeWidgetItem* i = new QTreeWidgetItem();
    i->setText(1,caption);
    i->setData(1,34,QString(xml->attribute("Name")));
    i->setFlags(i->flags() | Qt::ItemIsEditable);
    i->setFlags(i->flags() & ~Qt::ItemIsDropEnabled);
    if (xml->attributeValueInt("Direction") == 1) {
        outJacksItem()->addChild(i);
    }
    else {
        inJacksItem()->addChild(i);
    }
    ui->DeleteJackButton->setEnabled(true);
    return i;
}

QDomLiteElement *CCustomJacksDialog::customJackElement(const QString &customJackName) {
    for (QDomLiteElement* c : std::as_const(m_xml.childElements)) {
        if (c->attribute("Name") == customJackName) return c;
    }
    return nullptr;
}

QDomLiteElement *CCustomJacksDialog::currentCustomJack() {
    return customJackElement(ui->JacksList->currentItem()->data(1,34).toString());
}

QTreeWidgetItem *CCustomJacksDialog::inJacksItem() {
    return ui->JacksList->topLevelItem(1);
}

QTreeWidgetItem *CCustomJacksDialog::outJacksItem() {
    return ui->JacksList->topLevelItem(0);
}
