#include "cmacroboxform.h"
#include "ui_cmacroboxform.h"
#include <QInputDialog>
//#include <QMessageBox>
#include <QClipboard>
#include <QMenu>
#include "cparametersmenu.h"

CMacroBoxForm::CMacroBoxForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,false,parent),
    ui(new Ui::CMacroBoxForm)
{
    ui->setupUi(this);
    ui->verticalLayout->setContentsMargins(0,0,0,0);
    DesktopContainer=ui->DesktopContainer;
    DesktopComponent=ui->DesktopContainer->Desktop;
    DesktopComponent->init(parent);
    ui->verticalLayout_2->setContentsMargins(0,0,0,0);
    ui->li->setAttribute(Qt::WA_MacShowFocusRect,false);
    ui->widget->hide();

    connect(ui->li,qOverload<int>(&QComboBox::currentIndexChanged),this,&CMacroBoxForm::ChangeProgram);

    connect(DesktopComponent,&CDesktopComponent::parametersChanged,this,&CMacroBoxForm::PlugInIndexChanged);

    fillList();
    m_TimerID=startTimer(0);
}

CMacroBoxForm::~CMacroBoxForm()
{
    QMutexLocker locker(&mutex);
    qDebug() << "~CMacroBoxForm";
    killTimer(m_TimerID);
    m_TimerID=0;
    delete ui;
}

void CMacroBoxForm::unserializeCustom(const QDomLiteElement* xml)
{
    QMutexLocker locker(&mutex);
    DesktopComponent->unserialize(xml);
    fillList(m_Device->currentProgram());
}

void CMacroBoxForm::serializeCustom(QDomLiteElement* xml) const
{
    DesktopComponent->serialize(xml);
}

void CMacroBoxForm::fillList(int CurrentProgram)
{
    ui->li->blockSignals(true);
    ui->li->clear();
    ui->li->addItems(m_Device->programNames());
    ui->widget->setVisible(ui->li->count() > 1);
    if (CurrentProgram > -1)
    {
        ui->li->setCurrentIndex(CurrentProgram);
    }
    ui->li->blockSignals(false);
}

void CMacroBoxForm::PlugInIndexChanged()
{
    fillList(m_Device->currentProgram());
    //setVisible(true);
    m_Device->updateHostParameter();
}

void CMacroBoxForm::ChangeProgram(int programIndex)
{
    m_Device->setCurrentProgram(programIndex);
    m_Device->updateHostParameter();
}

void CMacroBoxForm::setProgram(const int programIndex)
{
    ChangeProgram(programIndex);
    if (ui->li->count() > programIndex)
    {
        ui->li->blockSignals(true);
        ui->li->setCurrentIndex(programIndex);
        ui->li->blockSignals(false);
        m_Device->updateHostParameter();
    }
}

void CMacroBoxForm::cascadeUIs()
{
    QPoint p(24,24);
    DesktopContainer->cascadeUIs(p);
}

bool CMacroBoxForm::event(QEvent *event)
{
    if (event->type()==QEvent::NonClientAreaMouseButtonPress)
    {
        if (dynamic_cast<QMouseEvent*>(event)->button()==Qt::RightButton)
        {
            //actionPasteParameters->setEnabled(QApplication::clipboard()->text().startsWith("<Parameters"));
            //parametersMenu->popup(mapToGlobal(dynamic_cast<QMouseEvent*>(event)->pos()));
            CParametersMenu* m = new CParametersMenu(m_Device,this,false);
            m->setAttribute(Qt::WA_DeleteOnClose,true);
            connect(m,&CParametersMenu::aboutToChange,ui->DesktopContainer->Desktop->MainMenu->UndoMenu,&CUndoMenu::addItem,Qt::DirectConnection);
            connect(m,&CParametersMenu::parametersChanged,this,&CMacroBoxForm::PlugInIndexChanged);
            m->addSeparator();
            m->addAction("UI map",ui->DesktopContainer,&CDesktopContainer::showMap);
            m->addAction("Hide UIs",ui->DesktopContainer,&CDesktopContainer::hideUIs);
            m->addAction("Cascade UIs",this,&CMacroBoxForm::cascadeUIs);

            m->popup(mapToGlobal(dynamic_cast<QMouseEvent*>(event)->pos()));
        }
    }
    return CSoftSynthsForm::event(event);
}

void CMacroBoxForm::timerEvent(QTimerEvent */*event*/)
{
    if (!m_TimerID) return;
    if (ui->li->count() != m_Device->programNames().size())
    {
        fillList(m_Device->currentProgram());
    }
    else if (ui->li->count())
    {
        const int p=m_Device->currentProgram();
        if (p != ui->li->currentIndex())
        {
            ui->li->blockSignals(true);
            ui->li->setCurrentIndex(p);
            ui->li->blockSignals(false);
        }
    }
}
