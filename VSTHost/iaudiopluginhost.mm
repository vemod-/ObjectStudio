#include "iaudiopluginhost.h"

//#ifndef __x86_64
//#include <Carbon/Carbon.h>
//#endif
//#import <Cocoa/Cocoa.h>

IAudioPlugInHost::IAudioPlugInHost(QWidget *parent):CMacWindow(parent),IMIDIParser()
{
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    m_Samplerate=presets.SampleRate;
    m_Buffersize=presets.ModulationRate;
}

const QPixmap* IAudioPlugInHost::picture()
{
    if (!viewId()) return nullptr;
    if (!isVisible()) return new QPixmap(backPix);
    backPix = grab();//CMacWindow::Grab();
    return new QPixmap(backPix);
}

void IAudioPlugInHost::resizeEvent(QResizeEvent *e)
{
    //qDebug() << "IAudioPlugInHost::resizeEvent";
    CMacWindow::resizeEvent(e);
    backPix = grab();//CMacWindow::Grab();
}

void IAudioPlugInHost::showEvent(QShowEvent *e)
{
    //qDebug() << "IAudioPlugInHost::showEvent";
    CMacWindow::showEvent(e);
    backPix = grab();//CMacWindow::Grab();
}
