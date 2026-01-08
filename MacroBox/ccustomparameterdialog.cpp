#include "ccustomparameterdialog.h"
#include "ui_ccustomparameterdialog.h"
#include "qsignalmenu.h"

CCustomParameterDialog::CCustomParameterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CCustomParameterDialog)
{
    ui->setupUi(this);
    connect(ui->ConnectTree,&QTreeWidget::itemClicked,this,&CCustomParameterDialog::itemClicked);
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,&CCustomParameterDialog::acceptDialog);
    connect(ui->ApplyButton,&QPushButton::clicked,this,&CCustomParameterDialog::applyDialog);
    connect(ui->addParameterButton,&QToolButton::clicked,this,&CCustomParameterDialog::addParameterClicked);
    connect(ui->removeParameterButton,&QToolButton::clicked,this,&CCustomParameterDialog::removeParameterClicked);
}

CCustomParameterDialog::~CCustomParameterDialog()
{
    delete ui;
}

void CCustomParameterDialog::fill(CDeviceList *l, CCustomParameterList *p, IDevice* d) {
    m_DeviceList = l;
    m_CustomParameters = p;
    m_ParentDevice = d;
    m_CustomParameters->serialize(&m_xml);
    ui->removeParameterButton->setEnabled(false);
    for (const QDomLiteElement* c : (const QDomLiteElementList)m_xml.elementsByTag("CustomParameter")) {
        QListWidgetItem* i = new QListWidgetItem(c->attribute("Name"));
        i->setFlags(i->flags() | Qt::ItemIsEditable);
        ui->parameterList->addItem(i);
    }
    connect(ui->parameterList,&QListWidget::itemClicked,this,&CCustomParameterDialog::selectParameter);
    connect(ui->parameterList,&QListWidget::itemChanged,this,&CCustomParameterDialog::editParameterName);
    connect(ui->parameterList,&QListWidgetEx::itemsReordered,this,&CCustomParameterDialog::reorderParameters);
    connect(ui->TypeCombo,&QComboBox::currentIndexChanged,this,&CCustomParameterDialog::editType);
    connect(ui->UnitEdit,&QLineEdit::textChanged,this,&CCustomParameterDialog::editUnit);
    connect(ui->List,&QPlainTextEdit::textChanged,this,&CCustomParameterDialog::editList);
    connect(ui->FactorCombo,&QComboBox::currentTextChanged,this,&CCustomParameterDialog::editDecimal);
    connect(ui->MinSpin,&QDoubleSpinBox::valueChanged,this,&CCustomParameterDialog::editMin);
    connect(ui->MaxSpin,&QDoubleSpinBox::valueChanged,this,&CCustomParameterDialog::editMax);
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
    if (ui->parameterList->count()) {
        ui->removeParameterButton->setEnabled(true);
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
    ui->TypeCombo->blockSignals(true);
    ui->UnitEdit->blockSignals(true);
    ui->List->blockSignals(true);
    ui->TypeCombo->blockSignals(true);
    ui->FactorCombo->blockSignals(true);
    ui->MinSpin->blockSignals(true);
    ui->MaxSpin->blockSignals(true);
    for (const QDomLiteElement* c : std::as_const(m_xml.childElements)) {
        if (c->attribute("Name") == item->text()) {
            ui->TypeCombo->setCurrentIndex(c->attributeValueInt("Type"));
            ui->UnitEdit->setText(c->attribute("Unit"));
            ui->MinSpin->setDecimals(0);
            ui->MinSpin->setValue(c->attributeValueInt("Min"));
            ui->MaxSpin->setDecimals(0);
            ui->MaxSpin->setValue(c->attributeValueInt("Max"));
            ui->FactorCombo->setCurrentText(c->attribute("DecimalFactor"));
            ui->List->clear();
            ui->List->setPlainText(QString(c->attribute("ListString")).replace("§","\n"));
            ui->UnitEdit->setEnabled(true);
            ui->MinSpin->setEnabled(true);
            ui->MaxSpin->setEnabled(true);
            ui->FactorCombo->setEnabled(true);
            ui->List->setEnabled(true);
            switch (c->attributeValueInt("Type")) {
            case 0: //Numeric
                ui->List->setEnabled(false);
                break;
            case 1: //Select
                ui->UnitEdit->setEnabled(false);
                ui->MinSpin->setEnabled(false);
                ui->MaxSpin->setEnabled(false);
                ui->FactorCombo->setEnabled(false);
                break;
            case 2: //dB
                ui->UnitEdit->setEnabled(false);
                ui->FactorCombo->setEnabled(false);
                ui->MinSpin->setDecimals(2);
                ui->MinSpin->setValue(lin2dB(c->attributeValueInt("Min")*0.01));
                ui->MaxSpin->setDecimals(2);
                ui->MaxSpin->setValue(lin2dB(c->attributeValueInt("Max")*0.01));
                ui->List->setEnabled(false);
                break;
            case 3: //Percent
                ui->UnitEdit->setEnabled(false);
                ui->FactorCombo->setEnabled(false);
                ui->List->setEnabled(false);
            }
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
    ui->TypeCombo->blockSignals(false);
    ui->UnitEdit->blockSignals(false);
    ui->List->blockSignals(false);
    ui->TypeCombo->blockSignals(false);
    ui->FactorCombo->blockSignals(false);
    ui->MinSpin->blockSignals(false);
    ui->MaxSpin->blockSignals(false);
}

void CCustomParameterDialog::editParameterName(QListWidgetItem *i){
    currentCustomParameter()->setAttribute("Name",i->text());
}

void CCustomParameterDialog::editType(int t){
    currentCustomParameter()->setAttribute("Type",t);
    selectParameter(ui->parameterList->currentItem());
}

void CCustomParameterDialog::editUnit(QString s){
    currentCustomParameter()->setAttribute("Unit",s);
}

void CCustomParameterDialog::editMin(double v){
    if (currentCustomParameter()->attributeValueInt("Type") == 2) {
        currentCustomParameter()->setAttribute("Min",dB2lin(v));
    } else {
        currentCustomParameter()->setAttribute("Min",v);
    }
}

void CCustomParameterDialog::editMax(double v){
    if (currentCustomParameter()->attributeValueInt("Type") == 2) {
        currentCustomParameter()->setAttribute("Max",dB2lin(v));
    } else {
        currentCustomParameter()->setAttribute("Max",v);
    }
}

void CCustomParameterDialog::editList(){
    QString s = ui->List->toPlainText().replace("\n","§");
    currentCustomParameter()->setAttribute("ListString",s);
    const int l = s.split("§").count() - 1;
    currentCustomParameter()->setAttribute("Max",l);
    ui->MaxSpin->setValue(l);
}

void CCustomParameterDialog::editDecimal(QString s){
    currentCustomParameter()->setAttribute("DecimalFactor",s.toInt());
}

void CCustomParameterDialog::addParameterClicked(){
    QMenu* menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    for (const IDevice* d : *m_DeviceList->devices()) {
        QSignalMenu* deviceMenu = new QSignalMenu(d->deviceID(), menu);
        menu->addMenu(deviceMenu);
        connect(deviceMenu,qOverload<QString>(&QSignalMenu::menuClicked),this,&CCustomParameterDialog::addParameter);
        for (int i = 0; i < d->parameterCount(); i++) {
            const QString pid = CParameterID::parameterID(d->deviceID(), d->parameter(i));
            QAction* a = deviceMenu->addAction(d->parameter(i)->Name,pid);
            for (const QDomLiteElement* p : (const QDomLiteElementList)m_xml.elementsByTag("Parameter",true)) {
                if (p->attribute("ParameterID") == pid) {
                    a->setEnabled(false);
                    break;
                }
            }
        }
    }
    menu->popup(cursor().pos());
}

void CCustomParameterDialog::addParameter(QString id){
    const CParameterID pid(id);
    CParameter* parameter = m_DeviceList->device(pid.DeviceID)->parameter(pid.ParameterName);
    if (parameter) {
        QDomLiteElement* c = m_xml.appendChild("CustomParameter","Name",pid.defaultCaption());
        c->setAttribute("Type",parameter->Type);
        c->setAttribute("Unit",parameter->Unit);
        c->setAttribute("Min",parameter->Min);
        c->setAttribute("Max",parameter->Max);
        c->setAttribute("DecimalFactor",parameter->DecimalFactor);
        c->setAttribute("ListString",parameter->List);
        c->appendChild("Parameter","ParameterID",id);
        QListWidgetItem* i = new QListWidgetItem(pid.defaultCaption());
        i->setFlags(i->flags() | Qt::ItemIsEditable);
        ui->parameterList->addItem(i);
        i->setSelected(true);
        selectParameter(i);
    }
}

void CCustomParameterDialog::removeParameterClicked(){
    m_xml.removeChild(currentCustomParameter());
    delete ui->parameterList->takeItem(ui->parameterList->currentRow());
    if (ui->parameterList->count()) {
        selectParameter(ui->parameterList->currentItem());
    }
    else {
        ui->removeParameterButton->setEnabled(false);
    }
}

void CCustomParameterDialog::acceptDialog(){
    applyDialog();
    accept();
}

void CCustomParameterDialog::reorderParameters(){
    QDomLiteElementList newOrder;
    for (int i = 0; i < ui->parameterList->count(); i++) {
        QDomLiteElement* e = customParameterElement(ui->parameterList->item(i)->text());
        newOrder.append(e);
    }
    for (int i = 0; i < newOrder.count(); i++) {
        m_xml.exchangeChild(i,newOrder.at(i));
    }
}

void CCustomParameterDialog::applyDialog(){
    for (const CCustomParameter& c : *std::as_const(m_CustomParameters)) {
        qDebug() << "removeParameter" << c.masterParameter->Name;
        m_DeviceList->removeCustomParameter(m_ParentDevice,c.masterParameter->Name);
    }
    m_CustomParameters->clear();
    m_ParentDevice->updateHostParameter();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    m_CustomParameters->unserialize(&m_xml,m_DeviceList,m_ParentDevice);
    m_ParentDevice->updateHostParameter();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

QDomLiteElement *CCustomParameterDialog::customParameterElement(const QString &customParameterName) {
    for (QDomLiteElement* c : std::as_const(m_xml.childElements)) {
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

QDomLiteElement *CCustomParameterDialog::currentCustomParameter() {
    return m_xml.childElement(ui->parameterList->currentRow());
}
