#include "corebuffer.h"
//#include <QApplication>
//#include <QMessageBox>
//#include <QtConcurrent/QtConcurrent>

#ifdef __MINIAUDIO__

void CCoreMainBuffers::data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    // In playback mode copy data to pOutput. In capture mode read data from pInput. In full-duplex mode, both
    // pOutput and pInput will be valid and you can move data from pInput into pOutput. Never process more than
    // frameCount frames.
    (static_cast<CCoreMainBuffers*>(pDevice->pUserData)->MainAudioLoop(pOutput,(void*)pInput,frameCount));
}

#else

int CCoreMainBuffers::AudioCallback( void *outputBuffer, void *inputBuffer, uint nBufferFrames, double /*streamTime*/, RtAudioStreamStatus /*status*/, void *userData )
{
    (static_cast<CCoreMainBuffers*>(userData)->MainAudioLoop(outputBuffer, inputBuffer, nBufferFrames));
    return 0;
}

void CCoreMainBuffers::AudioErrorCallback(RtAudioErrorType type, const std::string &errorText)
{
    QString txt(errorText.c_str());
    qDebug() << type << txt;
    //if (txt == "RtApiCore::closeStream(): device still running, calling AudioDeviceStop!")
    //if (txt == "RtApiCore: the stream device was disconnected (and closed)!")
    //{
        //deviceLost = true;
    //}
}

#endif

void CCoreMainBuffers::MainAudioLoop(void* OutBuffer, void* InBuffer, const uint BufferSize)
{
    if (BufferState==Ready)
    {
        BufferState = BufferStates(BufferState | Working);
        float* outBufferPointer=static_cast<float*>(OutBuffer);
        float* inBufferPointer=static_cast<float*>(InBuffer);
        for (uint i=0; i<BufferSize; i++)
        {
            if (TickCount >= presets.ModulationRate)
            {
                if (m_Playing)
                {
                    mSecCount.skipBuffer();
                    if (mSecCount.currentSample() > m_Samples) m_Playing=false;
                }
                //if (m_TickerDevice) m_TickerDevice->tick(); //Tick All Devices!!!
                IDevice::tick();
                ParseMidi(FetchP(jnMIDIOut));
                OutChannelBuffer=FetchAStereo(jnOut);
                if (OutChannelBuffer->isValid()) {
                    OutChannelBuffer->peakStereoBuffer(&PeakL,&PeakR,outputVol,outputVol);
                }
                else {
                    OutChannelBuffer = &m_NullBufferStereo;
                }
                if (m_Recording) WaveFile.pushBuffer(OutChannelBuffer->data(),OutChannelBuffer->size());
                TickCount=0;
            }
#ifdef __MINIAUDIO__
            if (*m_InDriverID.coreaudio != 0) InChannelBuffer->setAtFromInterleaved(TickCount,inBufferPointer);
#else
            if (m_InDriverID > -1) InChannelBuffer->setAtFromInterleaved(TickCount,inBufferPointer);
#endif
            OutChannelBuffer->interleaveAt(TickCount++,outBufferPointer,outputVol);
        }
        BufferState = BufferStates(BufferState & (!Working));
    }
    else if (BufferState==Starting)
    {
        if (m_Startcounter>presets.SampleRate) BufferState=Ready;
        m_Startcounter+=BufferSize/2;
    }
}

CCoreMainBuffers::CCoreMainBuffers()
{
    caffeine.setReason("Real Time Audio Processing");
    //m_Ticker=nullptr;
    TickCount=0;

    PeakL=0;
    PeakR=0;

    BufferState=Stopped;

    outputVol=1;
    m_Recording=false;

    ActivityCount=0;
    qDebug() << "accessing presets";
    ActivityLimit=presets.SampleRate*30;

    m_Startcounter=0;
    m_Samples=0;
    //m_Playing=false;

    scanDrivers();

#ifdef __MINIAUDIO__
    miniAudio = new ma_device();
#else
    m_Audio = new RtAudio();
    m_Audio->setErrorCallback(&AudioErrorCallback);
    m_Audio->showWarnings(true);
#endif

}

bool CCoreMainBuffers::driverCheck()
{
#ifdef __MINIAUDIO__
    if (miniAudio->state.value == ma_device_state_started) return true;
#else
    if (m_Audio->isStreamRunning()) return true;
#endif

    /*
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning,"Audio Buffer","The Audio Driver was disconnected, trying to start another one!");
    msgBox->addButton( "Ok", QMessageBox::YesRole );
    msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
    msgBox->setModal(false);
    msgBox->show();
*/
    //QFuture<void>f1 = QtConcurrent::run(nativeMessage,m_MainWindow,QString("Audio Buffer"),QString("The Audio Driver was disconnected, trying to start another one!"));
    //f1.waitForFinished();
    showNativeMessage("Audio Buffer","The Audio Driver was disconnected, trying to start another one!");

    finish();

#ifdef __MINIAUDIO__
    delete miniAudio;
    miniAudio = new ma_device();
#else
    delete m_Audio;
    m_Audio = new RtAudio();
    m_Audio->setErrorCallback(&AudioErrorCallback);
    m_Audio->showWarnings(true);
#endif

    scanDrivers();

    qDebug() << "Create main buffer" << CPresets::presets().BufferSize << CPresets::presets().ModulationRate;
    createBuffer();
    return false;
}

void CCoreMainBuffers::scanDrivers()
{
    soundDriverNames(1,m_InDriverNames,m_InDriverIDs);
    soundDriverNames(0,m_OutDriverNames,m_OutDriverIDs);

    qDebug() << m_InDriverNames;
    qDebug() << m_OutDriverNames;

    if (!m_InDriverIDs.empty()) m_InDriverID = m_InDriverIDs.first(); // m_Audio.getDefaultInputDevice(); // first available device
    if (!m_OutDriverIDs.empty()) m_OutDriverID = m_OutDriverIDs.first(); // m_Audio.getDefaultOutputDevice(); // first available device
}

void CCoreMainBuffers::init(const int Index, QWidget* MainWindow)
{
    m_Name="This";
    IDevice::init(Index, MainWindow);

    OutAudio=addJackStereoOut(jnIn,"In");
    OutMIDI=addJackMIDIOut(jnMIDIIn,"MIDI In");

    InAudio=addJackStereoIn("Out");
    InMIDI=addJackMIDIIn("MIDI Out");

    InChannelBuffer=StereoBuffer(jnIn);
    OutChannelBuffer=&m_NullBufferStereo;
}
/*
float inline CCoreMainBuffers::TruncateVal(float Buf, float& Peak)
{
    Buf*=outputVol;
    Peak = qMax<float>(qAbs<float>(Buf),Peak);
    return Buf;
}
*/
void inline CCoreMainBuffers::ParseMidi(const CMIDIBuffer* MIDIBuffer)
{
    if (!MIDIBuffer) return;
#ifndef __NOMIDI__
    if (!MIDIBuffer->isEmpty())
    {
        const CMIDIEventList l = MIDIBuffer->eventList();
        for (uint i = 0; i < l.size(); i++)
        {
            const CMIDIEvent* e = l[i];
            m_MidiOut.sendMessage(e->memPtr(),e->memSize());
        }
    }
#endif
}

CCoreMainBuffers::~CCoreMainBuffers()
{
#ifdef __MINIAUDIO__
    delete miniAudio;
#else
    delete m_Audio;
#endif
}

void CCoreMainBuffers::wait()
{
    while (BufferState != Ready){}
}

void CCoreMainBuffers::startRecording()
{
    QMutexLocker locker(&mutex);
    m_Recording=true;
    WaveFile.startRecording(uint(OutChannelBuffer->channels()),presets.SampleRate);
}

void CCoreMainBuffers::stopRecording()
{
    QMutexLocker locker(&mutex);
    if (m_Recording)
    {
        m_Recording=false;
        WaveFile.finishRecording();
    }
}

bool CCoreMainBuffers::saveRecording(const QString &fileName)
{
    QMutexLocker locker(&mutex);
    return WaveFile.save(fileName);
}

void CCoreMainBuffers::render(const QString &fileName)
{
    QMutexLocker locker(&mutex);
    const BufferStates s = BufferState;
    BufferState = Working;
    CWaveFile f;
    CSampleCounter mSec;
    mSec.reset();
    f.startRecording(OutChannelBuffer->channels(),presets.SampleRate);
    const ulong64 samples = IDevice::samples();//m_TickerDevice->milliSeconds();
    const ulong64 maxSamples = samples + (presets.SampleRate* 60);
    IDevice::play(true);//m_TickerDevice->play(true);
    while (mSec.currentSample() < samples)
    {
        mSec.skipBuffer();
        //if (m_TickerDevice) m_TickerDevice->tick(); //Tick All Devices!!!
        IDevice::tick();
        const CStereoBuffer* b=FetchAStereo(jnOut);
        f.pushBuffer(b->data(),b->size());
    }
    //m_TickerDevice->pause();
    IDevice::pause();
    while (mSec.currentSample() < maxSamples)
    {
        mSec.skipBuffer();
        //if (m_TickerDevice) m_TickerDevice->tick(); //Tick All Devices!!!
        IDevice::tick();
        const CStereoBuffer* b=FetchAStereo(jnOut);
        f.pushBuffer(b->data(),b->size());
        float l = 0;
        float r = 0;
        b->peakStereoBuffer(&l,&r);
        if (l+r < 0.000001) break;
    }
    f.finishRecording();
    f.save(fileName);
    BufferState = s;
}

void CCoreMainBuffers::getPeak(float &L, float &R)
{
    L=PeakL;
    R=PeakR;
    PeakL=0;
    PeakR=0;
}

CMIDIBuffer* CCoreMainBuffers::getNextP(const int /*ProcIndex*/)
{
#ifndef __NOMIDI__
    std::vector<byte> message;
    m_MidiIn.getMessage(&message);
    MIDIBuffer.fromVector(&message);
#endif
    return &MIDIBuffer;
}

void CCoreMainBuffers::createBuffer()
{
    QMutexLocker locker(&mutex);
    uint BufferSize=presets.BufferSize;
#ifdef __MINIAUDIO__
    ma_device_config config;

    ma_result r = MA_SUCCESS;

    if (*m_InDriverID.coreaudio == 0) {
        config = ma_device_config_init(ma_device_type_playback);
        config.playback.pDeviceID = (const ma_device_id*)&m_OutDriverID;
        config.playback.format   = ma_format_f32;   // Set to ma_format_unknown to use the device's native format.
        config.playback.channels = 2;               // Set to 0 to use the device's native channel count.
    }
    else if (*m_OutDriverID.coreaudio == 0) {
        config = ma_device_config_init(ma_device_type_capture);
        config.capture.pDeviceID = (const ma_device_id*)&m_InDriverID;
        config.capture.format   = ma_format_f32;   // Set to ma_format_unknown to use the device's native format.
        config.capture.channels = 2;               // Set to 0 to use the device's native channel count.
    }
    else {
        config = ma_device_config_init(ma_device_type_duplex);
        config.playback.pDeviceID = (const ma_device_id*)&m_OutDriverID;
        config.capture.pDeviceID = (const ma_device_id*)&m_InDriverID;
        config.playback.format   = ma_format_f32;   // Set to ma_format_unknown to use the device's native format.
        config.capture.format    = ma_format_f32;
        config.playback.channels = 2;               // Set to 0 to use the device's native channel count.
        config.capture.channels = 2;
        qDebug() << m_OutDriverID.coreaudio << m_InDriverID.coreaudio;
    }

    config.periodSizeInFrames = BufferSize;
    config.sampleRate        = presets.SampleRate;           // Set to 0 to use the device's native sample rate.
    config.dataCallback      = data_callback;   // This function will be called when miniaudio needs more data.
    config.pUserData         = this;            // Can be accessed from the device object (device.pUserData).

    r = ma_device_init(NULL, &config, miniAudio);
    if (r != MA_SUCCESS) {
        qDebug() << r;  // Failed to initialize the device.
        exit(0);
    }

    // The device is sleeping by default so you'll need to start it manually.
    r = ma_device_start(miniAudio);
    if (r != MA_SUCCESS) {
        qDebug() << r;  // Failed to initialize the device.
        exit(0);
    }
#else
    RtAudio::StreamParameters iParams, oParams;
    iParams.deviceId = m_InDriverID;
    iParams.nChannels = 2;
    iParams.firstChannel = 0;
    oParams.deviceId = m_OutDriverID;
    oParams.nChannels = 2;
    oParams.firstChannel = 0;
    qDebug() << iParams.deviceId << iParams.nChannels << oParams.deviceId << oParams.nChannels;

    RtAudioErrorType e = RtAudioErrorType::RTAUDIO_NO_ERROR;
    if (m_InDriverID == -1)
    {
        e = m_Audio->openStream( &oParams, nullptr, RTAUDIO_FLOAT32, presets.SampleRate, &BufferSize, &AudioCallback, this, NULL);
    }
    else if (m_OutDriverID == -1)
    {
        e = m_Audio->openStream( nullptr, &iParams, RTAUDIO_FLOAT32, presets.SampleRate, &BufferSize, &AudioCallback, this, NULL);
    }
    else
    {
        e = m_Audio->openStream( &oParams, &iParams, RTAUDIO_FLOAT32, presets.SampleRate, &BufferSize, &AudioCallback, this, NULL);
    }
    if (e) {
        qDebug() << m_Audio->getErrorText().c_str();
        finish();
        exit( 0 );
    }
    e = m_Audio->startStream();
    if (e) {
        qDebug() << m_Audio->getErrorText().c_str();
        finish();
        exit( 0 );
    }
#endif
#ifndef __NOMIDI__
    try
    {
        if ( m_MidiOut.getPortCount() > 0 )
        {
            m_MidiOut.openPort(0);
        }
    }
    catch ( RtMidiError& e )
    {
        qDebug() << e.what();
    }
    m_MidiIn.ignoreTypes( true, true, true );
#endif
    m_Startcounter=0;
    BufferState=Starting;
}

void CCoreMainBuffers::panic()
{
    CMIDIBuffer b;
    for (byte j=0;j<16;j++) b.append(0xB0+j,0x7B);
    for (byte j=0;j<16;j++) b.append(0xB0+j,0x78);
    for (byte i=0;i<16;i++) {
        for (byte n=1;n<128;n++) b.append(0x80+i,n,0);
    }
    ParseMidi(&b);
    //std::vector<byte> message;
    //b.fillVector(&message);
    //m_MidiOut.sendMessage(&message);
    //Bn 7B 00 All notes off!
    //Bn 78 00 All sound off!
}

void CCoreMainBuffers::finish()
{
    qDebug() << "CCoreMainBuffers finish";
    wait();
    QMutexLocker locker(&mutex);
    BufferState=Stopped;
#ifdef __MINIAUDIO__
    if (miniAudio->state.value == ma_device_state_started) ma_device_stop(miniAudio);
    ma_device_uninit(miniAudio);
#else
    if (m_Audio->isStreamRunning()) m_Audio->stopStream();
    if (m_Audio->isStreamOpen()) m_Audio->closeStream();
#endif
#ifndef __NOMIDI__
    panic();
    m_MidiIn.closePort();
    m_MidiOut.closePort();
#endif
    qDebug() << "exit finish";
}

void CCoreMainBuffers::soundDriverNames(int Direction, QStringList &names, QList<driverIDtype> &ids)
{
    QMutexLocker locker(&mutex);
    names.clear();
    ids.clear();
#ifdef __MINIAUDIO__
    ma_context context;
    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        return;
    }

    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;
    if (ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) == MA_SUCCESS) {
        if (Direction == 0) {
            for (uint i = 0; i < playbackCount; i++) {
                if (pPlaybackInfos[i].nativeDataFormatCount == 0) {
                    names.append(pPlaybackInfos[i].name);
                    ids.append(pPlaybackInfos[i].id);
                }
                else
                {
                    for (uint j = 0; j < pPlaybackInfos[i].nativeDataFormatCount; j++) {
                        if ((pPlaybackInfos[i].nativeDataFormats[j].format == ma_format_f32) || pPlaybackInfos[i].nativeDataFormats[j].format == ma_format_unknown) {
                            if ((pPlaybackInfos[i].nativeDataFormats[j].sampleRate == presets.SampleRate) || (pPlaybackInfos[i].nativeDataFormats[j].sampleRate == 0)) {
                                if ((pPlaybackInfos[i].nativeDataFormats[j].channels == 2) || (pPlaybackInfos[i].nativeDataFormats[j].channels == 0)) {
                                    names.append(pPlaybackInfos[i].name);
                                    ids.append(pPlaybackInfos[i].id);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            for (uint i = 0; i < captureCount; i++) {
                if (pCaptureInfos[i].nativeDataFormatCount == 0) {
                    names.append(pCaptureInfos[i].name);
                    ids.append(pCaptureInfos[i].id);
                }
                else
                {
                    for (uint j = 0; j < pCaptureInfos[i].nativeDataFormatCount; j++) {
                        if ((pCaptureInfos[i].nativeDataFormats[j].format == ma_format_f32) || pCaptureInfos[i].nativeDataFormats[j].format == ma_format_unknown) {
                            if ((pCaptureInfos[i].nativeDataFormats[j].sampleRate == presets.SampleRate) || (pCaptureInfos[i].nativeDataFormats[j].sampleRate == 0)) {
                                if ((pCaptureInfos[i].nativeDataFormats[j].channels == 2) || (pCaptureInfos[i].nativeDataFormats[j].channels == 0)) {
                                    names.append(pCaptureInfos[i].name);
                                    ids.append(pCaptureInfos[i].id);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ma_context_uninit(&context);
#else
    RtAudio audio;
    for (const uint i : audio.getDeviceIds())
    {
        const RtAudio::DeviceInfo info(audio.getDeviceInfo(i));
        qDebug() << info.ID << QString::fromStdString(info.name) << info.nativeFormats << info.sampleRates << info.inputChannels << info.outputChannels;
        if (((info.outputChannels>=2) && (Direction==0)) || ((info.inputChannels>=2) && (Direction==1)))
        {
            bool Match1=false;
            for (const uint& s : info.sampleRates)
            {
                if (s==presets.SampleRate)
                {
                    Match1=true;
                    break;
                }
            }
            if (Match1)
            {
                if (info.nativeFormats & RTAUDIO_FLOAT32)
                {
                    QString DrvName = QString::fromStdString(info.name);
                    QString Ext;
                    int ExistCount=0;
                    while (names.contains(DrvName+Ext))
                    {
                        ExistCount++;
                        Ext=" (" + QString::number(ExistCount)+ ")";
                    }
                    names.append(DrvName+Ext);
                    ids.append(i);
                }
            }
        }
    }
#endif
}

const QStringList CCoreMainBuffers::inDriverNames()
{
    return m_InDriverNames;
}

const QStringList CCoreMainBuffers::outDriverNames()
{
    return m_OutDriverNames;
}

void CCoreMainBuffers::setInDriver(const QString &driverName)
{
    const int index=m_InDriverNames.indexOf(driverName);
    if (index > -1)
    {
#ifdef __MINIAUDIO__
        if (m_InDriverID.coreaudio != m_InDriverIDs[index].coreaudio)
#else
        if (m_InDriverID != m_InDriverIDs[index])
#endif
        {
            finish();
            m_InDriverID = m_InDriverIDs[index];
            createBuffer();
        }
    }
}

void CCoreMainBuffers::setOutDriver(const QString &driverName)
{
    const int index=m_OutDriverNames.indexOf(driverName);
    if (index > -1)
    {
#ifdef __MINIAUDIO__
        if (m_OutDriverID.coreaudio != m_OutDriverIDs[index].coreaudio)
#else
        if (m_OutDriverID != m_OutDriverIDs[index])
#endif
        {
            finish();
            m_OutDriverID = m_OutDriverIDs[index];
            createBuffer();
        }
    }
}

void CCoreMainBuffers::setDrivers(const QString &inDriverName, const QString &outDriverName)
{
    const int inIndex=m_InDriverNames.indexOf(inDriverName);
    const int outIndex=m_InDriverNames.indexOf(outDriverName);
    if ((inIndex > -1) && (outIndex > -1))
    {
#ifdef __MINIAUDIO__
        if ((m_InDriverID.coreaudio != m_InDriverIDs[inIndex].coreaudio) || (m_OutDriverID.coreaudio != m_OutDriverIDs[outIndex].coreaudio))
#else
        if ((m_InDriverID != m_InDriverIDs[inIndex]) || (m_OutDriverID != m_OutDriverIDs[outIndex]))
#endif
        {
            finish();
            m_InDriverID = m_InDriverIDs[inIndex];
            m_OutDriverID = m_OutDriverIDs[outIndex];
            createBuffer();
        }
    }
}

const QString CCoreMainBuffers::inDriverName()
{
#ifdef __MINIAUDIO__
    int inIndex = -1;
    for (int i = 0; i < m_InDriverIDs.size(); i++) {
        if (m_InDriverIDs[i].coreaudio == m_InDriverID.coreaudio) inIndex = i;
    }
#else
    const int inIndex = m_InDriverIDs.indexOf(m_InDriverID);
#endif
    return ((inIndex < 0) || (inIndex >= m_InDriverNames.size())) ? QString() : m_InDriverNames[inIndex];
}

const QString CCoreMainBuffers::outDriverName()
{
#ifdef __MINIAUDIO__
    int outIndex = -1;
    for (int i = 0; i < m_OutDriverIDs.size(); i++) {
        if (m_OutDriverIDs[i].coreaudio == m_OutDriverID.coreaudio) outIndex = i;
    }
#else
    const int outIndex = m_InDriverIDs.indexOf(m_OutDriverID);
#endif
    return ((outIndex < 0) || (outIndex >= m_OutDriverNames.size())) ? QString() : m_OutDriverNames[outIndex];
}

const QList<uint> CCoreMainBuffers::sampleRates()
{
    QMutexLocker locker(&mutex);
    QList<uint> li;
    QList<uint> lo;
#ifdef __MINIAUDIO__
    /*
    ma_context context;
    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        return li;
    }

    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;
    if (ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        for (uint i = 0; i < playbackCount; i++) {
            for (uint j = 0; j < pPlaybackInfos->nativeDataFormatCount; j++) {
                if (! lo.contains(pPlaybackInfos[i].nativeDataFormats[j].sampleRate)) lo.append(pPlaybackInfos[i].nativeDataFormats[j].sampleRate);
            }
        }
        for (uint i = 0; i < captureCount; i++) {
            for (uint j = 0; j < pCaptureInfos->nativeDataFormatCount; j++) {
                if (! lo.contains(pCaptureInfos[i].nativeDataFormats[j].sampleRate)) lo.append(pCaptureInfos[i].nativeDataFormats[j].sampleRate);
            }
        }
    }
    ma_context_uninit(&context);
    */
    li = {48000,44100,32000,24000,22050,88200,96000,176400,192000,16000,11025,8000,352800,384000};
    lo = li;
#else
    RtAudio audio;
    for (const uint i : audio.getDeviceIds())
    {
        const RtAudio::DeviceInfo info(audio.getDeviceInfo(i));
        if (info.outputChannels>=2)
        {
            for (const uint& s : info.sampleRates) if (!lo.contains(s)) lo.append(s);
        }

        if (info.inputChannels>=2)
        {
            for (const uint& s : info.sampleRates) if (!li.contains(s)) li.append(s);
        }
    }
#endif
    qDebug() << li << lo;
    QList<uint> l;
    for (const uint& i : li) if (lo.contains(i)) l.append(i);
    std::sort(l.begin(),l.end());
    return l;
}

void CCoreMainBuffers::play(const bool FromStart)
{
    m_Samples=IDevice::samples();
    //IDevice::pause();
    if (FromStart) mSecCount.reset();
    IDevice::play(FromStart); //m_TickerDevice->play(FromStart);
    //m_Playing=true;
}

void CCoreMainBuffers::pause()
{
    //m_MilliSeconds=IDevice::milliSeconds();
    //m_Playing=false;
    //m_TickerDevice->pause();
    IDevice::pause();
}

void CCoreMainBuffers::skip(const ulong64 samples)
{
    //IDevice::pause();
    m_Samples=IDevice::samples();
    mSecCount.reset();
    /*
    if (mSecs==0)
    {
        IDevice::play(true);
    }
    else
    {
*/
    mSecCount.skip(samples);
        //m_TickerDevice->skip(samples);
        IDevice::skip(samples);
        //m_Playing=true;
    //}
}
