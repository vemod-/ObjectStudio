#ifndef CPITCHTEXTCONVERT_H
#define CPITCHTEXTCONVERT_H

#include <QStringList>
#include <QDomLite>

class CPitchTextConvert
{
public:
    CPitchTextConvert() { Pitch = 0; }
    CPitchTextConvert(int p) { Pitch = p; }
    CPitchTextConvert(const QString& txt) { Pitch = text2Pitch(txt); }
    int pitch() { return Pitch; }
    QString text() { return pitch2Text(Pitch); }
    static int text2Pitch(const QString& Txt)
    {
        if (Txt.isEmpty() || Txt.contains("?")) return 0;
        bool ok;
        int test = Txt.toInt(&ok);
        if (ok) return test;
        int LastChar = 0;
        QString Oct;
        for (int i=0;i<Txt.length();i++)
        {
            if (Txt[i].isDigit())
                Oct += Txt[i];
            else
                LastChar = i + 1;
        }
        return Note2Pitch(Txt.left(LastChar)) + ((Oct.toInt() + 1) * 12);
    }
    static QString pitch2Text(const int Pitch, const int Key = 0)
    {
        if (!Pitch) return "?";
        //QString Text;
        //int Octave= Pitch/12;
        const QString Text=Pitch2Note(Pitch, Key);
        return (Text.isEmpty()) ? QString("?") : Text + pitch2TextOctave(Pitch);
    }
    static QString pitch2TextStep(const int Pitch, const int Key = 0)
    {
        if (!Pitch) return "?";
        const QString Text = Pitch2Note(Pitch, Key);
        return (Text.isEmpty()) ? QString("?") : Text;
    }
    static QString pitch2TextOctave(const int Pitch)
    {
        if (!Pitch) return "?";
        const int Octave = Pitch / 12;
        const QString Text = QString::number(Octave - 1);
        return (Text.isEmpty()) ? QString("?") : Text;
    }
    static const QStringList naturalpitches;
    static const QStringList sharppitches;
    static const QStringList sharppitchesx;
    static const QStringList flatpitches;
    static const QStringList flatpitchesx;
    static const QStringList notenames;
private:
    int Pitch;
    static QString Pitch2Note(const int Pitch, const int Key = 0)
    {
        if (Key < -5) return flatpitchesx[Pitch % 12];
        if (Key < 0) return flatpitches[Pitch % 12];
        if (Key > 5) return sharppitchesx[Pitch % 12];
        if (Key > 0) return sharppitches[Pitch % 12];
        return naturalpitches[Pitch % 12];
    }
    static int Note2Pitch(const QString& Note)
    {
        return notenames.indexOf(Note.toUpper().replace("H","B")) % 12;
    }
};

#endif // CPITCHTEXTCONVERT_H
