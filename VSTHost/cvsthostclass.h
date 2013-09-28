#ifndef CVSTHOSTCLASS_H
#define CVSTHOSTCLASS_H

#include "aeffectx.h"
#include "IAudioPlugInHost.h"

typedef unsigned char BYTE;

#pragma pack(push, 1)
struct
        //--------------------------------------------------------------------
        // For Preset (Program) (.fxp) without chunk (magic = 'FxCk')
        //--------------------------------------------------------------------
        fxPreset// = packed record
{
    char chunkMagic[4];// : longint;   // 'CcnK'
    int byteSize;//   : longint;   // of this chunk, excl. magic + byteSize

    char fxMagic[4]; //   : longint;   // 'FxCk'
    int version; //   : longint;
    int fxID; //      : longint;   // fx unique id
    int fxVersion; // : longint;

    int numParams; //  : longint;
    char prgName[28];//    : array[0..27] of char;
    void* params;  //   : pointer; //array[0..0] of single;    // variable no. of parameters
};

//--------------------------------------------------------------------
// For Preset (Program) (.fxp) with chunk (magic = 'FPCh')
//--------------------------------------------------------------------
struct fxChunkSet // = packed record
{
    char chunkMagic[4]; // : longint;                // 'CcnK'
    int byteSize;  //  : longint;                // of this chunk, excl. magic + byteSize

    char fxMagic[4];  //   : longint;                // 'FPCh'
    int version;   //  : longint;
    int fxID;      //  : longint;                // fx unique id
    int fxVersion;  // : longint;

    int numPrograms;// : longint;
    char prgName[28];//     : array[0..27] of char;

    int chunkSize; //  : longint;
    void* chunk;    //   : pointer; //array[0..7] of char;    // variable
};

//--------------------------------------------------------------------
// For Bank (.fxb) without chunk (magic = 'FxBk')
//--------------------------------------------------------------------
struct fxSet //= packed record
{
    char chunkMagic[4]; // : longint;                   // 'CcnK'
    int byteSize;    //: longint;                   // of this chunk, excl. magic + byteSize

    char fxMagic[4];   //  : longint;                   // 'FxBk'
    int version;   //  : longint;
    int fxID;      //  : longint;                   // fx unique id
    int fxVersion; //  : longint;

    int numPrograms; //: longint;
    BYTE future[128]; //     : array[0..127] of byte;

    void* programs;  //  : pointer;//array[0..0] of fxPreset;  // variable no. of programs
};


//--------------------------------------------------------------------
// For Bank (.fxb) with chunk (magic = 'FBCh')
//--------------------------------------------------------------------
struct fxChunkBank //= packed record
{
    char chunkMagic[4]; // : longint;                // 'CcnK'
    int byteSize;  //  : longint;                // of this chunk, excl. magic + byteSize

    char fxMagic[4];   //  : longint;                // 'FBCh'
    int version;   //  : longint;
    int fxID;      //  : longint;                // fx unique id
    int fxVersion; //  : longint;

    int numPrograms; //: longint;
    BYTE future[128]; //     : array[0..127] of byte;

    int chunkSize; //  : longint;
    void* chunk;   //    : pointer; //array[0..7] of char;    // variable
};

struct VSTRect
{
    short top;
    short left;
    short bottom;
    short right;
};
#pragma pack(pop)

class TVSTHost : public IAudioPlugInHost
{
    Q_OBJECT
private:
    AEffect* ptrPlug;
    QList<VstMidiEvent> vstMidiEvents;
    std::vector<char> vstEventsBuffer;
    int GetChunk(void* pntr,bool isPreset);
    int SetChunk(void* data,long byteSize,bool isPreset);
    void SavePreset(QFile& str);
    void LoadBank(QFile& str);
    fxPreset GetPreset(long i);
    void LoadPreset(QFile& str);
    void SaveBank(QFile& str);
    void* vstBundle;
    float VSTVersion();
    void LoadProgramNames();
    QRect GetEffRect();
public:
    TVSTHost(unsigned int sampleRate, unsigned int bufferSize, QWidget* parent=0);
    ~TVSTHost();
    void KillPlug();
    const int NumInputs();
    const int NumOutputs();
    const bool Process();
    void DumpMIDI(CMIDIBuffer* MB, bool PatchChange);
    void AllNotesOff();
    QString CurrentBank;
    QString CurrentPreset;
    void LoadBank(QString FileName);
    void LoadPreset(QString FileName);
    void SaveBank(QString FileName);
    void SavePreset(QString FileName);
    const QString ProgramName();
    const QStringList ProgramNames();
    void SetProgram(const long index);
    const long CurrentProgram();
    const QStringList VSTCategories();
    const QStringList VSTFiles(QString category);
    const float GetParameter(const long index);
    void SetParameter(const long index, const float value);
    const int ParameterCount();
    const QString ParameterName(const long index);
    const QString ParameterValue(const long index);
    const QString SaveXML();
    void LoadXML(const QString& XML);
    const bool Load(QString Filename);
    static VstIntPtr VSTCALLBACK host(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
public slots:
    void LoadFromMenu(QString Filename);
    void Popup(QPoint pos);
protected:
    void timerEvent(QTimerEvent *);
};

#endif // CVSTHOSTCLASS_H
