#include "csequenserform.h"
#include "ui_csequenserform.h"
#include "cinsertpatternform.h"
#include "crepeatform.h"
#include "csequenser.h"

CSequenserForm::CSequenserForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CSequenserForm)
{
    ui->setupUi(this);
    PlayListMenu=new QMenu(this);
    PlayListMenu->addAction("Add...",this,SLOT(MenuAddPatternClick()));
    PlayListMenu->addAction("Remove",this,SLOT(MenuRemovePatternClick()));
    PlayListMenu->addAction("Edit...",this,SLOT(MenuEditPatternClick()));

    connect(ui->AddPattern,&QAbstractButton::clicked,this,&CSequenserForm::AddPatternClick);
    connect(ui->RemovePattern,&QAbstractButton::clicked,this,&CSequenserForm::RemovePatternClick);

    connect(ui->NameEdit,&QLineEdit::textEdited,this,&CSequenserForm::ChangeName);
    connect(ui->TempoSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CSequenserForm::ChangeTempo);
    connect(ui->BeatsSpin,qOverload<int>(&QSpinBox::valueChanged),this,&CSequenserForm::ChangeNumOfBeats);

    connect(ui->PatternList,&QListWidget::currentRowChanged,this,&CSequenserForm::ChangePatternIndex);
    connect(ui->PatternPlayList,&QWidget::customContextMenuRequested,this,&CSequenserForm::PlayListPopup);
    connect(ui->PatternPlayList,&QListWidget::currentRowChanged,this,&CSequenserForm::ChangeListIndex);

    UpdatePatterns();
    UpdateBeats();
    UpdatePatternlist();
}

CSequenserForm::~CSequenserForm()
{
    delete ui;
}

void CSequenserForm::UpdatePatterns()
{
    ui->PatternList->blockSignals(true);
    ui->PatternList->clear();
    ui->RemovePattern->setEnabled(SEQUENSERCLASS->Patterns.size()>1);
    for (const PatternType* p : std::as_const(SEQUENSERCLASS->Patterns)) ui->PatternList->addItem(p->Name);
    ui->PatternList->blockSignals(false);
}

void CSequenserForm::UpdateBeats()
{
    if (ui->PatternList->currentRow()<0)
    {
        if (ui->PatternList->count()==0) return;
        ui->PatternList->blockSignals(true);
        ui->PatternList->setCurrentRow(0);
        ui->PatternList->blockSignals(false);
    }
    ui->BeatsSpin->blockSignals(true);
    ui->TempoSpin->blockSignals(true);
    ui->NameEdit->blockSignals(true);
    CBeatFrame* Beat;
    const PatternType* CP=SEQUENSERCLASS->Patterns[ui->PatternList->currentRow()];
    for (CBeatFrame* b : std::as_const(m_Beats))//(int i=0;i<m_Beats.size();i++)
    {
        b->hide();
    }
    ui->BeatsSpin->setValue(CP->numOfBeats());
    ui->NameEdit->setText(CP->Name);
    ui->TempoSpin->setValue(CP->Tempo);
    for (int i=0;i<CP->numOfBeats();i++)
    {
        if (i>=m_Beats.size())
        {
            Beat=new CBeatFrame(this);
            Beat->hide();
            m_Beats.append(Beat);
            ui->BeatFrameLayout->addWidget(Beat);
        }
        Beat=m_Beats[i];
        Beat->Init(CP->beat(i),i,0,false,false,false);
        Beat->show();
    }
    ui->BeatsSpin->blockSignals(false);
    ui->TempoSpin->blockSignals(false);
    ui->NameEdit->blockSignals(false);
}

void CSequenserForm::UpdatePatternlist()
{
    ui->PatternPlayList->blockSignals(true);
    ui->PatternPlayList->clear();
    for (const PatternListType* p : std::as_const(SEQUENSERCLASS->PatternsInList)) ui->PatternPlayList->addItem(p->caption());
    ui->PatternPlayList->blockSignals(false);
}

void CSequenserForm::RemovePatternInList(int Index)
{
    if (!SEQUENSERCLASS->PatternsInList.isEmpty())
    {
        delete SEQUENSERCLASS->PatternsInList.takeAt(Index);
        UpdatePatternlist();
    }
}

void CSequenserForm::AddPatternToList(int NewIndex, int PatternIndex, int Repeats)
{
    if (NewIndex>SEQUENSERCLASS->PatternsInList.size()) NewIndex=SEQUENSERCLASS->PatternsInList.size();
    SEQUENSERCLASS->PatternsInList.insert(NewIndex,new PatternListType(SEQUENSERCLASS->Patterns[PatternIndex],Repeats));
    UpdatePatternlist();
}

void CSequenserForm::AddPatternClick()
{
    SEQUENSERCLASS->Patterns.append(new PatternType("New Pattern",16,1,100,0,0));
    UpdatePatterns();
    ui->PatternList->setCurrentRow(ui->PatternList->count()-1);
}
//---------------------------------------------------------------------------

void CSequenserForm::RemovePatternClick()
{
    if (ui->PatternList->count()>0)
    {
        int Curr=ui->PatternList->currentRow();
        RemovePattern(Curr);
        UpdatePatterns();
        if (Curr>ui->PatternList->count()-1)
        {
            ui->PatternList->setCurrentRow(ui->PatternList->count()-1);
        }
        else
        {
            ui->PatternList->setCurrentRow(Curr);
        }
    }
}
//---------------------------------------------------------------------------

void CSequenserForm::RemovePattern(int Index)
{
    for (int i=0;i<SEQUENSERCLASS->PatternsInList.size();i++)
    {
        const PatternListType* PLI=SEQUENSERCLASS->PatternsInList[i];
        PatternType* CP=PLI->Pattern;
        if (SEQUENSERCLASS->Patterns.indexOf(CP)==Index) RemovePatternInList(i);
    }
    delete SEQUENSERCLASS->Patterns.takeAt(Index);
}

void CSequenserForm::PlayListPopup(QPoint Pos)
{
    PlayListMenu->popup(ui->PatternPlayList->mapToGlobal(Pos));
}

void CSequenserForm::MenuAddPatternClick()
{
    int PIndex=0;
    if (ui->PatternPlayList->currentRow()>-1)
    {
        PIndex=ui->PatternPlayList->currentRow();
    }
    int NewPattern=0;
    int Repeats=4;
    bool InsertBefore=true;
    CInsertPatternForm d(this);
    d.SelectPattern(SEQUENSERCLASS->Patterns,NewPattern,Repeats,InsertBefore);
    if (d.exec() == QDialog::Accepted)
    {
        d.GetValues(NewPattern,Repeats,InsertBefore);
        if (!InsertBefore) PIndex++;
        AddPatternToList(PIndex,NewPattern,Repeats);
    }
}
//---------------------------------------------------------------------------

void CSequenserForm::MenuRemovePatternClick()
{
    if (ui->PatternPlayList->currentRow()>-1)
    {
        RemovePatternInList(ui->PatternPlayList->currentRow());
    }
}
//---------------------------------------------------------------------------

void CSequenserForm::MenuEditPatternClick()
{
    if (ui->PatternPlayList->currentRow()>-1)
    {
        PatternListType* PL=SEQUENSERCLASS->PatternsInList[ui->PatternPlayList->currentRow()];
        CRepeatForm d(this);
        d.SetRepeats(PL->Repeats);
        if (d.exec()==QDialog::Accepted)
        {
            PL->Repeats=d.GetRepeats();
            UpdatePatternlist();
        }
    }
}

void CSequenserForm::ChangePatternIndex()
{
    UpdateBeats();
}
//---------------------------------------------------------------------------

void CSequenserForm::ChangeName()
{
    const int r = ui->PatternList->currentRow();
    SEQUENSERCLASS->Patterns[r]->Name=ui->NameEdit->text();
    UpdatePatterns();
    ui->PatternList->setCurrentRow(r);
}

void CSequenserForm::ChangeNumOfBeats(int Value)
{
    SEQUENSERCLASS->Patterns[ui->PatternList->currentRow()]->setNumOfBeats(Value,100,0,0);
    UpdateBeats();
}
//---------------------------------------------------------------------------

void CSequenserForm::ChangeTempo(int Value)
{
    SEQUENSERCLASS->Patterns[ui->PatternList->currentRow()]->Tempo=Value;
}

void CSequenserForm::unserializeCustom(const QDomLiteElement* xml)
{
    while (!SEQUENSERCLASS->Patterns.empty()) RemovePattern(SEQUENSERCLASS->Patterns.size()-1);
    //SEQUENSERCLASS->Patterns.clear();
    SEQUENSERCLASS->PatternsInList.clear();
    if (!xml) return;
    for (const QDomLiteElement* Pattern : (const QDomLiteElementList)xml->elementsByTag("Pattern"))
    {
        int NOB = Pattern->attributeValueInt("NumOfBeats");
        const QString Name = Pattern->attribute("Name");
        auto P=new PatternType(Name,NOB);
        SEQUENSERCLASS->Patterns.append(P);
        P->Tempo = Pattern->attributeValueInt("Tempo");
        const QDomLiteElementList XMLBeats = Pattern->elementsByTag("Beat");
        for (int i=0;i<XMLBeats.size();i++)
        {
            const QDomLiteElement* Beat=XMLBeats[i];
            if (i<NOB)
            {
                BeatType* B = P->beat(i);
                B->Pitch[0]=Beat->attributeValueInt("Pitch");
                B->Length[0]=Beat->attributeValueInt("Length");
                B->Volume[0]=Beat->attributeValueInt("Volume");
            }
        }
    }
    for (const QDomLiteElement* PatternInList : (const QDomLiteElementList)xml->elementsByTag("PatternInList"))
    {
        const int Index = PatternInList->attributeValueInt("PatternIndex");
        const int Repeats = PatternInList->attributeValueInt("Repeats");
        SEQUENSERCLASS->PatternsInList.append(new PatternListType(SEQUENSERCLASS->Patterns[Index],Repeats));
    }
    UpdatePatterns();
    UpdateBeats();
    UpdatePatternlist();
}

void CSequenserForm::serializeCustom(QDomLiteElement* xml) const
{
    for (int i=0; i<SEQUENSERCLASS->Patterns.size(); i++)
    {
        const PatternType* P=SEQUENSERCLASS->Patterns[i];
        QDomLiteElement* Pattern = xml->appendChild("Pattern");
        Pattern->setAttribute("Name",P->Name);
        Pattern->setAttribute("NumOfBeats",P->numOfBeats());
        Pattern->setAttribute("Tempo",P->Tempo);
        for (int i1=0;i1<P->numOfBeats();i1++)
        {
            const BeatType* B=P->beat(i1);
            QDomLiteElement* Beat = Pattern->appendChild("Beat");
            Beat->setAttribute("Pitch",B->Pitch[0]);
            Beat->setAttribute("Length",B->Length[0]);
            Beat->setAttribute("Volume",B->Volume[0]);
        }
    }

    for (int i=0; i<SEQUENSERCLASS->PatternsInList.size(); i++)
    {
        const PatternListType* PLI=SEQUENSERCLASS->PatternsInList[i];
        const int Index=SEQUENSERCLASS->Patterns.indexOf(PLI->Pattern);
        QDomLiteElement* PatternInList = xml->appendChild("PatternInList");
        PatternInList->setAttribute("PatternIndex",Index);
        PatternInList->setAttribute("Repeats",PLI->Repeats);
    }
}

void CSequenserForm::Flash(int Pattern, int Beat)
{
    if (Pattern != ui->PatternPlayList->currentRow()) ui->PatternPlayList->setCurrentRow(Pattern);
    m_Beats[Beat]->Flash();
}

void CSequenserForm::ChangeListIndex(int index)
{
    QString PatternName=SEQUENSERCLASS->PatternsInList[index]->Pattern->Name;
    for (int i=0;i<ui->PatternList->count();i++)
    {
        if (ui->PatternList->item(i)->text()==PatternName)
        {
            ui->PatternList->setCurrentRow(i);
            break;
        }
    }
}
