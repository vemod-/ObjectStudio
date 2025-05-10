#include "cdrummachineform.h"
#include "ui_cdrummachineform.h"
#include "cinsertpatternform.h"
#include "crepeatform.h"
#include "cdrummachine.h"

CDrumMachineForm::CDrumMachineForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CDrumMachineForm)
{
    ui->setupUi(this);
    ui->InstrList->setDrawBase(false);
    ui->scrollAreaWidgetContents->setAutoFillBackground(false);
    PlayListMenu=new QMenu(this);
    PlayListMenu->addAction("Add...",this,SLOT(MenuAddPatternClick()));
    PlayListMenu->addAction("Remove",this,SLOT(MenuRemovePatternClick()));
    PlayListMenu->addAction("Edit...",this,SLOT(MenuEditPatternClick()));

    connect(ui->AddPattern,&QAbstractButton::clicked,this,&CDrumMachineForm::AddPatternClick);
    connect(ui->RemovePattern,&QAbstractButton::clicked,this,&CDrumMachineForm::RemovePatternClick);

    connect(ui->NameEdit,&QLineEdit::textEdited,this,&CDrumMachineForm::ChangeName);
    connect(ui->TempoSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CDrumMachineForm::ChangeTempo);
    connect(ui->BeatsSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CDrumMachineForm::ChangeNumOfBeats);

    connect(ui->PatternList,&QListWidget::currentRowChanged,this,&CDrumMachineForm::ChangePatternIndex);
    connect(ui->InstrList,&QTabBar::currentChanged,this,&CDrumMachineForm::ChangeInstrument);
    connect(ui->PatternPlayList,&QWidget::customContextMenuRequested,this,&CDrumMachineForm::PlayListPopup);
    connect(ui->PatternPlayList,&QListWidget::currentRowChanged,this,&CDrumMachineForm::ChangeListIndex);

    UpdateSounds();
    UpdatePatterns();
    UpdateBeats();
    UpdatePatternlist();
}

CDrumMachineForm::~CDrumMachineForm()
{
    delete ui;
}

void CDrumMachineForm::UpdateSounds()
{
    ui->InstrList->blockSignals(true);
    while (ui->InstrList->count()) ui->InstrList->removeTab(ui->InstrList->count()-1);
    for (const CWaveGeneratorX& w : DRUMMACHINECLASS->WG) ui->InstrList->addTab(w.Name);
    ui->InstrList->blockSignals(false);
}

void CDrumMachineForm::UpdatePatterns()
{
    ui->PatternList->blockSignals(true);
    ui->PatternList->clear();
    ui->RemovePattern->setEnabled(DRUMMACHINECLASS->Patterns.size()>1);
    for (const PatternType* p : std::as_const(DRUMMACHINECLASS->Patterns)) ui->PatternList->addItem(p->Name);
    ui->PatternList->blockSignals(false);
}

void CDrumMachineForm::UpdateBeats()
{
    const int soundIndex=ui->InstrList->currentIndex();
    if (ui->PatternList->currentRow()<0)
    {
        if (ui->PatternList->count()==0) return;
        ui->PatternList->blockSignals(true);
        ui->InstrList->blockSignals(true);
        ui->PatternList->setCurrentRow(0);
        ui->InstrList->setCurrentIndex(0);
        ui->PatternList->blockSignals(false);
        ui->InstrList->blockSignals(false);
    }
    ui->BeatsSpin->blockSignals(true);
    ui->TempoSpin->blockSignals(true);
    ui->NameEdit->blockSignals(true);
    const PatternType* CP=DRUMMACHINECLASS->Patterns.at(ui->PatternList->currentRow());
    for (CBeatFrame* b : std::as_const(m_Beats)) b->hide();
    ui->BeatsSpin->setValue(CP->numOfBeats());
    ui->NameEdit->setText(CP->Name);
    ui->TempoSpin->setValue(CP->Tempo);
    for (int i=0;i<CP->numOfBeats();i++)
    {
        CBeatFrame* Beat;
        if (i>=m_Beats.size())
        {
            Beat=new CBeatFrame(this);
            Beat->hide();
            m_Beats.append(Beat);
            ui->BeatFrameLayout->addWidget(Beat);
        }
        Beat=m_Beats.at(i);
        Beat->Init(CP->beat(i),i,soundIndex,false,false,true);
        Beat->show();
    }
    ui->BeatsSpin->blockSignals(false);
    ui->TempoSpin->blockSignals(false);
    ui->NameEdit->blockSignals(false);
}

void CDrumMachineForm::UpdatePatternlist()
{
    ui->PatternPlayList->blockSignals(true);
    ui->PatternPlayList->clear();
    for (int i=0;i<DRUMMACHINECLASS->PatternsInList.size();i++)
        ui->PatternPlayList->addItem(DRUMMACHINECLASS->PatternsInList[i]->caption());
    ui->PatternPlayList->blockSignals(false);
}

void CDrumMachineForm::RemovePatternInList(int Index)
{
    if (!DRUMMACHINECLASS->PatternsInList.isEmpty())
    {
        delete DRUMMACHINECLASS->PatternsInList.takeAt(Index);
        UpdatePatternlist();
    }
}

void CDrumMachineForm::AddPatternToList(int NewIndex, int PatternIndex, int Repeats)
{
    if (NewIndex>DRUMMACHINECLASS->PatternsInList.size()) NewIndex=DRUMMACHINECLASS->PatternsInList.size();
    DRUMMACHINECLASS->PatternsInList.insert(NewIndex,new PatternListType(DRUMMACHINECLASS->Patterns[PatternIndex],Repeats));
    UpdatePatternlist();
}

void CDrumMachineForm::AddPatternClick()
{
    DRUMMACHINECLASS->Patterns.append(new PatternType("New Pattern",16,DrumMachine::SoundCount,100,0,0));
    UpdatePatterns();
    ui->PatternList->setCurrentRow(ui->PatternList->count()-1);
}
//---------------------------------------------------------------------------

void CDrumMachineForm::RemovePatternClick()
{
    if (ui->PatternList->count()>0)
    {
        int Curr=ui->PatternList->currentRow();
        RemovePattern(Curr);
        UpdatePatterns();
        if (Curr>ui->PatternList->count()-1)
            ui->PatternList->setCurrentRow(ui->PatternList->count()-1);
        else
            ui->PatternList->setCurrentRow(Curr);
    }
}
//---------------------------------------------------------------------------

void CDrumMachineForm::RemovePattern(int Index)
{
    for (int i=0;i<DRUMMACHINECLASS->PatternsInList.size();i++)
        if (DRUMMACHINECLASS->Patterns.indexOf(DRUMMACHINECLASS->PatternsInList[i]->Pattern)==Index) RemovePatternInList(i);
    delete DRUMMACHINECLASS->Patterns.takeAt(Index);
}

void CDrumMachineForm::PlayListPopup(QPoint Pos)
{
    PlayListMenu->popup(ui->PatternPlayList->mapToGlobal(Pos));
}

void CDrumMachineForm::MenuAddPatternClick()
{
    int PIndex=0;
    if (ui->PatternPlayList->currentRow()>-1) PIndex=ui->PatternPlayList->currentRow();
    int NewPattern=0;
    int Repeats=4;
    bool InsertBefore=true;
    CInsertPatternForm d(this);
    d.SelectPattern(DRUMMACHINECLASS->Patterns,NewPattern,Repeats,InsertBefore);
    if (d.exec() == QDialog::Accepted)
    {
        d.GetValues(NewPattern,Repeats,InsertBefore);
        if (!InsertBefore) PIndex++;
        AddPatternToList(PIndex,NewPattern,Repeats);
    }
}
//---------------------------------------------------------------------------

void CDrumMachineForm::MenuRemovePatternClick()
{
    if (ui->PatternPlayList->currentRow()>-1)
    {
        RemovePatternInList(ui->PatternPlayList->currentRow());
    }
}
//---------------------------------------------------------------------------

void CDrumMachineForm::MenuEditPatternClick()
{
    if (ui->PatternPlayList->currentRow()>-1)
    {
        PatternListType* PL=DRUMMACHINECLASS->PatternsInList[ui->PatternPlayList->currentRow()];
        CRepeatForm d(this);
        d.SetRepeats(PL->Repeats);
        if (d.exec()==QDialog::Accepted)
        {
            PL->Repeats=d.GetRepeats();
            UpdatePatternlist();
        }
    }
}

void CDrumMachineForm::ChangePatternIndex()
{
    UpdateBeats();
}
//---------------------------------------------------------------------------
void CDrumMachineForm::ChangeName()
{
    const int r = ui->PatternList->currentRow();
    DRUMMACHINECLASS->Patterns[r]->Name=ui->NameEdit->text();
    UpdatePatterns();
    ui->PatternList->setCurrentRow(r);
}

void CDrumMachineForm::ChangeInstrument()
{
    UpdateBeats();
}
//---------------------------------------------------------------------------


void CDrumMachineForm::ChangeNumOfBeats(int Value)
{
    DRUMMACHINECLASS->Patterns[ui->PatternList->currentRow()]->setNumOfBeats(Value,100,0,0);
    UpdateBeats();
}
//---------------------------------------------------------------------------

void CDrumMachineForm::ChangeTempo(int Value)
{
    DRUMMACHINECLASS->Patterns[ui->PatternList->currentRow()]->Tempo=Value;
}

void CDrumMachineForm::unserializeCustom(const QDomLiteElement* xml)
{
    while (!DRUMMACHINECLASS->Patterns.empty()) RemovePattern(DRUMMACHINECLASS->Patterns.size()-1);
    //DRUMMACHINECLASS->Patterns.clear();
    DRUMMACHINECLASS->PatternsInList.clear();
    if (!xml) return;
    for (const QDomLiteElement* Pattern : (const QDomLiteElementList)xml->elementsByTag("Pattern"))
    {
        PatternType* P=new PatternType(Pattern->attribute("Name"),Pattern->attributeValueInt("NumOfBeats"),Pattern->attributeValueInt("Sounds"));
        DRUMMACHINECLASS->Patterns.append(P);
        P->Tempo = Pattern->attributeValueInt("Tempo");
        const QDomLiteElementList XMLBeats = Pattern->elementsByTag("Beat");
        for (int i=0;i<XMLBeats.size();i++)
        {
            const QDomLiteElement* Beat=XMLBeats[i];
            if (i < P->numOfBeats())
            {
                BeatType* B = P->beat(i);
                for (int i1=0;i1<P->polyphony();i1++)
                {
                    B->Pitch[i1]=Beat->attributeValueInt("Pitch"+QString::number(i1));
                    B->Length[i1]=Beat->attributeValueInt("Length"+QString::number(i1));
                    B->Volume[i1]=Beat->attributeValueInt("Volume"+QString::number(i1));
                }
            }
        }
    }
    for (const QDomLiteElement* PatternInList : (const QDomLiteElementList)xml->elementsByTag("PatternInList"))
    {
        int Index = PatternInList->attributeValueInt("PatternIndex");
        int Repeats = PatternInList->attributeValueInt("Repeats");
        DRUMMACHINECLASS->PatternsInList.append(new PatternListType(DRUMMACHINECLASS->Patterns[Index],Repeats));
    }
    UpdatePatterns();
    UpdateBeats();
    UpdatePatternlist();
}

void CDrumMachineForm::serializeCustom(QDomLiteElement* xml) const
{
    for (int i=0; i<DRUMMACHINECLASS->Patterns.size(); i++)
    {
        const PatternType* P=DRUMMACHINECLASS->Patterns[i];
        QDomLiteElement* Pattern = xml->appendChild("Pattern");
        Pattern->setAttribute("Name",P->Name);
        Pattern->setAttribute("NumOfBeats",P->numOfBeats());
        Pattern->setAttribute("Tempo",P->Tempo);
        Pattern->setAttribute("Sounds",P->polyphony());
        for (int i1=0;i1<P->numOfBeats();i1++)
        {
            const BeatType* B=P->beat(i1);
            QDomLiteElement* Beat = Pattern->appendChild("Beat");
            for (int i2=0;i2<P->polyphony();i2++)
            {
                Beat->setAttribute("Pitch"+QString::number(i2),B->Pitch[i2]);
                Beat->setAttribute("Length"+QString::number(i2),B->Length[i2]);
                Beat->setAttribute("Volume"+QString::number(i2),B->Volume[i2]);
            }
        }
    }

    for (int i=0; i<DRUMMACHINECLASS->PatternsInList.size(); i++)
    {
        const PatternListType* PLI=DRUMMACHINECLASS->PatternsInList[i];
        int Index=DRUMMACHINECLASS->Patterns.indexOf(PLI->Pattern);
        QDomLiteElement* PatternInList = xml->appendChild("PatternInList");
        PatternInList->setAttribute("PatternIndex",Index);
        PatternInList->setAttribute("Repeats",PLI->Repeats);
    }
}

void CDrumMachineForm::Flash(int Pattern, int Beat)
{
    if (Pattern != ui->PatternPlayList->currentRow()) ui->PatternPlayList->setCurrentRow(Pattern);
    m_Beats[Beat]->Flash();
}

void CDrumMachineForm::ChangeListIndex(int index)
{
    QString PatternName=DRUMMACHINECLASS->PatternsInList[index]->Pattern->Name;
    for (int i=0;i<ui->PatternList->count();i++)
    {
        if (ui->PatternList->item(i)->text()==PatternName)
        {
            ui->PatternList->setCurrentRow(i);
            break;
        }
    }
}
