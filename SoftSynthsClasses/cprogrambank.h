#ifndef CPROGRAMBANK_H
#define CPROGRAMBANK_H

#include "cpresets.h"
#include "QDomLite"

#define PresetNameAttribute "PresetName"
#define PresetTag "Preset"
#define PresetListTag "Presets"

class CProgramBank
{
public:
    CProgramBank();
    static void init(const QString& deviceName)
    {
        if (!m_ProgramNames.contains(deviceName)) loadProgramNames(deviceName);
    }
    static QDomLiteElement* getProgram(const QString& programName,const QString& deviceName)
    {
        QDomLiteDocument TempDoc(PresetListTag,deviceName);
        QString xml = CPresets::macros(deviceName);
        if (xml.length()) TempDoc.fromString(xml);
        if (!programName.isEmpty())
        {
            for(const QDomLiteElement* e : (const QDomLiteElementList)TempDoc.documentElement->elementsByTag(PresetTag))
            {
                if (e->attribute(PresetNameAttribute)==programName) return e->clone();
            }
        }
        return nullptr;
    }
    static void deleteProgram(const QString& programName,const QString& deviceName)
    {
        m_ProgramNames[deviceName].clear();
        QDomLiteDocument TempDoc(PresetListTag,deviceName);
        QString xml = CPresets::macros(deviceName);
        if (xml.length()) TempDoc.fromString(xml);
        for(QDomLiteElement* e : (const QDomLiteElementList)TempDoc.documentElement->elementsByTag(PresetTag))
        {
            if (e->attribute(PresetNameAttribute)==programName)
            {
                TempDoc.documentElement->removeChild(e);
            }
            else
            {
                m_ProgramNames[deviceName].append(e->attribute(PresetNameAttribute));
            }
        }
        CPresets::setMacros(deviceName,TempDoc.toString());
    }
    static void saveProgram(QDomLiteElement* Parameters,const QString& programName,const QString& deviceName)
    {
        Parameters->setAttribute(PresetNameAttribute,programName);
        m_ProgramNames[deviceName].clear();
        QDomLiteDocument TempDoc(PresetListTag,deviceName);
        QString xml=CPresets::macros(deviceName);
        if (xml.length()) TempDoc.fromString(xml);
        for(QDomLiteElement* e : (const QDomLiteElementList)TempDoc.documentElement->elementsByTag(PresetTag))
        {
            if (e->attribute(PresetNameAttribute)==programName)
            {
                TempDoc.documentElement->removeChild(e);
                break;
            }
            else
            {
                m_ProgramNames[deviceName].append(e->attribute(PresetNameAttribute));
            }
        }
        TempDoc.documentElement->appendClone(Parameters);
        m_ProgramNames[deviceName].append(programName);
        CPresets::setMacros(deviceName,TempDoc.toString());
    }
    static void loadProgramNames(const QString& deviceName)
    {
        m_ProgramNames[deviceName].clear();
        QDomLiteDocument TempDoc(PresetListTag,deviceName);
        QString xml = CPresets::macros(deviceName);
        if (xml.length()) TempDoc.fromString(xml);
        for (const QDomLiteElement* e : (const QDomLiteElementList)TempDoc.documentElement->elementsByTag(PresetTag))
        {
            m_ProgramNames[deviceName].append(e->attribute(PresetNameAttribute));
        }
    }
    static const QStringList programNames(const QString& deviceName)
    {
        if (!m_ProgramNames.contains(deviceName)) loadProgramNames(deviceName);
        return m_ProgramNames[deviceName];
    }
    static QString programMatches(QDomLiteElement* Parameters,const QString deviceName)
    {
        QDomLiteDocument TempDoc(PresetListTag,deviceName);
        QString xml = CPresets::macros(deviceName);
        if (xml.length()) TempDoc.fromString(xml);
        for (const QDomLiteElement* e : (const QDomLiteElementList)TempDoc.documentElement->elementsByTag(PresetTag))
        {
            const QString s=e->attribute(PresetNameAttribute);
            Parameters->setAttribute(PresetNameAttribute,s);
            if (Parameters->compare(e)) return s;
        }
        return QString();
    }

private:
    static QMap<QString,QStringList> m_ProgramNames;
};

#endif // CPROGRAMBANK_H
