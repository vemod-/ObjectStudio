#include "cmidinoteedit.h"
#include "ui_cmidinoteedit.h"
//#include <QDomLite>
#include "cpitchtextconvert.h"

CMIDINoteEdit::CMIDINoteEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CMIDINoteEdit)
{
    ui->setupUi(this);
    connect(ui->comboBox,qOverload<int>(&QComboBox::currentIndexChanged),this,&CMIDINoteEdit::fromSpin);
    m_Minimum = 1;
    m_Maximum = 127;
    fill();
}

void CMIDINoteEdit::fill()
{
    ui->comboBox->clear();
    for (int i = m_Minimum; i <= m_Maximum; i++)
    {
        ui->comboBox->addItem(CPitchTextConvert::pitch2Text(i));
    }
}

CMIDINoteEdit::~CMIDINoteEdit()
{
    delete ui;
}

int CMIDINoteEdit::value()
{
    return ui->comboBox->currentIndex() + m_Minimum;
}

void CMIDINoteEdit::setValue(int val)
{
    ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentIndex(val - m_Minimum);
    ui->comboBox->blockSignals(false);
}

void CMIDINoteEdit::fromSpin(int val)
{
    emit Changed(val + m_Minimum);
}

void CMIDINoteEdit::setMaximum(int val)
{
    m_Maximum = val;
    ui->comboBox->blockSignals(true);
    fill();
    setValue(qMin<int>(value(),m_Maximum));
    ui->comboBox->blockSignals(false);
}

void CMIDINoteEdit::setMinimum(int val)
{
    m_Minimum = val;
    ui->comboBox->blockSignals(true);
    fill();
    setValue(qMax<int>(value(),m_Minimum));
    ui->comboBox->blockSignals(false);
}
