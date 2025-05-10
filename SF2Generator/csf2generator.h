#ifndef CSF2GENERATOR_H
#define CSF2GENERATOR_H

#include "csfoscillator.h"
#include "isoundgenerator.h"
#include "caudiobuffer.h"
#include <QMutexLocker>

class CSF2Generator : public ISoundGenerator
{
public:
    CSF2Generator();
    ~CSF2Generator();
    short MidiBank;
    short MidiPreset;
    bool load(const QString& Path);
    void startNote(const short MidiNote, const short MidiVelo);
    void endNote();
    float* getNext();
    void setPitchWheel(const int cent);
    void addPortamento(const int steps);
    void setAftertouch(const int value);
    void resetPortamento();
    int banknumber(const int program) const;
    int presetnumber(const int program) const;
    const QString bankPresetName(const int program) const;
    const QString presetName(const int bank, const int preset) const;
    int bankPresetNumber(const int bank, const int preset) const;
    const QList<int> bankNumbers() const;
    const QList<int> presetNumbers(const int bank) const;
private:
    bool PlayEnd;
    bool Ready;
    std::vector<CSFOscillator> Osc;
    CSF2File* SFFile;
    CStereoBuffer Buffer;
    void Unref();
    QRecursiveMutex mutex;
};


#endif // CSF2GENERATOR_H
