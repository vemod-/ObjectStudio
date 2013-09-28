#ifndef CSF2GENERATOR_H
#define CSF2GENERATOR_H

#include "csf2file.h"

class SingleSF2Map : public QMap<QString, CSF2File*>
{
    public:
        static SingleSF2Map* getInstance()
        {
            static SingleSF2Map    instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
            return &instance;
        }
    private:
        SingleSF2Map() {}                   // Constructor? (the {} brackets) are needed here.
        SingleSF2Map(SingleSF2Map const&);              // Don't Implement
        void operator=(SingleSF2Map const&); // Don't implement
};

class CSF2Generator
{

private:
    bool PlayEnd;
    bool Ready;
    std::vector<OscType*> Osc;
    unsigned short OscCount;
    CSF2File* SFFile;
    float* AudioL;
    float* AudioR;
    unsigned int m_ModulationRate;
    int pitchWheel;
    int transpose;
    void Unref();
public:
    CSF2Generator();
    ~CSF2Generator();
    bool FinishedPlaying;
    short MidiBank;
    short MidiPreset;
    short ID;
    short Channel;
    bool LoadFile(const QString& Path);
    void ResetSample(const short MidiNote, const short MidiVelo);
    void EndSample(void);
    float* GetNext(void);
    short PresetCount(void);
    void setPitchWheel(int cent);
    void addTranspose(int steps);
    void setAftertouch(short value);
    void resetTranspose();
    SFPRESETHDRPTR Preset(short Index);
    SingleSF2Map* SF2Files;
};


#endif // CSF2GENERATOR_H
