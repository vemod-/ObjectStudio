#import <AVFoundation/AVFoundation.h>
#include <deque>
#import "avfoundation_wrapper.h"
#include <QDebug>

bool avf_read_audio(const QString& path,
                    std::vector<std::vector<float>> &outData,
                    double &outSampleRate,
                    int &outChannels)
{
    @autoreleasepool {
        NSURL *url = [NSURL fileURLWithPath:path.toNSString()];
        AVAudioFile *file = [[AVAudioFile alloc] initForReading:url error:nil];
        if (!file) return false;

        AVAudioFormat *format = file.processingFormat;
        outSampleRate = format.sampleRate;
        outChannels = format.channelCount;

        AVAudioPCMBuffer *buffer = [[AVAudioPCMBuffer alloc]
            initWithPCMFormat:format
            frameCapacity:(AVAudioFrameCount)file.length];
        if (buffer == nil) return false;
        [file readIntoBuffer:buffer error:nil];

        outData.resize(outChannels);
        for (int c = 0; c < outChannels; ++c)
            outData[c].resize(buffer.frameLength);

        //float **channels = buffer.floatChannelData;
        float **channels = (float **)buffer.floatChannelData;
        if (channels == nil) return false;
        for (int c = 0; c < outChannels; ++c) {
            memcpy(outData[c].data(), channels[c], buffer.frameLength * sizeof(float));
        }

        return true;
    }
}

bool avf_write_audio(const QString& path,
                     const std::vector<std::vector<float>> &inData,
                     double sampleRate)
{
    @autoreleasepool {
        NSURL *url = [NSURL fileURLWithPath:path.toNSString()];

        int channels = (int)inData.size();
        int frames = (int)inData[0].size();

        AVAudioFormat *format = [[AVAudioFormat alloc]
            initStandardFormatWithSampleRate:sampleRate
            channels:channels];

        AVAudioFile *file = [[AVAudioFile alloc]
            initForWriting:url
            settings:format.settings
            commonFormat:AVAudioPCMFormatFloat32
            interleaved:NO
            error:nil];

        AVAudioPCMBuffer *buffer = [[AVAudioPCMBuffer alloc]
            initWithPCMFormat:format
            frameCapacity:frames];
        buffer.frameLength = frames;

        //float **channelsData = buffer.floatChannelData;
        float **channelsData = (float **)buffer.floatChannelData;
        if (channelsData == nil) return false;
        for (int c = 0; c < channels; ++c) {
            memcpy(channelsData[c], inData[c].data(), frames * sizeof(float));
        }

        [file writeFromBuffer:buffer error:nil];
        return true;
    }
}

bool avf_has_video(const QString& path) {
        @autoreleasepool {
            NSURL *url = [NSURL fileURLWithPath:path.toNSString()];
            AVAsset *asset = [AVAsset assetWithURL:url];
            return [[asset tracksWithMediaType:AVMediaTypeVideo] count] > 0;
        }
}

bool avf_is_valid(const QString& path){
        @autoreleasepool {
            NSURL *url = [NSURL fileURLWithPath:path.toNSString()];
            AVURLAsset *asset = [AVURLAsset URLAssetWithURL:url options:nil];
            NSArray *audioTracks = [asset tracksWithMediaType:AVMediaTypeAudio];
            return ([audioTracks count] > 0);
        }
}

QImage avf_extract_fullframe(const QString& path,
                           double seconds)
{
    std::vector<uint8_t> rgba;
    int width = 0;
    int height = 0;
    @autoreleasepool
    {
        NSURL *url = [NSURL fileURLWithPath:path.toNSString()];
        AVAsset* asset = [AVAsset assetWithURL:url];
        if (!asset) return QImage();

        AVAssetImageGenerator* generator = [[AVAssetImageGenerator alloc] initWithAsset:asset];

        generator.appliesPreferredTrackTransform = YES;

        generator.maximumSize = CGSizeZero;

        CMTime time = CMTimeMakeWithSeconds(seconds, 600);

        NSError* error = nil;
        CGImageRef image = [generator copyCGImageAtTime:time actualTime:nil error:&error];

        if (!image) return QImage();

        width  = (int)CGImageGetWidth(image);
        height = (int)CGImageGetHeight(image);

        rgba.resize(width * height * 4);

        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

        CGContextRef context = CGBitmapContextCreate(
            rgba.data(),
            width,
            height,
            8,
            width * 4,
            colorSpace,
            kCGImageAlphaPremultipliedLast |
            kCGBitmapByteOrder32Big
        );

        CGContextDrawImage(context,
                           CGRect{{0,0},QSize(width,height).toCGSize()},
                           image);

        CGContextRelease(context);
        CGImageRelease(image);
        CGColorSpaceRelease(colorSpace);

    }
    return QImage(rgba.data(), width, height, QImage::Format_RGBA8888).copy();
}

double avf_video_track_duration(const QString& path) {
    NSURL *url = [NSURL fileURLWithPath:path.toNSString()];
    AVAsset* asset = [AVAsset assetWithURL:url];

    NSArray* tracks = [asset tracksWithMediaType:AVMediaTypeVideo];

    AVAssetTrack* videoTrack = tracks.firstObject;

    CMTime videoDuration = videoTrack.timeRange.duration;

    return CMTimeGetSeconds(videoDuration);
}

QSize avf_displaySize(const QString& path)
{
    NSURL *url = [NSURL fileURLWithPath:path.toNSString()];
    AVAsset* asset = [AVAsset assetWithURL:url];
    AVAssetTrack* track = [[asset tracksWithMediaType:AVMediaTypeVideo] firstObject];
    if (!track) return QSize();
    CGSize size = track.naturalSize;
    return QSize(size.width, size.height);
}

static void
imageToBuffer(const QImage& img, CVPixelBufferRef buffer)
{
    CVPixelBufferLockBaseAddress(buffer, 0);

    uchar* dst = (uchar*)CVPixelBufferGetBaseAddress(buffer);

    //size_t dstBPR = CVPixelBufferGetBytesPerRow(buffer);

    //QImage converted = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    //const uchar* src = converted.constBits();
    //int srcBPR = converted.bytesPerLine();
/*
    for (int y = 0; y < converted.height(); ++y)
    {
        memcpy(dst + y * dstBPR,
               src + y * srcBPR,
               srcBPR);
    }
*/
    memcpy(dst,img.constBits(),img.bytesPerLine() * img.height());
    CVPixelBufferUnlockBaseAddress(buffer, 0);
}

struct VideoExporter::Impl
{
    AVAssetWriter* writer = nil;
    AVAssetWriterInput* videoInput = nil;
    AVAssetWriterInput* audioInput = nil;
    AVAssetWriterInputPixelBufferAdaptor* adaptor = nil;

    QSize size;
    int fps;
    int sampleRate;
    int channels;
    CMAudioFormatDescriptionRef formatDesc;
    uint64_t audioSampleCursor = 0;
    quint64 frameIndex = 0;
    dispatch_queue_t videoQueue;
    dispatch_queue_t audioQueue;

    std::deque<QImage> videoFrames;
    std::deque<std::vector<float>> audioBlocks;
};

VideoExporter::VideoExporter(
        const QString& file,
        QSize size,
        int fps, int sampleRate, int channels)
{
    d = new Impl;
    d->size = size;
    d->fps = fps;
    d->sampleRate = sampleRate;
    d->channels = channels;

    d->videoQueue = dispatch_queue_create("video.queue", DISPATCH_QUEUE_SERIAL);
    d->audioQueue = dispatch_queue_create("audio.queue", DISPATCH_QUEUE_SERIAL);

    NSURL* url = [NSURL fileURLWithPath:file.toNSString()];

    NSError* err = nil;

    d->writer = [[AVAssetWriter alloc]
            initWithURL:url
            fileType:AVFileTypeQuickTimeMovie
            error:&err];

    NSDictionary* settings = @{
        AVVideoCodecKey: AVVideoCodecTypeH264,
        AVVideoWidthKey: @(size.width()),
        AVVideoHeightKey: @(size.height())
    };

    d->videoInput = [AVAssetWriterInput
            assetWriterInputWithMediaType:AVMediaTypeVideo
            outputSettings:settings];

    d->videoInput.expectsMediaDataInRealTime = NO;

    NSDictionary* attrs = @{
        (id)kCVPixelBufferPixelFormatTypeKey:
            @(kCVPixelFormatType_32BGRA),
        (id)kCVPixelBufferWidthKey:
            @(size.width()),
        (id)kCVPixelBufferHeightKey:
            @(size.height())
    };

    d->adaptor = [[AVAssetWriterInputPixelBufferAdaptor alloc]
            initWithAssetWriterInput:d->videoInput
            sourcePixelBufferAttributes:attrs];

    [d->writer addInput:d->videoInput];

    AudioChannelLayout acl = {};
    acl.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;

    AudioStreamBasicDescription asbd = {};
    asbd.mSampleRate       = d->sampleRate;
    asbd.mFormatID         = kAudioFormatLinearPCM;
    asbd.mFormatFlags      = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    asbd.mFramesPerPacket  = 1;
    asbd.mChannelsPerFrame = d->channels;
    asbd.mBitsPerChannel   = 32;
    asbd.mBytesPerFrame    = d->channels * sizeof(float);
    asbd.mBytesPerPacket   = asbd.mBytesPerFrame;

    CMAudioFormatDescriptionCreate(
        kCFAllocatorDefault,
        &asbd,
        0,
        NULL,
        0,
        NULL,
        NULL,
        &d->formatDesc);

    NSDictionary* audioSettings = @{
        AVFormatIDKey: @(kAudioFormatLinearPCM),

        AVSampleRateKey: @(d->sampleRate),
        AVNumberOfChannelsKey: @(d->channels),

        AVLinearPCMBitDepthKey: @(32),
        AVLinearPCMIsFloatKey: @YES,
        AVLinearPCMIsBigEndianKey: @NO,
        AVLinearPCMIsNonInterleaved: @NO
    };

    d->audioInput = [[AVAssetWriterInput alloc]
         initWithMediaType:AVMediaTypeAudio
         outputSettings:audioSettings];

    if ([d->writer canAddInput:d->audioInput]) {
        [d->writer addInput:d->audioInput];
    } else {
        qDebug() << "cannot add audio input";
    }

    [d->writer startWriting];
    [d->writer startSessionAtSourceTime:kCMTimeZero];

    [d->videoInput requestMediaDataWhenReadyOnQueue:d->videoQueue usingBlock:^{
        while (d->videoInput.readyForMoreMediaData) {
            if (d->videoFrames.empty()) break;
            const QImage img = d->videoFrames.front();
            d->videoFrames.pop_front();

            CVPixelBufferRef buffer;
            CVPixelBufferCreate(
                kCFAllocatorDefault,
                size.width(),
                size.height(),
                kCVPixelFormatType_32BGRA,
                NULL,
                &buffer);

            //imageToBuffer(img, buffer);
            CVPixelBufferLockBaseAddress(buffer, 0);
            uchar* dst = (uchar*)CVPixelBufferGetBaseAddress(buffer);
            memcpy(dst,img.constBits(),img.bytesPerLine() * img.height());
            CVPixelBufferUnlockBaseAddress(buffer, 0);

            const CMTime time = CMTimeMake(d->frameIndex++, d->fps);
            [d->adaptor appendPixelBuffer:buffer withPresentationTime:time];

            CVPixelBufferRelease(buffer);

        }
    }];


    [d->audioInput requestMediaDataWhenReadyOnQueue:d->audioQueue usingBlock:^{
        while (d->audioInput.readyForMoreMediaData) {
            if (d->audioBlocks.empty()) break;
            auto block = d->audioBlocks.front();
            d->audioBlocks.pop_front();

            writeAudioBlock(block.data(), block.size()/d->channels);
        }
    }];
    //qDebug() << "video ready %d" << d->videoInput.readyForMoreMediaData;
    //qDebug() << "audio ready %d" << d->audioInput.readyForMoreMediaData;
}

bool VideoExporter::addFrame(const QImage& img, quint64)
{
    d->videoFrames.push_back(img);
    return true;
}

bool VideoExporter::addAudio(float* b)
{
    const int samples = d->sampleRate / d->fps;
    const int total = samples * d->channels;

    std::vector<float> block(b, b + total);
    d->audioBlocks.push_back(std::move(block));

    return true;
}

bool VideoExporter::writeAudioBlock(float* b, int frames)
{
    if (d->writer.status == AVAssetWriterStatusFailed) {
        //qDebug() << "writer failed: %@" << d->writer.error;
        return false;
    }
    const CMTime pts = CMTimeMake(d->audioSampleCursor, d->sampleRate);

    d->audioSampleCursor += frames;

    const size_t dataSize = frames * d->channels * sizeof(float);

    CMBlockBufferRef blockBuffer = nullptr;
    CMSampleBufferRef sampleBuffer;

    CMBlockBufferCreateWithMemoryBlock(
        kCFAllocatorDefault,
        NULL,
        dataSize,
        kCFAllocatorDefault,
        NULL,
        0,
        dataSize,
        0,
        &blockBuffer);

    CMBlockBufferReplaceDataBytes(
        b,
        blockBuffer,
        0,
        dataSize);

    const CMTime duration = CMTimeMake(frames, d->sampleRate);

    CMSampleTimingInfo timing;
    timing.presentationTimeStamp = pts;
    timing.duration = duration;
    timing.decodeTimeStamp = kCMTimeInvalid;

    //qDebug() << frames << pts.value << duration.value << dataSize << d->channels << d->sampleRate;

    CMSampleBufferCreate(
        kCFAllocatorDefault,
        blockBuffer,
        true,
        NULL,
        NULL,
        d->formatDesc,
        frames,
        1,
        &timing,
        0,
        NULL,
        &sampleBuffer);

    if (!d->audioInput.readyForMoreMediaData) {
        CFRelease(sampleBuffer);
        CFRelease(blockBuffer);
        return true; // prova igen nästa frame
    }

    const bool ok = [d->audioInput appendSampleBuffer:sampleBuffer];

    CFRelease(sampleBuffer);
    CFRelease(blockBuffer);
    //if (!ok) qDebug() << "appendSampleBuffer failed: %@" << d->writer.error;
    return ok;
}

void VideoExporter::finish(std::function<void()> done)
{
    [d->videoInput markAsFinished];
    [d->audioInput markAsFinished];

    [d->writer finishWritingWithCompletionHandler:^{
        done();
    }];
    CFRelease(d->formatDesc);
    //qDebug() << "status: %ld" << d->writer.status;
    //qDebug() << "error: %@" << d->writer.error;
}

VideoExporter::~VideoExporter()
{
    delete d;
}

struct ImageExtractor::Impl
{
    AVAssetImageGenerator* generator = nil;
    CGColorSpaceRef colorSpace = nullptr;
    CGRect rect;
    QImage img;
    CGContextRef context = nullptr;
};

ImageExtractor::ImageExtractor(){
    d = new Impl;
}

ImageExtractor::~ImageExtractor()
{
    if (d->colorSpace) CGColorSpaceRelease(d->colorSpace);
    if (d->context) CGContextRelease(d->context);
    d->generator = nil;
    delete d;
}

void ImageExtractor::setSource(const QUrl& url, const QSize& s){

        videoUrl = url;
        frameSize = s;
        NSString* nsPath = url.path().toNSString();
        if (!nsPath) return;
        NSURL* nsurl = [NSURL fileURLWithPath:nsPath];
        AVAsset* asset = [AVAsset assetWithURL:nsurl];
        if (!asset) return;

        d->generator = [[AVAssetImageGenerator alloc] initWithAsset:asset];

        d->generator.appliesPreferredTrackTransform = YES;

        d->generator.maximumSize = frameSize.toCGSize();

        d->generator.requestedTimeToleranceBefore = kCMTimeZero;
        d->generator.requestedTimeToleranceAfter  = kCMTimeZero;

        if (d->colorSpace) CGColorSpaceRelease(d->colorSpace);
        d->colorSpace = CGColorSpaceCreateDeviceRGB();
        d->rect = { {0,0}, {frameSize.toCGSize()} };
        d->img = QImage(frameSize, QImage::Format_RGBA8888_Premultiplied);

        if (d->context) CGContextRelease(d->context);
        d->context = CGBitmapContextCreate(
            d->img.bits(),
            frameSize.width(),
            frameSize.height(),
            8,
            d->img.bytesPerLine(),
            d->colorSpace,
            kCGImageAlphaPremultipliedLast |
            kCGBitmapByteOrder32Big
        );

        CGContextSetInterpolationQuality(d->context, kCGInterpolationHigh);
}

QImage ImageExtractor::getImage(double time)
{
    @autoreleasepool
    {
        CMTime frametime = CMTimeMakeWithSeconds(time, 600);
        NSError* error = nil;
        const CGImageRef image = [d->generator copyCGImageAtTime:frametime actualTime:nil error:&error];
        if (!image) return QImage();
        CGContextDrawImage(d->context, d->rect, image);
        CGImageRelease(image);
        return d->img;
    }
}

struct AVFVideoPlayer::Impl
{
    AVPlayer* player = nil;
    AVPlayerItemVideoOutput* output = nil;
    CGSize frameSize;
    QImage img;
};

AVFVideoPlayer::AVFVideoPlayer()
{
    d = new Impl;
}

AVFVideoPlayer::~AVFVideoPlayer()
{
    if (d->player)
    {
        AVPlayerItem* item = [d->player currentItem];
        if (item && d->output) [item removeOutput:d->output];
        d->player = nil;
        d->output = nil;
    }
    delete d;
}

void AVFVideoPlayer::setSource(const QUrl& url)
{
    Url = QUrl();
    if (d->player)
    {
        AVPlayerItem* item = [d->player currentItem];
        if (item && d->output) [item removeOutput:d->output];
        d->player = nil;
        d->output = nil;
    }

    NSString* nsPath = url.path().toNSString();
    if (!nsPath) return;
    NSURL* nsurl = [NSURL fileURLWithPath:nsPath];
    Url = url;
    pause();

    AVAsset* asset = [AVAsset assetWithURL:nsurl];

    AVMutableComposition* composition = [AVMutableComposition composition];

    AVAssetTrack* videoTrack = [[asset tracksWithMediaType:AVMediaTypeVideo] firstObject];

    d->frameSize = videoTrack.naturalSize;

    d->img = QImage(d->frameSize.width,d->frameSize.height,QImage::QImage::Format_ARGB32);

    AVMutableCompositionTrack* compTrack =
        [composition addMutableTrackWithMediaType:AVMediaTypeVideo
                                  preferredTrackID:kCMPersistentTrackID_Invalid];

    [compTrack insertTimeRange:CMTimeRangeMake(kCMTimeZero, asset.duration)
                       ofTrack:videoTrack
                        atTime:kCMTimeZero
                         error:nil];

    AVPlayerItem* item = [AVPlayerItem playerItemWithAsset:composition];

    item.preferredForwardBufferDuration = 0;

    NSDictionary* attrs =
    @{
        (id)kCVPixelBufferPixelFormatTypeKey:
            @(kCVPixelFormatType_32BGRA),

        (id)kCVPixelBufferWidthKey:
            @(d->frameSize.width),

        (id)kCVPixelBufferHeightKey:
            @(d->frameSize.height)
    };

    d->output = [[AVPlayerItemVideoOutput alloc]
            initWithPixelBufferAttributes:attrs];

    [item addOutput:d->output];

    d->player = [AVPlayer playerWithPlayerItem:item];
    d->player.actionAtItemEnd = AVPlayerActionAtItemEndPause;
    d->player.automaticallyWaitsToMinimizeStalling = NO;

}

void AVFVideoPlayer::play()
{
    if (!d->player) return;
    [d->player play];
    playing = true;
}

void AVFVideoPlayer::pause()
{
    if (d->player) [d->player pause];
    playing = false;
}

QImage AVFVideoPlayer::currentFrame()
{
    if (!playing) return QImage();
    if (!d->player) return QImage();
    CMTime time = [d->player currentTime];

    if (![d->output hasNewPixelBufferForItemTime:time]) return QImage();

    CVPixelBufferRef buffer = [d->output copyPixelBufferForItemTime:time itemTimeForDisplay:nil];

    if (!buffer) return QImage();

    CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);

    const uchar* pixels = (uchar*)CVPixelBufferGetBaseAddress(buffer);

    const int stride = (int)CVPixelBufferGetBytesPerRow(buffer);

    QImage img(
        pixels,
        d->frameSize.width,
        d->frameSize.height,
        stride,
        QImage::Format_ARGB32
    );

    QImage copy = img.copy();

    CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferRelease(buffer);

    return copy;
}

/*
QImage AVFVideoPlayer::currentFrame()
{
    if (!playing) return QImage();
    if (!d->player) return QImage();

    CMTime time = [d->player currentTime];

    if (![d->output hasNewPixelBufferForItemTime:time]) return QImage();

    CVPixelBufferRef buffer = [d->output copyPixelBufferForItemTime:time
                         itemTimeForDisplay:nil];

    if (!buffer) return QImage();

    CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);

    const int width  = (int)CVPixelBufferGetWidth(buffer);
    const int height = (int)CVPixelBufferGetHeight(buffer);
    const int stride = (int)CVPixelBufferGetBytesPerRow(buffer);

    const uchar* src = (uchar*)CVPixelBufferGetBaseAddress(buffer);

    //for (int y = 0; y < height; ++y) memcpy(d->img.scanLine(y), src + y * stride, width * 4);
    if (stride == width * 4)
    {
        memcpy(d->img.bits(), src, width * height * 4);
    }
    else
    {
        for (int y = 0; y < height; ++y)
            memcpy(d->img.scanLine(y), src + y * stride, width * 4);
    }
    CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferRelease(buffer);

    return d->img;
}
*/
/*
QImage AVFVideoPlayer::currentFrame()
{
    if (!playing) return QImage();
    if (!d->player) return QImage();

    CMTime time = [d->player currentTime];

    if (![d->output hasNewPixelBufferForItemTime:time])
        return QImage();

    CVPixelBufferRef buffer =
        [d->output copyPixelBufferForItemTime:time
                         itemTimeForDisplay:nil];

    if (!buffer)
        return QImage();

    CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);

    int srcW = (int)CVPixelBufferGetWidth(buffer);
    int srcH = (int)CVPixelBufferGetHeight(buffer);
    int stride = (int)CVPixelBufferGetBytesPerRow(buffer);

    uchar* src =
        (uchar*)CVPixelBufferGetBaseAddress(buffer);

    //QImage img(d->frameSize.width, d->frameSize.height, QImage::Format_ARGB32);

    if (!d->needsResize) {
        for (int y = 0; y < d->frameSize.height; ++y)
            memcpy(d->img.scanLine(y), src + y * stride, d->frameSize.width * 4);
    }
    else {
        CGDataProviderRef provider =
            CGDataProviderCreateWithData(
                NULL,
                src,
                stride * srcH,
                NULL
            );

        CGImageRef cg =
            CGImageCreate(
                srcW,
                srcH,
                8,
                32,
                stride,
                d->colorSpace,
                kCGImageAlphaPremultipliedFirst |
                kCGBitmapByteOrder32Little,
                provider,
                NULL,
                false,
                kCGRenderingIntentDefault
            );

        CGContextSetInterpolationQuality(d->context, kCGInterpolationHigh);

        CGContextDrawImage(
            d->context,
            CGRectMake(0,0,d->frameSize.width,d->frameSize.height),
            cg
        );

        CGImageRelease(cg);
        CGDataProviderRelease(provider);
    }
    CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferRelease(buffer);

    return d->img;
}
*/
void AVFVideoPlayer::setPosition(double seconds)
{
    if (!d->player) return;

    AVPlayerItem* item = [d->player currentItem];

    [item cancelPendingSeeks];

    CMTime time = CMTimeMakeWithSeconds(seconds, 600);

    [d->player seekToTime:time
        toleranceBefore:kCMTimeZero
         toleranceAfter:kCMTimeZero];
}

double AVFVideoPlayer::position() const
{
    if (!d->player) return 0;
    CMTime time = [d->player currentTime];
    return CMTimeGetSeconds(time);
}

double AVFVideoPlayer::duration() const
{
    if (!d->player) return 0;
    AVPlayerItem* item = [d->player currentItem];
    return CMTimeGetSeconds(item.duration);
}

void AVFVideoPlayer::setPlaybackRate(double rate)
    {
        if (!d->player) return;
        d->player.rate = rate;
    }

double AVFVideoPlayer::playbackRate() const
    {
        if (!d->player) return 0.0;
        return d->player.rate;
    }
