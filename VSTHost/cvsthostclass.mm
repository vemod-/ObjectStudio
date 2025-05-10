#include "cvsthostclass.h"
//#import <Cocoa/Cocoa.h>
#include <QFileDialog>
#include "singlevstpluginlist.h"
#include "macstrings.h"
#include <QApplication>
#include "qsignalmenu.h"

char CVSTHostClass::returnString[1024];
VstTimeInfo CVSTHostClass::returnInfo;
char** CVSTHostClass::FileStrings=nullptr;
int CVSTHostClass::nFileStrings=0;

CVSTHostClass::CVSTHostClass(QWidget* parent)
    : IAudioPlugInHost(parent)
{
    ptrPlug=nullptr;
    mMainMenu=new QMenu(this);
    for (const QString& s : VSTCategories())
    {
        QSignalMenu* m=new QSignalMenu(s,mMainMenu);
        connect(m,SIGNAL(menuClicked(QString)),this,SLOT(LoadFromMenu(QString)));
        mMainMenu->addMenu(m);
        QStringList l=VSTFiles(s);
        for (const QString& s : std::as_const(l)) m->addAction(QFileInfo(s).baseName(),"VSTHost&&&&&&" + s);
    }
    //m_TimerID=startTimer(200);
}

void CVSTHostClass::popup(QPoint pos)
{
    for(QSignalMenu* m : (const QList<QSignalMenu*>)mMainMenu->findChildren<QSignalMenu*>()) m->checkAction("VSTHost&&&&&&" + filename());
    mMainMenu->popup(pos);
}

CVSTHostClass::~CVSTHostClass()
{
    qDebug() << "~VSTHostClass";
    //killTimer(m_TimerID);
    //m_TimerID=0;
    KillPlug();
    qDebug() << "Exit VSTHostClass";
}

const QStringList CVSTHostClass::VSTCategories()
{
    return SingleVSTPlugInList::categories();
}

const QStringList CVSTHostClass::VSTFiles(QString category)
{
    return SingleVSTPlugInList::files(category);
}

void CVSTHostClass::LoadFromMenu(QString Filename)
{
    QStringList l = Filename.split("&&&&&&");
    fileParameter->openFile(l[1]);
}

QString getPlugString(AEffect* eff,AEffectOpcodes OpCode,const long index)
{
    char s[256];
    zeroMemory(s,256);
    eff->dispatcher(eff,OpCode,index,0,s,0.0f);
    return qt_mac_MacRomanToQString(s).trimmed();
}

bool CVSTHostClass::loadFile(const QString& Filename)
{
    setVisible(false);
    //CFileIdentifier id(Filename);
    //if (id == m_Filename) return true;

    AEffect* TempPlug;
    //find and load the DLL and get a pointer to its main function
    //this has a protoype like this: AEffect *main (audioMasterCallback audioMaster)
#ifndef __x86_64
    if (!bundleIsI386(Filename))
    {
        qDebug() << "Plugin is not I386";
        return false;
    }
#else
    if (!bundleIsX86_64(Filename))
    {
        qDebug() << Filename << "Plugin is not X86_64";
        return false;
    }
#endif
    const CFBundleRef TempBundle= pathToCFBundleRef(Filename);

    if (TempBundle == nullptr)
    {
        qDebug() << "Not found!";
        return false;
    }
    // use the result in a call to dlsym
    qDebug() << "Found";
    //DLL was loaded OK
    AEffect* (VSTCALLBACK* getNewPlugInstance)(audioMasterCallback);

    getNewPlugInstance =(AEffect* (VSTCALLBACK*)(audioMasterCallback)) functionPointerInBundle("VSTPluginMain",TempBundle);
    if (!getNewPlugInstance) getNewPlugInstance =(AEffect* (VSTCALLBACK*)(audioMasterCallback)) functionPointerInBundle("main_macho",TempBundle);
    if (!getNewPlugInstance) getNewPlugInstance =(AEffect* (VSTCALLBACK*)(audioMasterCallback)) functionPointerInBundle("main",TempBundle);
    if (!getNewPlugInstance) getNewPlugInstance =(AEffect* (VSTCALLBACK*)(audioMasterCallback)) functionPointerInBundle("main_plugin",TempBundle);
    if (getNewPlugInstance == nullptr)
    {
        qDebug() << "Plugin could not be instantiated";
        CFRelease(TempBundle);
        return false;
    }
    //main function located OK
    try
    {
        TempPlug=getNewPlugInstance(host);
    }
    catch (...)
    {
        qDebug() << "Load error";
        CFRelease(TempBundle);
        return false;
    }

    if (TempPlug == nullptr)
    {
        qDebug() << "Plugin main function could not be located";
        CFRelease(TempBundle);
        return false;
    }
    //plugin instantiated OK
    qDebug() << "Plugin was loaded OK";
    if (TempPlug->magic != kEffectMagic)
    {
        qDebug() << "Not a VST plugin";
        CFRelease(TempBundle);
        return false;
    }
    qDebug() << "It's a valid VST plugin";

    if (!(TempPlug->flags & effFlagsHasEditor)) {
        qDebug() << "VST No Editor";
        CFRelease(TempBundle);
        return false;
    }

    KillPlug();

    //m_Filename=Filename;

    TempPlug->user=this;
    //switch the plugin off (calls Suspend)
    TempPlug->dispatcher(TempPlug,effMainsChanged,0,0,nullptr,0.0f);

    qDebug() << ("Plug-In Loaded, OK");
    //set sample rate and block size
    TempPlug->dispatcher(TempPlug,effSetSampleRate,0,0,nullptr,m_Samplerate);
    TempPlug->dispatcher(TempPlug,effSetBlockSize,0,m_Buffersize,nullptr,0.0f);

    if (TempPlug->dispatcher(TempPlug,effGetVstVersion,0,0,nullptr,0.0f) >= 2)
    {
        //get I/O configuration for synth plugins - they will declare their
        //own output and input channels
        for (int i=0; i<TempPlug->numInputs+TempPlug->numOutputs;i++)
        {
            if (i<TempPlug->numInputs)
            {
                //input pin
                VstPinProperties temp;
                if (TempPlug->dispatcher(TempPlug,effGetInputProperties,i,0,&temp,0.0f)==1)
                {
                    qDebug() << ("Input pin " + QString::number(i+1) + " label " + QString(temp.label));
                    if (temp.flags & kVstPinIsActive)
                    {
                        qDebug() << ("Input pin " + QString::number(i+1) + " is active");
                    }
                    if (temp.flags & kVstPinIsStereo)
                    {
                        // is index even or zero?
                        if (i%2==0 || i==0)
                        {
                            qDebug() << ("Input pin " + QString::number(i+1) + " is left channel of a stereo pair");
                        }
                        else
                        {
                            qDebug() << ("Input pin " + QString::number(i+1) + " is right channel of a stereo pair");
                        }
                    }
                    else
                    {
                        qDebug() << ("Input pin " + QString::number(i+1) + " is mono");
                    }
                }
            }
            else
            {
                //output pin
                VstPinProperties temp;
                if (TempPlug->dispatcher(TempPlug,effGetOutputProperties,i-TempPlug->numInputs,0,&temp,0.0f)==1)
                {
                    qDebug() << ("Output pin " + QString::number(i-TempPlug->numInputs+1) + " label " + QString(temp.label));
                    if (temp.flags & kVstPinIsActive)
                    {
                        qDebug() << ("Output pin " + QString::number(i-TempPlug->numInputs+1) + " is active");
                    }
                    else
                    {
                        qDebug() << ("Output pin " + QString::number(i-TempPlug->numInputs+1) + " is inactive");
                    }

                    if (temp.flags & kVstPinIsStereo)
                    {
                        // is index even or zero?
                        if ((i-TempPlug->numInputs)%2==0 || (i-TempPlug->numInputs)==0)
                        {
                            qDebug() << ("Output pin " + QString::number(i-TempPlug->numInputs+1) + " is left channel of a stereo pair");
                        }
                        else
                        {
                            qDebug() << ("Output pin " + QString::number(i-TempPlug->numInputs+1) + " is right channel of a stereo pair");
                        }
                    }
                    else
                    {
                        qDebug() << ("Output pin " + QString::number(i-TempPlug->numInputs+1) + " is mono");
                    }
                }
            }
        }
    }	//end VST2 specific

    //switch the plugin back on (calls Resume)
    qDebug() << "Mains on";
    TempPlug->dispatcher(TempPlug,effMainsChanged,0,1,nullptr,0.0f);

    qDebug() << "Zero buffers";
    InBuffers.initZero(m_Buffersize,TempPlug->numInputs);
    OutBuffers.initZero(m_Buffersize,TempPlug->numOutputs);

    qDebug() << "Init view";
    init(nullptr);

    /*
    QSize s = UISize();
    NSView* v = (__bridge NSView*)hostView;
    [v setFrame:NSMakeRect(0,0,s.width(),s.height())];
    */
    qDebug() << "effEditOpen";
    TempPlug->dispatcher(TempPlug,effEditOpen,0,0,superId(),0.0f);

    ptrPlug=TempPlug;
    vstBundle=TempBundle;

    //if (NSView* v = (__bridge NSView*)viewId()) [v setAutoresizingMask:NSViewNotSizable];
    qDebug() << "Set viewSize";
    setViewSize(UISize());

    qDebug() << "Load programNames";
    LoadProgramNames();

    qDebug() << "Set bankpreset";
    setBankPreset(0);
    setVisible(true);
    qDebug() << "Emit plugin changed";
    emit PlugInChanged();
    qDebug() << "Return";
    return true;
}

void CVSTHostClass::LoadProgramNames()
{
    m_ProgramNames.clear();
    if (ptrPlug)
    {
        QString s;
        for (long i=0;i<ptrPlug->numPrograms;i++)
        {
            setBankPreset(i);
            const QString pn=getPlugString(ptrPlug,effGetProgramName,i);
            if (s==pn) break;
            s=pn;
            m_ProgramNames.append(s);
        }
    }
}

//host callback function
//this is called directly by the plug-in!!
//
VstIntPtr VSTCALLBACK CVSTHostClass::host(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr /*value*/, void* ptr, float opt)
{
    //char S[1024];
    //VstTimeInfo VTI;
    long retval=0;
    VstFileSelect* FS=static_cast<VstFileSelect*>(ptr);
    char* p = static_cast<char*>(ptr);
    QStringList FileNames;
    QString Filename;
    QString Filter;
    VstFileType* FT;
    QByteArray b;
    //qDebug() << "VSTHostClass callback" << opcode;
    switch (opcode)
    {
    //VST 1.0 opcodes
    case audioMasterVersion:
        //Input values:
        //none

        //Return Value:
        //0 or 1 for old version
        //2 or higher for VST2.0 host?
        //Debug("plug called audioMasterVersion");
        retval=2400;
        break;

    case audioMasterAutomate:
        //Input values:
        //<index> parameter that has changed
        //<opt> new value

        //Return value:
        //not tested, always return 0

        //NB - this is called when the plug calls
        //setParameterAutomated

        //Debug("plug called audioMasterAutomate");
        effect->setParameter(effect,index,opt);
        break;

    case audioMasterCurrentId:
        //Input values:
        //none

        //Return Value
        //the unique id of a plug that's currently loading
        //zero is a default value and can be safely returned if not known
        //Debug("plug called audioMasterCurrentId");
        retval=effect->uniqueID;
        break;
    case audioMasterIdle:
        //Input values:
        //none

        //Return Value
        //not tested, always return 0

        //NB - idle routine should also call effEditIdle for all open editors
        //Sleep(1);
        //Debug("plug called audioMasterIdle");
        /* 2.4
                        effect->dispatcher(effect,effIdle,0,0,nullptr,0.0f);
                        for (int i=0;i<Plugs.count();i++)
                        {
                            AEffect* E=Plugs[i];
                            if (E->flags & effFlagsHasEditor)
                            {
                                E->dispatcher(E,effEditIdle,0,0,nullptr,0.0f);
                            }
                        }
                        */
        break;
        /* 2.4
                case audioMasterPinConnected:
                        //Input values:
                        //<index> pin to be checked
                        //<value> 0=input pin, non-zero value=output pin

                        //Return values:
                        //0=true, non-zero=false
                        //Debug("plug called audioMasterPinConnected");
                        break;

                //VST 2.0 opcodes
                case audioMasterWantMidi:
                        //Input Values:
                        //<value> filter flags (which is currently ignored, no defined flags?)

                        //Return Value:
                        //not tested, always return 0
                        //Debug("plug called audioMasterWantMidi");
                        break;
*/
    case audioMasterGetTime:
        //Input Values:
        //<value> should contain a mask indicating which fields are required
        //...from the following list?
        //kVstNanosValid
        //kVstPpqPosValid
        //kVstTempoValid
        //kVstBarsValid
        //kVstCyclePosValid
        //kVstTimeSigValid
        //kVstSmpteValid
        //kVstClockValid

        //Return Value:
        //ptr to populated const VstTimeInfo structure (or 0 if not supported)

        returnInfo.sampleRate =  CPresets::presets().SampleRate;
        returnInfo.timeSigNumerator = 4;
        returnInfo.timeSigDenominator = 4;
        returnInfo.smpteFrameRate = 1;
        returnInfo.samplePos = 0;
        returnInfo.ppqPos = 0;
        //Flags := [vtiNanosValid, vtiPpqPosValid, vtiTempoValid, vtiBarsValid,
        // vtiCyclePosValid, vtiTimeSigValid, vtiSmpteValid, vtiClockValid];
        retval=(long)&returnInfo;
        //NB - this structure will have to be held in memory for long enough
        //for the plug to safely make use of it
        //Debug("plug called audioMasterGetTime");
        break;

    case audioMasterProcessEvents:
        //Input Values:
        //<ptr> Pointer to a populated VstEvents structure

        //Return value:
        //0 if error
        //1 if OK
        //Debug("plug called audioMasterProcessEvents");
        break;
        /* 2.4
                case audioMasterSetTime:
                        //IGNORE!
                        break;

                case audioMasterTempoAt:
                        //Input Values:
                        //<value> sample frame location to be checked

                        //Return Value:
                        //tempo (in bpm * 10000)
                        //Debug("plug called audioMasterTempoAt");
                        retval= 120*10000;
                        break;

                case audioMasterGetNumAutomatableParameters:
                        //Input Values:
                        //None

                        //Return Value:
                        //number of automatable parameters
                        //zero is a default value and can be safely returned if not known

                        //NB - what exactly does this mean? can the host set a limit to the
                        //number of parameters that can be automated?
                        //Debug("plug called audioMasterGetNumAutomatableParameters");
                        break;

                case audioMasterGetParameterQuantization:
                        //Input Values:
                        //None

                        //Return Value:
                        //integer value for +1.0 representation,
                        //or 1 if full single float precision is maintained
                        //in automation.

                        //NB - ***possibly bugged***
                        //Steinberg notes say "parameter index in <value> (-1: all, any)"
                        //but in aeffectx.cpp no parameters are taken or passed
                        //Debug("plug called audioMasterGetParameterQuantization");
                        break;
*/
    case audioMasterIOChanged:
        //Input Values:
        //None

        //Return Value:
        //0 if error
        //non-zero value if OK
        //Debug("plug called audioMasterIOChanged");
        break;
        /* 2.4
                case audioMasterNeedIdle:
                        //Input Values:
                        //None

                        //Return Value:
                        //0 if error
                        //non-zero value if OK

                        //NB plug needs idle calls (outside its editor window)
                        //this means that effIdle must be dispatched to the plug
                        //during host idle process and not effEditIdle calls only when its
                        //editor is open
                        //Check despatcher notes for any return codes from effIdle
                        //Debug("plug called audioMasterNeedIdle");
                        effect->dispatcher(effect,effIdle,0,0,nullptr,0.0f);
                        if (effect->flags & effFlagsHasEditor)
                        {
                            effect->dispatcher(effect,effEditIdle,0,0,nullptr,0.0f);
                        }
                        retval=1;
                        break;
*/
    case audioMasterSizeWindow:
        //Input Values:
        //<index> width
        //<value> height

        //Return Value:
        //0 if error
        //non-zero value if OK
        //Debug("plug called audioMasterSizeWindow");
        break;

    case audioMasterGetSampleRate:
        //Input Values:
        //None

        //Return Value:
        //not tested, always return 0

        //NB - Host must despatch effSetSampleRate to the plug in response
        //to this call
        //Check despatcher notes for any return codes from effSetSampleRate
        //Debug("plug called audioMasterGetSampleRate");
        //TempF=Presets().SampleRate;
        //effect->dispatcher(effect,effSetSampleRate,0,0,nullptr,TempF);
        retval=CPresets::presets().SampleRate;
        break;

    case audioMasterGetBlockSize:
        //Input Values:
        //None

        //Return Value:
        //not tested, always return 0

        //NB - Host must despatch effSetBlockSize to the plug in response
        //to this call
        //Check despatcher notes for any return codes from effSetBlockSize
        //Debug("plug called audioMasterGetBlockSize");
        //TempL=Presets().ModulationRate;
        //effect->dispatcher(effect,effSetBlockSize,0,TempL,nullptr,0.0f);
        retval=CPresets::presets().ModulationRate;
        break;

    case audioMasterGetInputLatency:
        //Input Values:
        //None

        //Return Value:
        //input latency (in sampleframes?)
        //Debug("plug called audioMasterGetInputLatency");
        break;

    case audioMasterGetOutputLatency:
        //Input Values:
        //None

        //Return Value:
        //output latency (in sampleframes?)
        //Debug("plug called audioMasterGetOutputLatency");
        break;
        /* 2.4
                case audioMasterGetPreviousPlug:
                        //Input Values:
                        //None

                        //Return Value:
                        //pointer to AEffect structure or nullptr if not known?

                        //NB - ***possibly bugged***
                        //Steinberg notes say "input pin in <value> (-1: first to come)"
                        //but in aeffectx.cpp no parameters are taken or passed
                        //Debug("plug called audioMasterGetPreviousPlug");
                        break;

                case audioMasterGetNextPlug:
                        //Input Values:
                        //None

                        //Return Value:
                        //pointer to AEffect structure or nullptr if not known?

                        //NB - ***possibly bugged***
                        //Steinberg notes say "output pin in <value> (-1: first to come)"
                        //but in aeffectx.cpp no parameters are taken or passed
                        //Debug("plug called audioMasterGetNextPlug");
                        break;

                case audioMasterWillReplaceOrAccumulate:
                        //Input Values:
                        //None

                        //Return Value:
                        //0: not supported
                        //1: replace
                        //2: accumulate
                        //Debug("plug called audioMasterWillReplaceOrAccumulate");
                        retval=1;
                        break;
*/
    case audioMasterGetCurrentProcessLevel:
        //Input Values:
        //None

        //Return Value:
        //0: not supported,
        //1: currently in user thread (gui)
        //2: currently in audio thread (where process is called)
        //3: currently in 'sequencer' thread (midi, timer etc)
        //4: currently offline processing and thus in user thread
        //other: not defined, but probably pre-empting user thread.
        //Debug("plug called audioMasterGetCurrentProcessLevel");
        break;

    case audioMasterGetAutomationState:
        //Input Values:
        //None

        //Return Value:
        //0: not supported
        //1: off
        //2:read
        //3:write
        //4:read/write
        //Debug("plug called audioMasterGetAutomationState");
        break;

    case audioMasterGetVendorString:
        //Input Values:
        //<ptr> string (max 64 chars) to be populated

        //Return Value:
        //0 if error
        //non-zero value if OK
        //Debug("plug called audioMasterGetVendorString");
        zeroMemory(ptr,64);
        b = "Veinge Musik och Data";
        strlcpy((char*)ptr, b.constData(), b.size());
        //ptr=S;
        retval=1;
        break;

    case audioMasterGetProductString:
        //Input Values:
        //<ptr> string (max 64 chars) to be populated

        //Return Value:
        //0 if error
        //non-zero value if OK
        //Debug("plug called audioMasterGetProductString");
        zeroMemory(ptr,64);
        b = "Object Studio";
        strlcpy((char*)ptr, b.constData(), b.size());
        //ptr=S;
        retval=1;
        break;

    case audioMasterGetVendorVersion:
        //Input Values:
        //None

        //Return Value:
        //Vendor specific host version as integer
        //Debug("plug called audioMasterGetVendorVersion");
        retval=1;
        break;

    case audioMasterVendorSpecific:
        //Input Values:
        //<index> lArg1
        //<value> lArg2
        //<ptr> ptrArg
        //<opt>	floatArg

        //Return Values:
        //Vendor specific response as integer
        //Debug("plug called audioMasterVendorSpecific");
        break;
        /* 2.4
                case audioMasterSetIcon:
                        //IGNORE
                        break;
*/
    case audioMasterCanDo:
        //Input Values:
        //<ptr> predefined "canDo" string

        //Return Value:
        //0 = Not Supported
        //non-zero value if host supports that feature

        //NB - Possible Can Do strings are:
        //"sendVstEvents",
        //"sendVstMidiEvent",
        //"sendVstTimeInfo",
        //"receiveVstEvents",
        //"receiveVstMidiEvent",
        //"receiveVstTimeInfo",
        //"reportConnectionChanges",
        //"acceptIOChanges",
        //"sizeWindow",
        //"asyncProcessing",
        //"offline",
        //"supplyIdle",
        //"supportShell"
        //Debug("plug called audioMasterCanDo" + AnsiString((char*)ptr));
        if (strcmp(p,"sendVstEvents")==0 ||
                strcmp(p,"receiveVstMidiEvent")==0 ||
                strcmp(p,"receiveVstEvents")==0 ||
                strcmp(p,"sendVstMidiEvent")==0 ||
                strcmp(p,"sendVstTimeInfo")==0 ||
                strcmp(p,"asyncProcessing")==0 ||
                strcmp(p,"offline")==0 ||
                strcmp(p,"sizeWindow")==0 ||
                strcmp(p,"supplyIdle")==0)
        {
            retval=1;
        }
        else
        {
            retval=0;
        }

        break;

    case audioMasterGetLanguage:
        //Input Values:
        //None

        //Return Value:
        //kVstLangEnglish
        //kVstLangGerman
        //kVstLangFrench
        //kVstLangItalian
        //kVstLangSpanish
        //kVstLangJapanese
        //Debug("plug called audioMasterGetLanguage");
        retval=kVstLangEnglish;
        break;
        /*
                MAC SPECIFIC?

                case audioMasterOpenWindow:
                        //Input Values:
                        //<ptr> pointer to a VstWindow structure

                        //Return Value:
                        //0 if error
                        //else platform specific ptr
                        Debug("plug called audioMasterOpenWindow");
                        break;

                case audioMasterCloseWindow:
                        //Input Values:
                        //<ptr> pointer to a VstWindow structure

                        //Return Value:
                        //0 if error
                        //Non-zero value if OK
                        Debug("plug called audioMasterCloseWindow");
                        break;
*/
    case audioMasterGetDirectory:
        //Input Values:
        //None

        //Return Value:
        //0 if error
        //FSSpec on MAC, else char* as integer

        //NB Refers to which directory, exactly?
        //Debug("plug called audioMasterGetDirectory");
        b = CPresets::presets().VSTPath.toUtf8();
        zeroMemory(returnString,1024);
        strlcpy(returnString, b.constData(),b.size());
        retval=(long)&returnString[0];
        break;

    case audioMasterUpdateDisplay:
        //Input Values:
        //None

        //Return Value:
        //Unknown
        //Debug("plug called audioMasterUpdateDisplay");
        if (effect->flags & effFlagsHasEditor)
        {
            effect->dispatcher(effect,effEditIdle,0,0,nullptr,0.0f);
        }
        break;
        //---from here VST 2.1 extension opcodes------------------------------------------------------
    case audioMasterBeginEdit:
        // begin of automation session (when mouse down), parameter index in <index>
        //Debug("plug called audioMasterBeginEdit");
        break;
    case audioMasterEndEdit:
        // end of automation session (when mouse up),     parameter index in <index>
        //Debug("plug called audioMasterEndEdit");
        break;
    case audioMasterOpenFileSelector:
        // open a fileselector window with VstFileSelect* in <ptr>
        if (ptr) // && !HostDialog)
        {
            switch (FS->type)
            {
            case kVstFileLoad:
                FT=static_cast<VstFileType*>(FS->fileTypes);
                for (int i=0;i<FS->nbFileTypes;i++)
                {
                    Filter = QString(QString(FT[i].name) + " (*." +
                                     QString(FT[i].dosType) + ")|*." +
                                     QString(FT[i].dosType) + "|");
                }
                Filename=QFileDialog::getOpenFileName(0,FS->title,FS->initialPath,Filter);
                if (!Filename.isEmpty())
                {
                    b = Filename.toUtf8();
                    nFileStrings = 1;
                    FileStrings = new char*[nFileStrings];
                    FileStrings[0] = new char[b.size()];
                    strlcpy(FileStrings[0],b.constData(),b.size());
                    FS->returnPath=FileStrings[0];
                    FS->sizeReturnPath=b.size();
                }
                break;
            case kVstFileSave:
                FT=static_cast<VstFileType*>(FS->fileTypes);
                for (int i=0;i<FS->nbFileTypes;i++)
                {
                    Filter = QString(QString(FT[i].name) + " (*." +
                                     QString(FT[i].dosType) + ")|*." +
                                     QString(FT[i].dosType) + "|");
                }
                Filename=QFileDialog::getSaveFileName(0,FS->title,FS->initialPath,Filter);
                if (!Filename.isEmpty())
                {
                    b = Filename.toUtf8();
                    nFileStrings = 1;
                    FileStrings = new char*[nFileStrings];
                    FileStrings[0] = new char[b.size()];
                    strlcpy(FileStrings[0],b.constData(),b.size());
                    FS->returnPath=FileStrings[0];
                    FS->sizeReturnPath=b.size();
                }
                break;
            case kVstMultipleFilesLoad:
                FT=static_cast<VstFileType*>(FS->fileTypes);
                for (int i=0;i<FS->nbFileTypes;i++)
                {
                    Filter = QString(QString(FT[i].name) + " (*." +
                                     QString(FT[i].dosType) + ")|*." +
                                     QString(FT[i].dosType) + "|");
                }
                FileNames=QFileDialog::getOpenFileNames(0,FS->title,FS->initialPath,Filter);
                if (FileNames.size())
                {
                    nFileStrings=FileNames.size();
                    FileStrings=new char*[nFileStrings];
                    for (int i=0;i<nFileStrings;i++)
                    {
                        b = FileNames[i].toUtf8();
                        FileStrings[i]=new char[b.size()];
                        strlcpy(FileStrings[i],b.constData(),b.size());
                    }
                    FS->nbReturnPath=nFileStrings;
                    FS->returnMultiplePaths=FileStrings;
                }
                break;
            case kVstDirectorySelect:
                Filename=QFileDialog::getExistingDirectory(0,FS->title,FS->initialPath);
                if (!Filename.isEmpty())
                {
                    b = Filename.toUtf8();
                    nFileStrings = 1;
                    FileStrings = new char*[nFileStrings];
                    FileStrings[0] = new char[b.size()];
                    strlcpy(FileStrings[0],b.constData(),b.size());
                    FS->returnPath=FileStrings[0];
                    FS->sizeReturnPath=b.size();
                }
                break;

            }
            if (!FS->returnPath)
            {
                retval=1;
            }
        }
        //Debug("plug called audioMasterOpenFileSelector");
        break;
        //---from here VST 2.2 extension opcodes------------------------------------------------------
    case audioMasterCloseFileSelector:
        // close a fileselector operation with VstFileSelect* in <ptr>
        // Must be always called after an open !
        /*
                        if (HostDialog && ptr)
                        {
                            switch (FS->type)
                            {
                                case kVstFileLoad:
                                    OD=(TOpenDialog*)HostDialog;
                                    if (OD->Title.AnsiCompare(FS->title)==0)
                                    {
                                        delete OD;
                                        //OD=nullptr;
                                    }
                                    break;
                                case kVstFileSave:
                                    SD=(TSaveDialog*)HostDialog;
                                    if (SD->Title.AnsiCompare(FS->title)==0)
                                    {
                                        delete SD;
                                        //SD=nullptr;
                                    }
                                    break;
                                case kVstMultipleFilesLoad:
                                    OD=(TOpenDialog*)HostDialog;
                                    if (OD->Title.AnsiCompare(FS->title)==0)
                                    {
                                        delete OD;
                                        //OD=nullptr;
                                    }
                                    break;
                                case kVstDirectorySelect:
                                    BD=(TLMDBrowseDlg*)HostDialog;
                                    if (BD->CaptionTitle.AnsiCompare(FS->title)==0)
                                    {
                                        delete BD;
                                        //BD=nullptr;
                                    }
                                    break;
                            }
                        }
                        */
        if (FileStrings)
        {
            for (int i=0;i<nFileStrings;i++)
            {
                delete[] FileStrings[i];
            }
            delete[] FileStrings;
            FileStrings=nullptr;
        }

        //Debug("plug called audioMasterCloseFileSelector");
        break;
        /* 2.4
                case audioMasterEditFile:
                                                // open an editor for audio (defined by XML text in ptr)
                        //Debug("plug called audioMasterEditFile");
                        break;
                case audioMasterGetChunkFile:
                                        // get the native path of currently loading bank or project
                                        // (called from writeChunk) void* in <ptr> (char[2048], or sizeof(FSSpec))
                        //Debug("plug called audioMasterGetChunkFile");
                        break;
*/
    }

    return retval;
}

int CVSTHostClass::parameterCount() const
{
    if (ptrPlug) return ptrPlug->numParams;
    return 0;
}

float CVSTHostClass::parameter(const long index) const
{
    if (ptrPlug) return ptrPlug->getParameter(ptrPlug,index);
    return 0;
}

void CVSTHostClass::setParameter(const long index, const float value)
{
    if (ptrPlug) ptrPlug->setParameter(ptrPlug,index,value);
}

const QString CVSTHostClass::parameterName(const long index)
{;
    QString retVal;
    if (ptrPlug)
    {
        //qDebug() << "VSTHostClass parameterName";
        retVal = getPlugString(ptrPlug,effGetParamName,index).trimmed();
        if (retVal.isEmpty()) retVal = getPlugString(ptrPlug,effGetParamLabel,index).trimmed();
    }
    return retVal;
}

const QString CVSTHostClass::parameterValue(const long index)
{
    if (ptrPlug)
    {
        //qDebug() << "VSTHostClass parameterValue";
        return getPlugString(ptrPlug,effGetParamDisplay,index).trimmed();
    }
    return QString();
}

int CVSTHostClass::inputCount()
{
    if (ptrPlug) return ptrPlug->numInputs;
    return 0;
}

int CVSTHostClass::outputCount()
{
    if (ptrPlug) return ptrPlug->numOutputs;
    return 0;
}

float CVSTHostClass::VSTVersion()
{
    if (!ptrPlug) return 0;
    return ptrPlug->dispatcher(ptrPlug,effGetVstVersion,0,0,nullptr,0.0f);
}

bool CVSTHostClass::process()
{
    if (!ptrPlug) return false;
        //qDebug() << "VSTHostClass Process";
    ptrPlug->dispatcher(ptrPlug,effStartProcess,0,0,nullptr,0.0f);
    // Called before the start of process call

    //ProcessEvents
    if (ptrPlug && vstMidiEvents.size()) {
        vstEventsBuffer.resize(sizeof(VstEvents) +
                               (sizeof(VstEvent *) * vstMidiEvents.size()));
        VstEvents *vstEvents = reinterpret_cast<VstEvents*>(&vstEventsBuffer.front());

        vstEvents->numEvents = vstMidiEvents.size();
        vstEvents->reserved = 0;
        for (ulong64 i = 0, n = vstEvents->numEvents; i < n; i++)
        {
            vstEvents->events[i] = vstMidiEvents[i];
        }
        ptrPlug->dispatcher(ptrPlug,effProcessEvents,0,0,vstEvents,0.0f);
    }

    //Some plugs don't replace even if processReplacing is called so we must flush buffers
    //Some people don't do that for you !!!
    /*
                for (int i=0;i<ptrPlug->numOutputs;i++)
                {
                        ZeroMemory(ptrOutputBuffers[i],ModRate*sizeof(float));
                }
                */
    //process (replacing)
    //if (ptrPlug->flags & effFlagsCanReplacing)
    //{
    if (!ptrPlug) return false;
    ptrPlug->processReplacing(ptrPlug,InBuffers.channelPointers(),OutBuffers.channelPointers(),m_Buffersize);
    /* 2.4
                }
                else
                {
                        ptrPlug->process(ptrPlug,ptrInputBuffers,ptrOutputBuffers,ModRate);
                }
                */
    //((TfrmVSTHost*)m_EditForm)->EditIdle();
    // Called after the stop of process call
    if (!ptrPlug) return false;
    ptrPlug->dispatcher(ptrPlug,effStopProcess,0,0,nullptr,0.0f);
    qDeleteAll(vstMidiEvents);
    vstMidiEvents.clear();
    vstSysExData.clear();
    return true;
}

VstEvent* createVSTEvent(const CMIDIEvent* Event)
{
    VstMidiEvent* e=new VstMidiEvent;
    e->type=kVstMidiType;
    e->byteSize = sizeof(VstMidiEvent);
    e->deltaFrames=0L;
    e->flags=0L;
    e->noteLength=0L;
    e->noteOffset=0L;

    e->midiData[0]=Event->message();	//status & channel
    e->midiData[1]=Event->data(0);	//MIDI byte #2
    if (Event->dataSize() > 1)
    {
        e->midiData[2]=Event->data(1);	//MIDI byte #3
    }
    else
    {
        e->midiData[2]=0x00;
    }
    e->midiData[3]=0x00;	//MIDI byte #4 - blank

    e->detune=0x00;
    e->noteOffVelocity=0x00;
    e->reserved1=0x00;
    e->reserved2=0x00;
    return reinterpret_cast<VstEvent*>(e);
}

VstEvent* createVSTSysExEvent(std::vector<byte>& sysExData)
{
    VstMidiSysexEvent* e=new VstMidiSysexEvent;
    e->type = kVstSysExType;
    e->byteSize = sizeof(VstMidiSysexEvent);
    e->deltaFrames = 0L;
    e->flags = 0L;
    e->dumpBytes = sysExData.size();
    e->resvd1 = 0x00;
    e->sysexDump = reinterpret_cast<char*>(sysExData.data());
    e->resvd2 = 0x00;
    return reinterpret_cast<VstEvent*>(e);
}

void CVSTHostClass::parseEvent(const CMIDIEvent* Event)
{
    if (!ptrPlug) return;
    if (Event->isSysEx())
    {
        std::vector<byte> v(Event->toVector());
        vstMidiEvents.append(createVSTSysExEvent(v));
        vstSysExData.append(v);
    }
    else
    {
        Event->transpose(m_Transpose);
        vstMidiEvents.append(createVSTEvent(Event));
    }
}

void CVSTHostClass::KillPlug()
{
    qDeleteAll(vstMidiEvents);
    vstMidiEvents.clear();
    vstSysExData.clear();

    if (!ptrPlug) return;

    AEffect* TempPlug = ptrPlug;
    ptrPlug=nullptr;

    CFBundleRef TempBundle=(CFBundleRef)vstBundle;

    qDebug() << "effEditClose";
    if (TempPlug->flags & effFlagsHasEditor)
    {
        //try {
            //TempPlug->dispatcher(TempPlug,effEditClose,0,0,nullptr,0.0f);
        //}
        //catch(...) {}
    }
    qDebug() << "Destroy Macwin";
    destroyMacWindow();

    qDebug() << "effMainsChanged";
    TempPlug->dispatcher(TempPlug,effMainsChanged,0,0,nullptr,0.0f); //calls suspend

    qDebug() << "clear programnames";
    m_ProgramNames.clear();
    qDebug() << "clear bank";
    CurrentBank.clear();
    qDebug() << "clear preset";
    CurrentPreset.clear();
    qDebug() << "clear fileparameter";
    //fileParameter->clear();

    //Shut the plugin down and free the library (this deletes the C++ class
    //memory and calls the appropriate destructors...)
    qDebug() << "effClose";
    TempPlug->dispatcher(TempPlug,effClose,0,0,nullptr,0.0f);
    TempPlug = nullptr;
    qDebug() << "CFRelease";
    CFRelease(TempBundle);
}

void CVSTHostClass::serialize(QDomLiteElement* xml) const
{
    xml->setAttribute("BankPath",QDir(presets.VSTPath).relativeFilePath(CurrentBank));
    xml->setAttribute("PresetPath",QDir(presets.VSTPath).relativeFilePath(CurrentPreset));
    int p = currentBankPreset();
    xml->setAttribute("Preset",p);
    xml->setAttribute("NumParams",parameterCount());
    QDomLiteElement* Params = xml->appendChild("Parameters");
    for (int i=0;i<parameterCount();i++) Params->setAttribute("Param" + QString::number(i),parameter(i));
    //IAudioPlugInHost::serialize(xml);
}

void CVSTHostClass::unserialize(const QDomLiteElement* xml)
{
    if (!xml) return;
    CurrentBank=xml->attribute("BankPath");
    CurrentPreset=xml->attribute("PresetPath");
    if (!CurrentBank.isEmpty())
    {
        const QString Expath=QDir(presets.VSTPath).absoluteFilePath(CurrentBank);
        loadBank(Expath);
    }
    if (!CurrentPreset.isEmpty())
    {
        const QString Expath=QDir(presets.VSTPath).absoluteFilePath(CurrentPreset);
        loadPreset(Expath);
    }
    const int nParams=xml->attributeValueInt("NumParams");
    const int p=xml->attributeValueInt("Preset");
    setBankPreset(p);
    if (const QDomLiteElement* Params = xml->elementByTag("Parameters"))
    {
        for (int i = nParams-1; i >- 1; i--)
        {
            const float Param = Params->attributeValue("Param" + QString::number(i));
            setParameter(i,Param);
        }
    }
    //IAudioPlugInHost::unserialize(xml);
}

void CVSTHostClass::loadBank(QString FileName)
{
    QFile f(FileName);
    if (!f.exists())
    {
        qDebug() << "Bank does not exist";
        return;
    }
    if (f.open(QIODevice::ReadOnly))
    {
        loadBank(f);
        if (CurrentBank.isEmpty()) CurrentBank=FileName;
    }
}

void CVSTHostClass::loadPreset(QString FileName)
{
    QFile f(FileName);
    if (!f.exists())
    {
        return;
    }
    if (f.open(QIODevice::ReadOnly))
    {
        loadPreset(f);
        if (CurrentPreset.isEmpty()) CurrentPreset=FileName;
    }
}

fxPreset CVSTHostClass::GetPreset(long Index) const
{
    fxPreset result;
    ptrPlug->dispatcher(ptrPlug,effSetProgram,0,Index,nullptr,0.0f);
    setDescriptor(result.chunkMagic,"CcnK");
    setDescriptor(result.fxMagic,"FxCk");
    result.version=qToBigEndian<int>(1);
    result.fxID=qToBigEndian<int>(ptrPlug->uniqueID);
    result.fxVersion=qToBigEndian<int>((int)ptrPlug->version);
    result.numParams=qToBigEndian<int>((int)ptrPlug->numParams);
    char temp[256];
    ptrPlug->dispatcher(ptrPlug,effGetProgramName,0,0,temp,0.0f);
    copyMemory(result.prgName,temp,256);
    result.params=new float[ptrPlug->numParams];//calloc(ptrPlug->numParams,sizeof(float));
    int* pc=static_cast<int*>(result.params);
    for (long i=0;i<ptrPlug->numParams;i++)
    {
        float si=ptrPlug->getParameter(ptrPlug,i);
        int x=*(reinterpret_cast<int*>(&si));
        x=qToBigEndian<int>(x);
        *pc=x;
        pc++;
    }
    result.byteSize=qToBigEndian<int>(sizeof(result)-sizeof(int)*2+(ptrPlug->numParams-1)*sizeof(float));
    return result;
}

void CVSTHostClass::saveBank(QString FileName)
{
    QFile f(FileName);
    if (f.open(QIODevice::WriteOnly))
    {
        saveBank(f);
        CurrentBank=FileName;
    }
}

void CVSTHostClass::savePreset(QString FileName)
{
    QFile f(FileName);
    if (f.open(QIODevice::WriteOnly))
    {
        savePreset(f);
    }
}

int CVSTHostClass::GetChunk(void* pntr,bool isPreset) const
{
    return ptrPlug->dispatcher(ptrPlug,effGetChunk,long(isPreset),0,pntr,0.0f);
}

int CVSTHostClass::SetChunk(void* data,long byteSize,bool isPreset)
{
    return ptrPlug->dispatcher(ptrPlug,effSetChunk, long(isPreset), byteSize, data,0.0f);
}


void CVSTHostClass::savePreset(QFile& str) const
{
    if (!ptrPlug) return;
    if (ptrPlug->flags & effFlagsProgramChunks)
    {
        fxChunkSet p2;
        setDescriptor(p2.chunkMagic,"CcnK");
        setDescriptor(p2.fxMagic,"FPCh");
        p2.version=qToBigEndian<int>(1);
        p2.fxID=qToBigEndian<int>(ptrPlug->uniqueID);
        p2.fxVersion=qToBigEndian<int>(ptrPlug->version);
        p2.numPrograms=qToBigEndian<int>(ptrPlug->numPrograms);
        char temp[256];
        ptrPlug->dispatcher(ptrPlug,effGetProgramName,0,0,temp,0.0f);
        copyMemory(p2.prgName,temp,256);
        void* PBuffer;
        int x=GetChunk(&PBuffer,true);
        p2.chunkSize=qToBigEndian<int>(x);
        p2.byteSize=qToBigEndian<int>(sizeof(p2) - sizeof(int) * 2 + x - 8);
        str.write((char*)&p2,sizeof(p2)-sizeof(void*));
        str.write((char*)PBuffer, x);
    }
    else
    {
        long i=ptrPlug->dispatcher(ptrPlug,effGetProgram,0,0,nullptr,0.0f);
        fxPreset pp=GetPreset(i);
        str.write((char*)&pp,sizeof(pp)-sizeof(float));
        str.write((char*)pp.params, sizeof(float) * ptrPlug->numParams);
        delete[] (float*)pp.params;
    }
}

void CVSTHostClass::loadBank(QFile& str)
{
    if (!ptrPlug) return;
    if (ptrPlug->flags & effFlagsProgramChunks)
    {
        fxChunkBank p2;
        str.read(reinterpret_cast<char*>(&p2),sizeof(fxChunkBank)-sizeof(void*));
        if (qFromBigEndian<int>(p2.fxID) != ptrPlug->uniqueID)
        {
            qDebug() << "Bank file not for this Plug-in";
            return;
        }
        const int x=str.size() - str.pos();
        void* pb2=calloc(1,x+1);
        const long j=str.read(reinterpret_cast<char*>(pb2),x);
        SetChunk(pb2,j,false);
        free(pb2);
    }
    else
    {
        fxSet p;
        str.read(reinterpret_cast<char*>(&p),sizeof(fxSet)-sizeof(void*));
        if (qFromBigEndian<int>(p.fxID) != ptrPlug->uniqueID)
        {
            qDebug() << "Bank file not for this Plug-in";
            return;
        }
        VstPatchChunkInfo pci;
        pci.version=1;
        pci.pluginUniqueID=ptrPlug->uniqueID;
        pci.pluginVersion=ptrPlug->version;
        pci.numElements=ptrPlug->numPrograms;
        ptrPlug->dispatcher(ptrPlug,effBeginLoadBank,0,0,&pci,0.0f);
        p.numPrograms=qFromBigEndian<int>(p.numPrograms);
        for (long j=0;j<p.numPrograms;j++)
        {
            fxPreset pp;
            str.read(reinterpret_cast<char*>(&pp),sizeof(fxPreset)-sizeof(void*));
            ptrPlug->dispatcher(ptrPlug,effBeginSetProgram,0,0,nullptr,0.0f);
            ptrPlug->dispatcher(ptrPlug,effSetProgram,0,j,nullptr,0.0f);
            ptrPlug->dispatcher(ptrPlug,effEndSetProgram,0,0,nullptr,0.0f);
            ptrPlug->dispatcher(ptrPlug,effSetProgramName,0,0,pp.prgName,0.0f);
            pp.numParams=qFromBigEndian<int>(pp.numParams);
            int x;
            for (long i=0;i<pp.numParams;i++)
            {
                str.read(reinterpret_cast<char*>(&x),sizeof(int));
                x=qFromBigEndian<int>(x);
                const float s=*(reinterpret_cast<float*>(&x));
                ptrPlug->setParameter(ptrPlug,i,s);
            }
        }
    }
    ptrPlug->dispatcher(ptrPlug,effBeginSetProgram,0,0,nullptr,0.0f);
    ptrPlug->dispatcher(ptrPlug,effSetProgram,0,0,nullptr,0.0f);
    ptrPlug->dispatcher(ptrPlug,effEndSetProgram,0,0,nullptr,0.0f);
}

void CVSTHostClass::loadPreset(QFile& str)
{
    if (!ptrPlug) return;
    if (ptrPlug->flags & effFlagsProgramChunks)
    {
        fxChunkSet p2;
        str.read(reinterpret_cast<char*>(&p2),sizeof(fxChunkSet)-sizeof(void*));
        if (qFromBigEndian<int>(p2.fxID) != ptrPlug->uniqueID)
        {
            qDebug() << "Preset file not for this Plug-in";
            return;
        }
        ptrPlug->dispatcher(ptrPlug,effSetProgramName,0,0,p2.prgName,0.0f);
        int x=str.size()-str.pos();
        void* pb2=calloc(1,x+1);
        long j=str.read(reinterpret_cast<char*>(pb2),x);
        SetChunk(pb2,j,true);
        free(pb2);
    }
    else
    {
        fxPreset p;
        str.read(reinterpret_cast<char*>(&p),sizeof(fxPreset)-sizeof(void*));
        if (qFromBigEndian<int>(p.fxID) != ptrPlug->uniqueID)
        {
            qDebug() << "Preset file not for this Plug-in";
            return;
        }
        VstPatchChunkInfo pci;
        pci.version=1;
        pci.pluginUniqueID=ptrPlug->uniqueID;
        pci.pluginVersion=ptrPlug->version;
        pci.numElements=ptrPlug->numParams;
        ptrPlug->dispatcher(ptrPlug,effBeginLoadProgram,0,0,&pci,0.0f);
        ptrPlug->dispatcher(ptrPlug,effSetProgramName,0,0,p.prgName,0.0f);
        p.numParams=qFromBigEndian<int>(p.numParams);
        int x;
        for (long i=0;i<p.numParams;i++)
        {
            str.read(reinterpret_cast<char*>(&x),sizeof(int));
            x=qFromBigEndian<int>(x);
            const float s=*(reinterpret_cast<float*>(&x));
            ptrPlug->setParameter(ptrPlug,i,s);
        }
    }
    ptrPlug->dispatcher(ptrPlug,effBeginSetProgram,0,0,nullptr,0.0f);
    ptrPlug->dispatcher(ptrPlug,effSetProgram,0,0,nullptr,0.0f);
    ptrPlug->dispatcher(ptrPlug,effEndSetProgram,0,0,nullptr,0.0f);
}

void CVSTHostClass::saveBank(QFile& str) const
{
    if (!ptrPlug) return;
    if (ptrPlug->flags & effFlagsProgramChunks)
    {
        fxChunkBank p2;
        setDescriptor(p2.chunkMagic,"Ccnk");
        setDescriptor(p2.fxMagic,"FBCh");
        p2.version=qToBigEndian<int>(1);
        p2.fxID=qToBigEndian<int>(ptrPlug->uniqueID);
        p2.fxVersion=qToBigEndian<int>(ptrPlug->version);
        p2.numPrograms=qToBigEndian<int>(ptrPlug->numPrograms);
        void* PBuffer=nullptr;
        int x=GetChunk(PBuffer,false);
        p2.chunkSize=qToBigEndian<int>(x);
        p2.byteSize=qToBigEndian<int>(sizeof(p2) - sizeof(int) * 3 + x + 8);
        str.write(reinterpret_cast<char*>(&p2),sizeof(p2)-sizeof(void*));
        str.write(reinterpret_cast<char*>(PBuffer),x);
    }
    else
    {
        fxSet p;
        setDescriptor(p.chunkMagic,"Ccnk");
        setDescriptor(p.fxMagic,"FxBk");
        p.version=qToBigEndian<int>(1);
        p.fxID=qToBigEndian<int>(ptrPlug->uniqueID);
        p.fxVersion=qToBigEndian<int>(ptrPlug->version);
        p.numPrograms=qToBigEndian<int>(ptrPlug->numPrograms);
        p.byteSize=qToBigEndian<int>(sizeof(p) - sizeof(int) + (sizeof(fxPreset) + (ptrPlug->numParams-1) * sizeof(float)) * ptrPlug->numParams);
        str.write(reinterpret_cast<char*>(&p),sizeof(p)-sizeof(float));
        for (int j=0;j<ptrPlug->numPrograms;j++)
        {
            fxPreset pp =GetPreset(j);
            str.write(reinterpret_cast<char*>(&pp),sizeof(pp)-sizeof(float));
            str.write(reinterpret_cast<char*>(pp.params),sizeof(float)*ptrPlug->numParams);
            free(pp.params);
        }
    }
}

const QString CVSTHostClass::bankPresetName() const
{
    return m_ProgramName;
}

const QStringList CVSTHostClass::bankPresetNames() const
{
    return m_ProgramNames;
}

void CVSTHostClass::setBankPreset(const long index)
{
    if (ptrPlug)
    {
        m_ProgramName.clear();
        if ((index > -1) && (index < ptrPlug->numPrograms))
        {
            ptrPlug->dispatcher(ptrPlug,effBeginSetProgram,0,0,nullptr,0.0f);
            ptrPlug->dispatcher(ptrPlug,effSetProgram,0,index,nullptr,0.0f);
            ptrPlug->dispatcher(ptrPlug,effEndSetProgram,0,0,nullptr,0.0f);
            ptrPlug->dispatcher(ptrPlug,effEditIdle,0,0,nullptr,0.0f);
            m_ProgramName=getPlugString(ptrPlug,effGetProgramName,currentBankPreset());
        }
        IAudioPlugInHost::setBankPreset(index);
    }
}

long CVSTHostClass::currentBankPreset(const int /*channel*/) const
{
    if (ptrPlug) return ptrPlug->dispatcher(ptrPlug,effGetProgram,0,0,nullptr,0.0f);
    return -1;
}

const QSize CVSTHostClass::UISize() const
{
    VSTRect* FormRect;
    if (ptrPlug)
    {
        //qDebug() << "VSTHostClass UISize";
        ptrPlug->dispatcher(ptrPlug,effEditGetRect,0,0,&FormRect,0.0f);
        //NSView* v = (__bridge NSView*)hostView;
        //NSRect r = [v frame];
        //qDebug() << FormRect->top << FormRect->left << FormRect->right << FormRect->bottom << r.origin.x << r.origin.y << r.size.width << r.size.height;
        return QSize(FormRect->right-FormRect->left,FormRect->bottom-FormRect->top);
    }
    return QSize(0,0);
}

/*
void CVSTHostClass::timerEvent(QTimerEvent *)
{
    if (!m_TimerID) return;
    if (ptrPlug)
    {
        QSize s(UISize());
        if (s != contentSize()) setContentSize(s);
    }
}
*/
