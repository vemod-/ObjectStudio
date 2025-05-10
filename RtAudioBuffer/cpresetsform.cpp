#include "cpresetsform.h"
#include "ui_cpresetsform.h"
#include "cpresets.h"
#include <QPushButton>
#include <QFileDialog>

CPresetsForm::CPresetsForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CPresetsForm)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    setWindowTitle("Object Studio Presets");
    connect(ui->buttonBox->button(QDialogButtonBox::Apply),&QAbstractButton::clicked,this,&CPresetsForm::apply);
    connect(ui->VSTPathButton,&QAbstractButton::clicked,this,&CPresetsForm::selectFolder);
}

CPresetsForm::~CPresetsForm()
{
    delete ui;
}

void CPresetsForm::fill(const QList<uint> sampleRates)
{
    for (uint i: sampleRates) ui->SampleRateCombo->addItem(QString::number(i));
    ui->SampleRateCombo->setCurrentText(QString::number(CPresets::presets().SampleRate));
    for (uint i = 64; i < 10000; i*=2 )
    {
        ui->BufferSizeCombo->addItem(QString::number(i));
        ui->ModulationRateCombo->addItem(QString::number(i));
    }
    ui->ModulationRateCombo->setCurrentText(QString::number(CPresets::presets().ModulationRate));
    ui->BufferSizeCombo->setCurrentText(QString::number(CPresets::presets().BufferSize));
    ui->VSTPathEdit->setText(CPresets::presets().VSTPath);
}

void CPresetsForm::apply()
{
    CPresets::presets().SampleRate=ui->SampleRateCombo->currentText().toInt();
    CPresets::presets().BufferSize=ui->BufferSizeCombo->currentText().toInt();
    CPresets::presets().ModulationRate=ui->ModulationRateCombo->currentText().toInt();
    CPresets::presets().VSTPath=ui->VSTPathEdit->text();
    this->accept();
}

void CPresetsForm::selectFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, "VST Path", ui->VSTPathEdit->text(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) ui->VSTPathEdit->setText(dir);
}
