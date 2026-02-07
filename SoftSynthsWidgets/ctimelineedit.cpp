#include "ctimelineedit.h"
#include "ui_ctimelineedit.h"
#include <QWidgetAction>

CTimeLineEdit::CTimeLineEdit(CTimeLine* t, QWidget* parent)
    : QWidget(parent),
    ui(new Ui::CTimeLineEdit)
{
    ui->setupUi(this);
    m_TimeLine = t;
    ui->TempoSpin->setRange(10,300);
    ui->UpperSpin->setRange(1,48);
    ui->ViewCombo->addItems({"Time","Bars"});
    ui->LowerCombo->addItems({"1","2","4","8","16"});
    QDomLiteElement e;
    m_TimeLine->serialize(&e);
    if (QDomLiteElement* e1 = e.elementByTag("TimeLine")) {
        ui->ViewCombo->setCurrentIndex(e1->attributeValueInt("View"));
        ui->TempoSpin->setValue(e1->attributeValue("Tempo"));
        ui->UpperSpin->setValue(e1->attributeValueInt("Upper"));
        ui->LowerCombo->setCurrentText(QString::number(e1->attributeValueInt("Lower")));
    }
}

void CTimeLineEdit::hideEvent(QHideEvent * /*event*/) {
    QDomLiteElement e;
    if (QDomLiteElement* e1 = e.appendChild("TimeLine")) {
        e1->setAttribute("View",ui->ViewCombo->currentIndex());
        e1->setAttribute("Tempo",ui->TempoSpin->value());
        e1->setAttribute("Upper",ui->UpperSpin->value());
        e1->setAttribute("Lower",ui->LowerCombo->currentText().toInt());
    }
    m_TimeLine->unserialize(&e);
    emit Changed();
}

CTimeLineEdit::~CTimeLineEdit()
{
    delete ui;
}

CTimeLineMenu::CTimeLineMenu(CTimeLine *t, QWidget *parent)
    : QMenu(parent) {
    //setAttribute(Qt::WA_DeleteOnClose);
    QWidgetAction* a = new QWidgetAction(this);
    m_Edit = new CTimeLineEdit(t,this);
    a->setDefaultWidget(m_Edit);
    addAction(a);
    connect(m_Edit,&CTimeLineEdit::Changed,this,&CTimeLineMenu::Changed);
    m_Edit->show();
    m_Edit->setFixedSize(m_Edit->sizeHint());
    m_Edit->updateGeometry();
    setFixedSize(m_Edit->size()+QSize(10,10));
}

void CTimeLineMenu::closeEvent(QCloseEvent *) {
    m_Edit->deleteLater();
    deleteLater();
}
