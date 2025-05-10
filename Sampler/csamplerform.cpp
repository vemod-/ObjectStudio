#include "csamplerform.h"
#include "ui_csamplerform.h"
#include "cpitchtextconvert.h"
#include <QFileDialog>

CSamplerForm::CSamplerForm(IDevice* Device, QWidget *parent) :
    CSoftSynthsForm(Device,true,parent),
    ui(new Ui::CSamplerForm)
{
    ui->setupUi(this);
    menu=new QMenu(this);
    menu->addAction("Import sfz",this,SLOT(loadSfz()));
}

CSamplerForm::~CSamplerForm()
{
    delete ui;
}

void CSamplerForm::mousePressEvent(QMouseEvent* e)
{
    if (e->button() & Qt::RightButton)
    {
        menu->popup(QCursor::pos());
    }
    CSoftSynthsForm::mousePressEvent(e);
}

void CSamplerForm::loadSfz()
{
    QFileDialog d(this);
    if (d.exec())
    {
        QMutexLocker locker(&mutex);
        const QString p=d.selectedFiles().first();
        if (!p.isEmpty()) ConvertSfz(p);
    }
}

void CSamplerForm::Init(CSamplerDevice *Device)
{
    m_Sampler=Device;
    ui->WaveLayers->Init(Device);
    ui->WaveLayers->Update();
}

void CSamplerForm::unserializeCustom(const QDomLiteElement* xml)
{
    if (!xml) return;
    QMutexLocker locker(&mutex);
    if (const QDomLiteElement* Layers = xml->elementByTag("Layers")) m_Sampler->unserialize(Layers->elementByTag("Custom"));
    ui->WaveLayers->Update();
}

void CSamplerForm::serializeCustom(QDomLiteElement* xml) const
{
    m_Sampler->serialize(xml->appendChild("Layers")->appendChild("Custom"));
}

void CSamplerForm::ReleaseLoop()
{
    ui->WaveLayers->ReleaseLoop();
}

QString lineVal(const QString& inp, const QString& s)
{
    if (inp.startsWith(s,Qt::CaseInsensitive)) return inp.split('=').last().trimmed();
    return QString();
}

void CSamplerForm::ConvertSfz(const QString& filepath)
{
    enum ReadModes {
        none,
        control,
        global,
        group,
        region
    };
    QMutexLocker locker(&mutex);
    QDomLiteElement xml("Custom");
    QFile inputFile(filepath);
    QString path=QFileInfo(filepath).absolutePath();
    QString defaultPath;
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QDomLiteElement* CurrentLayer=nullptr;
        QDomLiteElement* CurrentRange=nullptr;
        ReadModes readMode=none;
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.startsWith("//")) {}
            else if (line.isEmpty()) {}
            else
            {
                QStringList l;
                for (const QString& s : (const QStringList)line.split(' '))
                {
                    if (s.contains('='))
                    {
                        l.append(s);
                    }
                    else
                    {
                        if (l.isEmpty())
                        {
                            l.append(s);
                        }
                        else
                        {
                            l[l.size()-1]+=' '+s;
                        }
                    }
                }
                for (const QString& s : std::as_const(l))
                {
                    if (s.isEmpty()) {}
                    else
                    {
                        if (s.startsWith("<control>"))
                        {
                            readMode=control;
                        }
                        else if (s.startsWith("<global>"))
                        {
                            readMode=global;
                        }
                        else if (s.startsWith("<group>"))
                        {
                            if (readMode==group)
                            {
                                if (CurrentRange)
                                {
                                    CurrentRange=CurrentLayer->elementByTagCreate("Custom")->appendClone(CurrentRange);
                                }
                            }
                            CurrentLayer=xml.elementByPathCreate("Layers/Custom")->appendChild("Layer");
                            readMode=group;
                        }
                        else if (s.startsWith("<region>"))
                        {
                            if (!CurrentLayer)
                            {
                                CurrentLayer=xml.elementByPathCreate("Layers/Custom")->appendChild("Layer");
                                //readMode=group;
                            }
                            CurrentRange=CurrentLayer->elementByTagCreate("Custom")->appendChild("Range");
                            readMode=region;
                        }
                        if (readMode==control)
                        {
                            if (!lineVal(s,"default_path").isEmpty())
                            {
                                defaultPath=lineVal(s,"default_path").trimmed().replace('\\','/');
                            }
                        }
                        else if (readMode==global)
                        {
                            if (!lineVal(s,"ampeg_delay").isEmpty())
                            {
                                xml.elementByPathCreate("Layers/Custom/ADSR/Custom")->setAttribute("Delay",lineVal(s,"ampeg_delay").toDouble()*1000);
                            }
                            if (!lineVal(s,"ampeg_attack").isEmpty())
                            {
                                xml.elementByPathCreate("Layers/Custom/ADSR/Custom")->setAttribute("Attack",lineVal(s,"ampeg_attack").toDouble()*1000);
                            }
                            if (!lineVal(s,"ampeg_hold").isEmpty())
                            {
                                xml.elementByPathCreate("Layers/Custom/ADSR/Custom")->setAttribute("Hold",lineVal(s,"ampeg_hold").toDouble()*1000);
                            }
                            if (!lineVal(s,"ampeg_decay").isEmpty())
                            {
                                xml.elementByPathCreate("Layers/Custom/ADSR/Custom")->setAttribute("Decay",lineVal(s,"ampeg_decay").toDouble()*1000);
                            }
                            if (!lineVal(s,"ampeg_sustain").isEmpty())
                            {
                                xml.elementByPathCreate("Layers/Custom/ADSR/Custom")->setAttribute("Sustain",lineVal(s,"ampeg_sustain"));
                            }
                            if (!lineVal(s,"ampeg_release").isEmpty())
                            {
                                xml.elementByPathCreate("Layers/Custom/ADSR/Custom")->setAttribute("Release",lineVal(s,"ampeg_release").toDouble()*1000);
                            }
                        }
                        else if (readMode==group)
                        {
                            if (!lineVal(s,"tune").isEmpty())
                            {
                                CurrentLayer->elementByTagCreate("Custom")->setAttribute("Tune",lineVal(s,"tune"));
                            }
                            else if (!lineVal(s,"lovel").isEmpty())
                            {
                                int p = CPitchTextConvert::text2Pitch(lineVal(s,"lovel"));
                                CurrentLayer->elementByTagCreate("Custom")->setAttribute("LowerTop",p);
                                CurrentLayer->elementByTagCreate("Custom")->setAttribute("LowerZero",p);
                            }
                            else if (!lineVal(s,"hivel").isEmpty())
                            {
                                int p = CPitchTextConvert::text2Pitch(lineVal(s,"hivel"));
                                CurrentLayer->elementByTagCreate("Custom")->setAttribute("UpperTop",p);
                                CurrentLayer->elementByTagCreate("Custom")->setAttribute("UpperZero",p);
                            }
                        }
                        else if (readMode==region)
                        {
                            if (!lineVal(s,"sample").isEmpty())
                            {
                                QString p=lineVal(s,"sample");
                                p.replace('\\','/');
                                CurrentRange->setAttribute("WaveFile",QString(path+"/"+defaultPath+p));
                            }
                            else if (!lineVal(s,"lokey").isEmpty())
                            {
                                int p = CPitchTextConvert::text2Pitch(lineVal(s,"lokey"));
                                CurrentRange->setAttribute("LowerTop",p);
                                CurrentRange->setAttribute("LowerZero",p);
                            }
                            else if (!lineVal(s,"hikey").isEmpty())
                            {
                                int p = CPitchTextConvert::text2Pitch(lineVal(s,"hikey"));
                                CurrentRange->setAttribute("UpperTop",p);
                                CurrentRange->setAttribute("UpperZero",p);
                            }
                            else if (!lineVal(s,"pitch_keycenter").isEmpty())
                            {
                                int p = CPitchTextConvert::text2Pitch(lineVal(s,"pitch_keycenter"));
                                CurrentRange->setAttribute("MIDINote",p);
                            }
                            else if (!lineVal(s,"offset").isEmpty())
                            {
                                CurrentRange->setAttribute("Start",lineVal(s,"offset"));
                            }
                            else if (!lineVal(s,"end").isEmpty())
                            {
                                CurrentRange->setAttribute("End",lineVal(s,"end"));
                            }
                            else if (!lineVal(s,"loop_start").isEmpty())
                            {
                                CurrentRange->setAttribute("LoopStart",lineVal(s,"loop_start"));
                            }
                            else if (!lineVal(s,"loop_end").isEmpty())
                            {
                                CurrentRange->setAttribute("LoopEnd",lineVal(s,"loop_end"));
                            }
                            else if (!lineVal(s,"tune").isEmpty())
                            {
                                CurrentRange->setAttribute("Tune",lineVal(s,"tune"));
                            }
                            else if (!lineVal(s,"lovel").isEmpty())
                            {
                                CurrentRange->setAttribute("LoVel",lineVal(s,"lovel"));
                            }
                            else if (!lineVal(s,"hivel").isEmpty())
                            {
                                CurrentRange->setAttribute("HiVel",lineVal(s,"hivel"));
                            }
                            else if (!lineVal(s,"volume").isEmpty())
                            {
                                CurrentRange->setAttribute("Volume",lround(dB2lin(lineVal(s,"volume").toDouble())*100));
                            }
                        }
                    }
                }
            }
        }
        if (readMode==group)
        {
            if (CurrentRange)
            {
                CurrentLayer->elementByTagCreate("Custom")->appendClone(CurrentRange);
            }
        }
        for (QDomLiteElement* Layer : (const QDomLiteElementList)xml.elementByPathCreate("Layers/Custom")->elementsByTag("Layer"))
        {
            QDomLiteElement Temp("Custom");
            for (QDomLiteElement* Range : (const QDomLiteElementList)Layer->elementByTagCreate("Custom")->elementsByTag("Range"))
            {
                if (Range->attributeExists("LoVel") || Range->attributeExists("HiVel"))
                {
                    bool match=false;
                    for (QDomLiteElement* l : std::as_const(Temp.childElements))
                    {
                        if (l->elementByTagCreate("Custom")->attributeValueInt("LowerTop",0)==Range->attributeValueInt("LoVel",0))
                        {
                            if (l->elementByTag("Custom")->attributeValueInt("UpperTop",127)==Range->attributeValueInt("HiVel",127))
                            {
                                l->elementByTag("Custom")->appendClone(Range);
                                match=true;
                                break;
                            }
                        }
                    }
                    if (!match)
                    {
                        QDomLiteElement* l=Temp.appendChild("Layer")->appendChild("Custom");
                        l->setAttributesString(Layer->elementByTagCreate("Custom")->attributesString());
                        l->setAttribute("LowerTop",Range->attributeValueInt("LoVel",0));
                        l->setAttribute("LowerZero",Range->attributeValueInt("LoVel",0));
                        l->setAttribute("UpperTop",Range->attributeValueInt("HiVel",127));
                        l->setAttribute("UpperZero",Range->attributeValueInt("HiVel",127));
                        l->appendClone(Range);
                    }
                    delete Layer->elementByTag("Custom")->takeChild(Range);
                }
            }
            if (Layer->elementByTag("Custom")->elementsByTag("Range").empty()) delete xml.elementByPath("Layers/Custom")->takeChild(Layer);
            QDomLiteElementList tempLayers=Temp.elementsByTag("Layer");
            for (QDomLiteElement* e : std::as_const(tempLayers))//for (int i=0;i<tempLayers.size();i++)
            {
                xml.elementByPathCreate("Layers/Custom")->appendClone(e);
            }
        }
        inputFile.close();
        unserializeCustom(&xml);
    }
}
