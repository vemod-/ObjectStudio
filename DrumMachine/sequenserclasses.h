#ifndef SEQUENSERCLASSES_H
#define SEQUENSERCLASSES_H

#include <QtCore>
#include "softsynthsdefines.h"

class BeatType
{
public:
    BeatType(int Poly)
    {
        Length=new byte[Poly];
        Pitch=new byte[Poly];
        Volume=new byte[Poly];
        for (int i=0;i<Poly;i++)
        {
            Length[i]=100;
            Pitch[i]=0;
            Volume[i]=100;
        }
    }
    BeatType(byte Length,byte Pitch,byte Volume,int Poly)
    {
        this->Length=new byte[Poly];
        this->Pitch=new byte[Poly];
        this->Volume=new byte[Poly];
        for (int i=0;i<Poly;i++)
        {
            this->Length[i]=Length;
            this->Pitch[i]=Pitch;
            this->Volume[i]=Volume;
        }
    }
    ~BeatType()
    {
        delete [] Length;
        delete [] Pitch;
        delete [] Volume;
    }

    byte* Length;
    byte* Pitch;
    byte* Volume;
};

class PatternType
{
public:
    PatternType(const QString& sName,int NumOfBeats,int Poly=1)
    {
        Name=sName;
        Tempo=100;
        m_Polyphony=Poly;
        for (int i=0;i<NumOfBeats;i++) Beats.append(new BeatType(Poly));
    }
    PatternType(const QString& sName,int NumOfBeats,int Poly,byte Length,byte Pitch,byte Volume)
    {
        Name=sName;
        Tempo=100;
        m_Polyphony=Poly;
        for (int i=0;i<NumOfBeats;i++) Beats.append(new BeatType(Length,Pitch,Volume,Poly));
    }
    ~PatternType()
    {
        setNumOfBeats(0);
    }
    QString Name;
    int numOfBeats() const
    {
        return Beats.size();
    }
    int Tempo;
    BeatType* beat(int Index) const
    {
        return Beats.at(Index);
    }
    void setNumOfBeats(int NewNumOfBeats)
    {
        while (Beats.size() > NewNumOfBeats) delete Beats.takeLast();
        while (Beats.size() < NewNumOfBeats) Beats.append(new BeatType(m_Polyphony));
    }
    void setNumOfBeats(int NewNumOfBeats,byte Length,byte Pitch,byte Volume)
    {
        while (Beats.size() > NewNumOfBeats) delete Beats.takeLast();
        while (Beats.size() < NewNumOfBeats) Beats.append(new BeatType(Length,Pitch,Volume,m_Polyphony));
    }
    int polyphony() const { return m_Polyphony; }
private:
    QList<BeatType*> Beats;
    int m_Polyphony;
};

class PatternListType
{
public:
    PatternListType(PatternType* Pattern, int Repeats=4)
    {
        this->Pattern=Pattern;
        this->Repeats=Repeats;
    }
    QString caption() const
    {
        return (Repeats>0) ? QString(Pattern->Name + " " + QString::number(Repeats) + "x") : Pattern->Name + " Loop";
    }

    PatternType* Pattern;
    int Repeats;
};

#endif // SEQUENSERCLASSES_H
