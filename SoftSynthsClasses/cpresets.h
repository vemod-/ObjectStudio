#ifndef CPRESETS_H
#define CPRESETS_H

#include <QtCore>
#include "softsynthsdefines.h"

typedef long double ldouble;

class PresetsType
{
public:
    PresetsType() {}
    uint SampleRate;
    uint HalfRate;
    uint DoubleRate;
    uint ModulationRate;
    uint ModulationHalfRate;
    int MaxCutoff;
    double ModulationTime;
    double uSPerSample;
    double SamplesPermSec;
    uint BufferSize;
    double ModulationsPermSec;
    QString VSTPath;
    double ConnectionsOpacity;
    inline ulong64 mSecsToSamples(const ulong mSecs) const {
        return ldouble(mSecs) * SamplesPermSec;
    }
    inline ulong samplesTomSecs(const ulong64 samples) const {
        return ldouble(samples) / SamplesPermSec;
    }
};
/*
class CPresets
{
public:
    inline static PresetsType& presets()
    {
        return getInstance()->m_Presets;
    }
    static void setSampleRate(const uint s)
    {
        getInstance()->m_Presets.SampleRate=s;
        getInstance()->calcParams();
    }
    static const QString macros(const QString& deviceName)
    {
        return getInstance()->m_Macros[deviceName].toString();
    }
    static void setMacros(const QString& deviceName, const QString& xml)
    {
        getInstance()->m_Macros[deviceName]=xml;
    }
    static const QString resolveFilename(const QString& Filename);
    static ulong64 mSecsToSamples(const ulong mSecs) {
        return presets().mSecsToSamples(mSecs);
    }
    static ulong samplesTomSecs(const ulong64 samples) {
        return presets().samplesTomSecs(samples);
    }
    static QString Organization() {
        return getInstance()->m_Settings->organizationName();
    }
    static QString AppName() {
        return getInstance()->m_Settings->applicationName();
    }
    static void destroyInstance() {
        delete getInstance();
    }
    inline CPresets()
    {
        qDebug() << "Presets loaded";
#ifdef Q_OS_IOS
        m_Settings = new QSettings("Vemod", "ObjectStudio");
#else
        m_Settings = new QSettings("http://www.musiker.nu/objectstudio","ObjectStudio");
#endif
        qDebug() << "Presets file loaded";
        m_Presets.SampleRate=m_Settings->value("SampleRate",44100).toUInt();
        m_Presets.ModulationRate=m_Settings->value("ModulationRate",256).toUInt();
        m_Presets.BufferSize=m_Settings->value("BufferSize",512).toUInt();
        calcParams();
        m_Presets.VSTPath=m_Settings->value("VSTPath",QDir("/Library/Audio/Plug-Ins/VST").absolutePath()).toString();
        m_Presets.ConnectionsOpacity=m_Settings->value("ConnctionsOpacity",0.75).toDouble();
        m_ReplacementFiles=m_Settings->value("ReplacementFiles").toMap();
        m_Macros=m_Settings->value("Macros").toMap();
        qDebug() << "Presets finished loading";
    }
    inline ~CPresets()
    {
        //save presets
        qDebug() << "Presets saved";
        m_Settings->setValue("SampleRate",m_Presets.SampleRate);
        m_Settings->setValue("ModulationRate",m_Presets.ModulationRate);
        m_Settings->setValue("BufferSize",m_Presets.BufferSize);
        m_Settings->setValue("VSTPath",m_Presets.VSTPath);
        m_Settings->setValue("ConnctionsOpacity",m_Presets.ConnectionsOpacity);
        //m_Settings->setValue("RecentFiles",m_RecentFiles);
        m_Settings->setValue("ReplacementFiles",m_ReplacementFiles);
        m_Settings->setValue("Macros",m_Macros);
        delete m_Settings;
    }
private:
    inline CPresets(CPresets const&);              // Don't Implement
    inline void operator=(CPresets const&); // Don't implement
    inline static CPresets* getInstance() {
        static std::unique_ptr<CPresets> instance = std::make_unique<CPresets>();
        return &(*instance);
    }
    QMap<QString,QVariant> m_ReplacementFiles;
    PresetsType m_Presets;
    QStringList m_RecentFiles;
    QMap<QString,QVariant> m_Macros;
    inline void calcParams()
    {
        m_Presets.HalfRate=m_Presets.SampleRate/2;
        m_Presets.ModulationsPermSec=(double(m_Presets.SampleRate)/double(m_Presets.ModulationRate))*0.001;
        m_Presets.ModulationTime=(double(m_Presets.ModulationRate)/double(m_Presets.SampleRate))*1000;
        m_Presets.uSPerSample=1000000.0/double(m_Presets.SampleRate);
        m_Presets.SamplesPermSec=double(m_Presets.SampleRate)*0.001;
        m_Presets.DoubleRate=m_Presets.SampleRate*2;
        m_Presets.ModulationHalfRate=m_Presets.ModulationRate/2;
        m_Presets.MaxCutoff=int(m_Presets.SampleRate*0.425);
    }
protected:
    QSettings* m_Settings;
};
*/
class CPresets
{
public:
    static CPresets* getInstance() {
        // Initieras första gången den faktiskt används
        static CPresets instance;
        return &instance;
    }

    static PresetsType& presets() {
        return getInstance()->m_Presets;
    }

    static void setSampleRate(const uint s)
    {
        getInstance()->m_Presets.SampleRate=s;
        getInstance()->calcParams();
    }
    static const QString macros(const QString& deviceName)
    {
        return getInstance()->m_Macros[deviceName].toString();
    }
    static void setMacros(const QString& deviceName, const QString& xml)
    {
        getInstance()->m_Macros[deviceName]=xml;
    }
    static const QString resolveFilename(const QString& Filename);
    static ulong64 mSecsToSamples(const ulong mSecs) {
        return presets().mSecsToSamples(mSecs);
    }
    static ulong samplesTomSecs(const ulong64 samples) {
        return presets().samplesTomSecs(samples);
    }
    static QString Organization() {
        return getInstance()->m_Settings->organizationName();
    }
    static QString AppName() {
        return getInstance()->m_Settings->applicationName();
    }
    //static void destroyInstance() {
    //    delete getInstance();
    //}

private:
    CPresets() {
        qDebug() << "Presets loaded";

#ifdef Q_OS_IOS
        // NativeFormat = NSUserDefaults på iOS
        m_Settings = new QSettings(QSettings::NativeFormat,
                                   QSettings::UserScope,
                                   "Vemod",
                                   "ObjectStudio");
#else
        m_Settings = new QSettings("http://www.musiker.nu/objectstudio", "ObjectStudio");
#endif

        qDebug() << "Presets file loaded";
        load();
    }

    void load() {
        m_Presets.SampleRate = m_Settings->value("SampleRate", 44100).toUInt();
        m_Presets.ModulationRate = m_Settings->value("ModulationRate", 256).toUInt();
        m_Presets.BufferSize = m_Settings->value("BufferSize", 512).toUInt();
        calcParams();
        m_Presets.VSTPath = m_Settings->value("VSTPath",
                                              QDir("/Library/Audio/Plug-Ins/VST").absolutePath()).toString();
        m_Presets.ConnectionsOpacity = m_Settings->value("ConnctionsOpacity", 0.75).toDouble();
        m_ReplacementFiles = m_Settings->value("ReplacementFiles").toMap();
        m_Macros = m_Settings->value("Macros").toMap();
        qDebug() << "Presets finished loading";
    }

    ~CPresets() {
        save();
        delete m_Settings;
    }

    void save() {
        m_Settings->setValue("SampleRate", m_Presets.SampleRate);
        m_Settings->setValue("ModulationRate", m_Presets.ModulationRate);
        m_Settings->setValue("BufferSize", m_Presets.BufferSize);
        m_Settings->setValue("VSTPath", m_Presets.VSTPath);
        m_Settings->setValue("ConnctionsOpacity", m_Presets.ConnectionsOpacity);
        m_Settings->setValue("ReplacementFiles", m_ReplacementFiles);
        m_Settings->setValue("Macros", m_Macros);
    }

    void calcParams() {
        m_Presets.HalfRate = m_Presets.SampleRate / 2;
        m_Presets.ModulationsPermSec = (double(m_Presets.SampleRate) /
                                        double(m_Presets.ModulationRate)) * 0.001;
        m_Presets.ModulationTime = (double(m_Presets.ModulationRate) /
                                    double(m_Presets.SampleRate)) * 1000;
        m_Presets.uSPerSample = 1000000.0 / double(m_Presets.SampleRate);
        m_Presets.SamplesPermSec = double(m_Presets.SampleRate) * 0.001;
        m_Presets.DoubleRate = m_Presets.SampleRate * 2;
        m_Presets.ModulationHalfRate = m_Presets.ModulationRate / 2;
        m_Presets.MaxCutoff = int(m_Presets.SampleRate * 0.425);
    }

    QSettings* m_Settings = nullptr;
    PresetsType m_Presets;
    QMap<QString,QVariant> m_ReplacementFiles;
    QStringList m_RecentFiles;
    QMap<QString,QVariant> m_Macros;
};

class IPresetRef
{
public:
    IPresetRef() : presets(CPresets::presets()) {}
    const PresetsType& presets;
};

#endif // CPRESETS_H
