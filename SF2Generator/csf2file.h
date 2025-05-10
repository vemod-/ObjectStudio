#ifndef CSF2FILE_H
#define CSF2FILE_H

#include "softsynthsdefines.h"
#include "enabler/enab.h"
#include <QtCore>
#include "csinglemap.h"
#include "idevicebase.h"

namespace SF2File
{
const QString SF2Filter("Sound font files (*.sf2)");
}

class CSF2File : public IRefCounter
{
public:
    class SF2Preset
    {
    public:
        SF2Preset():programNumber(0){}
        SF2Preset(const int p,char* n):programNumber(p),presetName(n){}
        void assign(const int p,char* n){
            programNumber=p;
            presetName=n;
        }
        int programNumber;
        QString presetName;
    };
    class SF2Bank
    {
    public:
        SF2Bank(){}
        QHash<int,SF2Preset> presets;
    };
    CSF2File() : IRefCounter() { sampleCount=0; }
    ~CSF2File()
    {
        QMutexLocker locker(&mutex);
        sampleCount=0;
        SF2Enabler.UnloadSFBank();
        banks.clear();
        path.clear();
    }
    bool load(const QString& filePath);
    QString path;
    CSF2Enabler SF2Enabler;
    inline short readSample(const ulong64 Pos) {
        return (Pos >= sampleCount) ? 0 : data[Pos];
    }
    QHash<int,SF2Bank> banks;
    std::vector<sfPresetHdr> programHeaders;
    ushort programCount;
    ulong64 sampleCount;
private:
    short* data;
    QRecursiveMutex mutex;
};



#endif // CSF2FILE_H
