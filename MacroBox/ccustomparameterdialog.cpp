#include "ccustomparameterdialog.h"
#include "ui_ccustomparameterdialog.h"

CCustomParameterDialog::CCustomParameterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CCustomParameterDialog)
{
    ui->setupUi(this);
    connect(ui->ConnectTree,&QTreeWidget::itemClicked,this,&CCustomParameterDialog::itemClicked);
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,&CCustomParameterDialog::acceptDialog);
}

CCustomParameterDialog::~CCustomParameterDialog()
{
    delete ui;
}

void CCustomParameterDialog::fill(CDeviceList *l, CCustomParameterList *p, IDevice* d) {
    m_DeviceList = l;
    m_CustomParameters = p;
    m_ParentDevice = d;
    //connect(this,&CCustomParameterDialog::accepted,this,&CCustomParameterDialog::addParameter);
    for (CCustomParameter& c : *m_CustomParameters) {
        QListWidgetItem* i = new QListWidgetItem(c.Caption());
        i->setFlags(i->flags() | Qt::ItemIsEditable);
        ui->parameterList->addItem(i);
    }
    connect(ui->parameterList,&QListWidget::itemClicked,this,&CCustomParameterDialog::selectParameter);
    connect(ui->parameterList,&QListWidget::itemChanged,this,&CCustomParameterDialog::editParameterName);
    ui->TypeCombo->addItems({"Numeric","Select Box","Percent","dB"});
    const QList<IDevice*>* devices = l->devices();
    for (const IDevice* d : *devices) {
        QTreeWidgetItem* di = new QTreeWidgetItem();
        di->setText(0,d->deviceID());
        ui->ConnectTree->addTopLevelItem(di);
        for (int i = 0; i < d->parameterCount(); i++) {
            QTreeWidgetItem* j = new QTreeWidgetItem();
            j->setText(1,d->parameter(i)->Name);
            j->setCheckState(1,Qt::Unchecked);
            di->addChild(j);
        }
    }
    m_CustomParameters->serialize(&m_xml);
    if (ui->parameterList->count()) {
        ui->parameterList->item(0)->setSelected(true);
        selectParameter(ui->parameterList->item(0));
    }
}

void CCustomParameterDialog::itemClicked(QTreeWidgetItem *i, int col) {
    if (col == 1) {
        CParameterID id(i->parent()->text(0),i->text(1));
        if (QDomLiteElement* c = customParameterElement(ui->parameterList->currentItem()->text())) {
            if (QDomLiteElement* p = parameterElement(c, id.parameterID())) {
                c->removeChild(p);
            }
            else {
                c->appendChild("Parameter","ParameterID",id.parameterID());
            }
            selectParameter(ui->parameterList->currentItem());
        }
    }
}

void CCustomParameterDialog::selectParameter(QListWidgetItem *item){

    for (QDomLiteElement* c : (const QDomLiteElementList)m_xml.childElements) {
        if (c->attribute("Name") == item->text()) {
            ui->NameEdit->setText(c->attribute("Name"));
            ui->TypeCombo->setCurrentIndex(c->attributeValueInt("Type"));
            for (int i = 0; i < ui->ConnectTree->topLevelItemCount(); i++) {
                QTreeWidgetItem* t = ui->ConnectTree->topLevelItem(i);
                for (int j = 0; j < t->childCount(); j++) {
                    QTreeWidgetItem* p = t->child(j);
                    QString id = CParameterID::parameterID(t->text(0),p->text(1));
                    qDebug() << id << parameterElement(c,id);
                    if (parameterElement(c,id)) {
                        p->setCheckState(1,Qt::Checked);
                    } else {
                        p->setCheckState(1,Qt::Unchecked);
                    }
                }
            }
        }
    }
}

void CCustomParameterDialog::editParameterName(QListWidgetItem *i){
    m_xml.childElement(ui->parameterList->currentRow())->setAttribute("Name",i->text());
}

void CCustomParameterDialog::addParameter(){

}

void CCustomParameterDialog::acceptDialog(){
    for (CCustomParameter& c : *std::as_const(m_CustomParameters)) {
        qDebug() << "removeParameter" << c.masterParameter->Name;
        m_DeviceList->removeCustomParameter(m_ParentDevice,c.masterParameter->Name);
    }
    m_CustomParameters->clear();
    m_ParentDevice->updateHostParameter();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    m_CustomParameters->unserialize(&m_xml,m_DeviceList,m_ParentDevice);
    m_ParentDevice->updateHostParameter();
    accept();
}

QDomLiteElement *CCustomParameterDialog::customParameterElement(const QString &customParameterName) {
    for (QDomLiteElement* c : (const QDomLiteElementList)m_xml.childElements) {
        if (c->attribute("Name") == customParameterName) return c;
    }
    return nullptr;
}

QDomLiteElement *CCustomParameterDialog::parameterElement(const QString& customParameterName, const QString &parameterID) {
    if (QDomLiteElement* c = customParameterElement(customParameterName)) {
        return parameterElement(c, parameterID);
    }
    return nullptr;
}

QDomLiteElement *CCustomParameterDialog::parameterElement(const QDomLiteElement *c, const QString &parameterID) {
    const CParameterID id(parameterID);
    for (QDomLiteElement* p : (const QDomLiteElementList)c->elementsByTag("Parameter")) {
        if (p->attribute("ParameterID") == parameterID) return p;
    }
    return nullptr;
}

QTreeWidgetItem *CCustomParameterDialog::parameterItem(const QString &parameterID) {
    const CParameterID id(parameterID);
    for (int i = 0; i < ui->ConnectTree->topLevelItemCount(); i++) {
        const QTreeWidgetItem* t = ui->ConnectTree->topLevelItem(i);
        if (t->text(0) == id.DeviceID) {
            for (int j = 0; j < t->childCount(); j++) {
                if (t->child(j)->text(1) == id.ParameterName) {
                    return t->child(j);
                }
            }
        }
    }
    return nullptr;
}
