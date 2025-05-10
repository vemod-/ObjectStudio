#ifndef CAUPLOADER_H
#define CAUPLOADER_H

#include "cwavelanes.h"

class CAupLoader {
public:
    CAupLoader(CWaveLanes* lanes, const QString& path) {
        m_Lanes = lanes;
        const QDomLiteDocument doc(path);
        const QString projname = doc.documentElement->attribute("projname");
        const QString OrigPath=QFileInfo(path).absolutePath() + "/" + projname;
        const QDomLiteElementList tracks = (const QDomLiteElementList)doc.documentElement->elementsByTag("wavetrack");
        qDebug() << projname << OrigPath << tracks.size();
        //Time=0;
        //m_MilliSeconds=0;
        int TrackIndex = 0;
        for (const QDomLiteElement* track : tracks)
        {
            if (m_Lanes->lanes.size() <= TrackIndex) m_Lanes->AddLaneInternal();
            m_Lane = m_Lanes->lanes[TrackIndex];
            loadTrack(track,OrigPath);
            //auto AT=new AudacityTrack();
            //AT->loadTrack(track,OrigPath);
            //m_MilliSeconds=qMax<ulong>(AT->milliSeconds(),m_MilliSeconds);
            //Tracks.append(AT);
            TrackIndex++;
        }
    }
    void loadTrack(const QDomLiteElement* xml, const QString& ProjectPath)
    {
        if (!xml) return;
        if (xml->matches("wavetrack"))
        {
            int Channel = xml->attributeValueInt("channel");
            //QString Name = xml->attribute("name");
            bool Linked = xml->attributeValueBool("linked");
            TrackOffset=xml->attributeValueLDouble("offset");
            //Mute = StrToInt("0"+AnsiString(xmldoc->GetAttribute("mute")));
            //Solo = StrToInt("0"+AnsiString(xmldoc->GetAttribute("solo")));
            TrackRate = xml->attributeValueInt("rate");
            float Gain = float(xml->attributeValue("gain"));
            float Pan = float(xml->attributeValue("pan"));

            //float FactorL = (Pan > 0) ? Gain * (1.f - Pan) : Gain;
            //float FactorR = (Pan < 0) ? Gain * (1.f + Pan) : Gain;
            m_Lane->parameter(0)->setPercentValue(Gain);
            m_Lane->parameter(1)->setPercentValue(Pan);
            if (Channel == 1) m_Lane->parameter(1)->setValue(100);
            if (Linked) m_Lane->parameter(1)->setValue(-100);

            for (const QDomLiteElement* clip : (const QDomLiteElementList)xml->elementsByTag("waveclip"))
            {
                //auto AC=new AudacityClip();
                loadClip(clip,ProjectPath,TrackRate);
                //Clips.append(AC);
            }
            for(const QDomLiteElement* clip : (const QDomLiteElementList)xml->elementsByTag("sequence"))
            {
                //auto AC=new AudacityClip();
                loadSequence(clip,ProjectPath,TrackRate);
                //Clips.append(AC);
            }
        }
    }
    void loadClip(const QDomLiteElement* xml, const QString& ProjectPath,int RateOverride)
    {
        if (!xml) return;
        if (xml->matches("waveclip"))
        {
            ClipOffset=xml->attributeValueLDouble("offset");
            if (const QDomLiteElement* sequenze = xml->elementByTag("sequence"))
            {
                for (const QDomLiteElement* block : (const QDomLiteElementList)sequenze->elementsByTag("waveblock"))
                {
                    const ulong Start=block->attributeValueULong("start");
                    if (const QDomLiteElement* file = block->elementByTag("simpleblockfile"))
                    {
                        const QString Filename=file->attribute("filename");
                        QString FilePath=ProjectPath + "/" + Filename;
                        if (QFileInfo::exists(FilePath))
                        {
                            AddBlock(FilePath,Start,RateOverride);
                        }
                        else
                        {
                            const QString Path1=Filename.mid(0,3);
                            const QString Path2="d" + Filename.mid(3,2);
                            FilePath=ProjectPath + "/" + Path1 + "/" + Path2 + "/" + Filename;
                            AddBlock(FilePath,Start,RateOverride);
                        }
                    }
                    if (const QDomLiteElement* file = block->elementByTag("pcmaliasblockfile"))
                    {
                        const QString Filename=file->attribute("aliasfile");
                        const uint AliasChannel=file->attributeValueUInt("aliaschannel");
                        const ulong AliasStart=file->attributeValueULong("aliasstart");
                        AddAliasBlock(Filename,Start,AliasChannel,AliasStart);
                    }
                }
            }
        }
    }

    void loadSequence(const QDomLiteElement* xml, const QString& ProjectPath,int RateOverride)
    {
        if (!xml) return;
        if (xml->matches("sequence"))
        {
            ClipOffset=0;

            for (const QDomLiteElement* block : (const QDomLiteElementList)xml->elementsByTag("waveblock"))
            {
                const ulong Start=block->attributeValueULong("start");
                if (const QDomLiteElement* file = block->elementByTag("simpleblockfile"))
                {
                    const QString Filename=file->attribute("filename");
                    QString FilePath=ProjectPath + "/" + Filename;
                    if (QFileInfo::exists(FilePath))
                    {
                        AddBlock(FilePath,Start,RateOverride);
                    }
                    else
                    {
                        //AnsiString Path1=Filename.SubString(1,3);
                        //AnsiString Path2="d" + Filename.SubString(4,2);
                        const QString Path1=Filename.mid(1,3);
                        const QString Path2="d" + Filename.mid(4,2);
                        FilePath=ProjectPath + "/" + Path1 + "/" + Path2 + "/" + Filename;
                        AddBlock(FilePath,Start,RateOverride);
                    }
                }
                if (const QDomLiteElement* file = block->elementByTag("pcmaliasblockfile"))
                {
                    const QString Filename=file->attribute("aliasfile");
                    const uint AliasChannel=file->attributeValueUInt("aliaschannel");
                    const ulong AliasStart=file->attributeValueULong("aliasstart");
                    AddAliasBlock(Filename,Start,AliasChannel,AliasStart);
                }
            }
        }
    }
    void AddBlock(const QString& Filename,ulong Start,int RateOverride)
    {
        const QString FN=CPresets::resolveFilename(Filename);
        if (!QFileInfo::exists(FN)) return;
        //auto AB=new AudacityBlock();
        loadBlock(FN,Start,0,0,RateOverride); //Blocks.append(AB);
        //qDebug() << "AddBlock" << Start << AB->Start << RateOverride;
    }

    void AddAliasBlock(const QString& Filename,ulong Start,uint Channel,ulong AliasStart)
    {
        const QString FN=CPresets::resolveFilename(Filename);
        if (!QFileInfo::exists(FN)) return;
        //auto AB=new AudacityBlock();
        loadBlock(FN,Start,Channel,AliasStart,0); //Blocks.append(AB);
        //qDebug() << "AddAliasBlock" << Start << AB->Start << AliasStart;
    }
    bool loadBlock(const QString& Filename, ulong StartPtr, uint /*Channels*/, ulong AliasPointer,int /*RateOverride*/)
    {
        if (CWaveTrack* wa = new CWaveTrack(Filename, StartPtr))
        {
            //ulong Start=StartPtr;
            //ulong AliasStart=AliasPointer;
            wa->loopParameters.Start = AliasPointer;
            //int Rate=RateOverride;
            qDebug() << wa->waveGenerator.origRate() << CPresets::presets().SampleRate;
            if (wa->waveGenerator.origRate() != CPresets::presets().SampleRate)
            {
                const double oldSampleRate=wa->waveGenerator.origRate();
                    const double newSampleRate=CPresets::presets().SampleRate;
                    const ldouble rateFactor=newSampleRate/oldSampleRate;
                    wa->start *= rateFactor;
                    wa->loopParameters.Start *= rateFactor;
                    //wa->loopParameters.End *= rateFactor;
                    //wa->loopParameters.Speed = rateFactor;//oldSampleRate/newSampleRate;
            }
            //if (Channels < wa->waveGenerator.channels()) Channel=Channels;
            m_Lane->tracks.append(wa);
            //Reset();
            return true;
        }
        return false;
    }

private:
    //ldouble Time = 0;
    //ulong m_MilliSeconds = 0;
    CWaveLanes* m_Lanes;
    CWaveLane* m_Lane;
    //Track
    ldouble TrackOffset = 0;
    int TrackRate = 0;
    //Clip
    ldouble ClipOffset = 0;

};

#endif // CAUPLOADER_H
