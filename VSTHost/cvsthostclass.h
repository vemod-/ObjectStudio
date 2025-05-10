#ifndef CVSTHOSTCLASS_H
#define CVSTHOSTCLASS_H

//#include "audioeffectx.h"
#include "aeffectx.h"
#include "iaudiopluginhost.h"

typedef byte byte;

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
    byte future[128]; //     : array[0..127] of byte;

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
    byte future[128]; //     : array[0..127] of byte;

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

class CVSTHostClass : public IAudioPlugInHost
{
    Q_OBJECT
private:
    AEffect* ptrPlug;
    QList<VstEvent*> vstMidiEvents;
    QList<std::vector<byte>> vstSysExData;
    QVector<char> vstEventsBuffer;
    int GetChunk(void* pntr,bool isPreset) const;
    int SetChunk(void* data,long byteSize,bool isPreset);
    void savePreset(QFile& str) const;
    void loadBank(QFile& str);
    fxPreset GetPreset(long i) const;
    void loadPreset(QFile& str);
    void saveBank(QFile& str) const;
    void* vstBundle;
    float VSTVersion();
    void LoadProgramNames();
    //int m_TimerID;
    void KillPlug();
public:
    CVSTHostClass(QWidget* parent=nullptr);
    ~CVSTHostClass();
    void parseEvent(const CMIDIEvent* Event);
    int inputCount();
    int outputCount();
    bool process();
    QString CurrentBank;
    QString CurrentPreset;
    void loadBank(QString FileName);
    void loadPreset(QString FileName);
    void saveBank(QString FileName);
    void savePreset(QString FileName);
    const QString bankPresetName() const;
    const QStringList bankPresetNames() const;
    void setBankPreset(const long index);
    long currentBankPreset(const int channel=-1) const;
    const QStringList VSTCategories();
    const QStringList VSTFiles(QString category);
    float parameter(const long index) const;
    void setParameter(const long index, const float value);
    int parameterCount() const;
    const QString parameterName(const long index);
    const QString parameterValue(const long index);
    void serialize(QDomLiteElement* xml) const;
    void unserialize(const QDomLiteElement* xml);
    bool loadFile(const QString& filename);
    const QString filename() const {
        if (fileParameter) return fileParameter->filename();
        return QString();
    }
    const QSize UISize() const;
    CFileParameter* fileParameter;
    static VstIntPtr VSTCALLBACK host(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
    static char returnString[1024];
    static VstTimeInfo returnInfo;
    static char** FileStrings;
    static int nFileStrings;
public slots:
    void LoadFromMenu(QString filename);
    void popup(QPoint pos);
protected:
    //void timerEvent(QTimerEvent *);
};

#endif // CVSTHOSTCLASS_H
