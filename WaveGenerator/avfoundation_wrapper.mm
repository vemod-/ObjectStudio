#import <AVFoundation/AVFoundation.h>
#include <deque>
#import "avfoundation_wrapper.h"
#include <QDebug>

bool avf_read_audio(const char* path,
                    std::vector<std::vector<float>> &outData,
                    double &outSampleRate,
                    int &outChannels)
{
    @autoreleasepool {
        NSString *filePath = [NSString stringWithUTF8String:path];
        if (filePath == nil) return false;
        NSURL *url = [NSURL fileURLWithPath:filePath];
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

bool avf_write_audio(const char* path,
                     const std::vector<std::vector<float>> &inData,
                     double sampleRate)
{
    @autoreleasepool {
        NSString *filePath = [NSString stringWithUTF8String:path];
        if (filePath == nil) return false;
        NSURL *url = [NSURL fileURLWithPath:filePath];

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

bool avf_has_video(const char *path) {
        @autoreleasepool {
            NSString *filePath = [NSString stringWithUTF8String:path];
            if (filePath == nil) return false;
            NSURL *url = [NSURL fileURLWithPath:filePath];
            AVAsset *asset = [AVAsset assetWithURL:url];
            return [[asset tracksWithMediaType:AVMediaTypeVideo] count] > 0;
        }
}

bool avf_is_valid(const char *path){
        @autoreleasepool {
            NSString *filePath = [NSString stringWithUTF8String:path];
            if (filePath == nil) return false;
            NSURL *url = [NSURL fileURLWithPath:filePath];
            AVURLAsset *asset = [AVURLAsset URLAssetWithURL:url options:nil];
            NSArray *audioTracks = [asset tracksWithMediaType:AVMediaTypeAudio];
            return ([audioTracks count] > 0);
        }
}

bool avf_extract_thumbnail(const char* path,
                           double seconds,
                           std::vector<uint8_t>& outRGBA,
                           int& width,
                           int& height)
{
    @autoreleasepool {

        NSString *filePath = [NSString stringWithUTF8String:path];
        if (!filePath) return false;

        NSURL *url = [NSURL fileURLWithPath:filePath];
        AVURLAsset *asset = [AVURLAsset URLAssetWithURL:url options:nil];
        if (!asset) return false;

        NSArray *videoTracks =
            [asset tracksWithMediaType:AVMediaTypeVideo];
        if (videoTracks.count == 0)
            return false;

        AVAssetTrack *track = videoTracks.firstObject;

        // Korrekt upplösning med transform (portrait videos mm)
        CGSize naturalSize = track.naturalSize;
        CGAffineTransform t = track.preferredTransform;
        CGSize transformed =
            CGSizeApplyAffineTransform(naturalSize, t);

        width  = std::abs(transformed.width);
        height = std::abs(transformed.height);

        AVAssetImageGenerator *generator =
            [[AVAssetImageGenerator alloc] initWithAsset:asset];

        generator.appliesPreferredTrackTransform = YES;

        CMTime time = CMTimeMakeWithSeconds(seconds, 600);

        NSError *error = nil;
        CGImageRef image =
            [generator copyCGImageAtTime:time
                              actualTime:NULL
                                   error:&error];

        if (!image)
            return false;

        outRGBA.resize(width * height * 4);

        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

        CGContextRef ctx = CGBitmapContextCreate(
            outRGBA.data(),
            width,
            height,
            8,
            width * 4,
            colorSpace,
            kCGImageAlphaPremultipliedLast
        );

        CGContextDrawImage(ctx,
                           CGRectMake(0,0,width,height),
                           image);

        CGContextRelease(ctx);
        CGColorSpaceRelease(colorSpace);
        CGImageRelease(image);

        return true;
    }
}

bool avf_naturalsize(const char* path,
                           int& width,
                           int& height)
{
    @autoreleasepool {
        NSString *filePath = [NSString stringWithUTF8String:path];
        if (!filePath) return false;

        NSURL *url = [NSURL fileURLWithPath:filePath];
        AVAsset* asset = [AVAsset assetWithURL:url];
        NSArray* tracks = [asset tracksWithMediaType:AVMediaTypeVideo];

        if (tracks.count > 0) {
            AVAssetTrack* track = tracks[0];
            //CGSize size = track.naturalSize;
            CGAffineTransform txf = track.preferredTransform;
            CGSize size = CGSizeApplyAffineTransform(track.naturalSize, txf);
            size.width  = fabs(size.width);
            size.height = fabs(size.height);
            width = size.width;
            height = size.height;
            return true;
        }
        return false;
    }
}

bool avf_extract_fullframe(const char* path,
                           double seconds,
                           std::vector<unsigned char>& rgba,
                           int& width,
                           int& height)
{
    @autoreleasepool
    {
        NSString* nsPath = [NSString stringWithUTF8String:path];
        if (!nsPath) return false;
        NSURL* url = [NSURL fileURLWithPath:nsPath];

        AVAsset* asset = [AVAsset assetWithURL:url];
        if (!asset)
            return false;

        AVAssetImageGenerator* generator =
            [[AVAssetImageGenerator alloc] initWithAsset:asset];

        generator.appliesPreferredTrackTransform = YES;

        generator.maximumSize = CGSizeZero;

        CMTime time = CMTimeMakeWithSeconds(seconds, 600);

        NSError* error = nil;
        CGImageRef image =
            [generator copyCGImageAtTime:time
                              actualTime:nil
                                   error:&error];

        if (!image)
            return false;

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
                           CGRectMake(0, 0, width, height),
                           image);

        CGContextRelease(context);
        CGImageRelease(image);
        CGColorSpaceRelease(colorSpace);

        return true;
    }
}

double avf_lastVideoFrameTime(const char* path)
{
    NSString* nsPath = [NSString stringWithUTF8String:path];
    if (!nsPath) return 0;
    NSURL* url = [NSURL fileURLWithPath:nsPath];

    AVAsset* asset = [AVAsset assetWithURL:url];

    AVAssetTrack* track =
        [[asset tracksWithMediaType:AVMediaTypeVideo] firstObject];

    NSError* error = nil;

    AVAssetReader* reader =
        [[AVAssetReader alloc] initWithAsset:asset error:&error];

    NSDictionary* settings =
    @{
        (id)kCVPixelBufferPixelFormatTypeKey:
        @(kCVPixelFormatType_32BGRA)
    };

    AVAssetReaderTrackOutput* output =
        [[AVAssetReaderTrackOutput alloc]
            initWithTrack:track
            outputSettings:settings];

    [reader addOutput:output];
    [reader startReading];

    CMSampleBufferRef sample = nil;
    CMTime last = kCMTimeZero;

    while ((sample = [output copyNextSampleBuffer]))
    {
        last = CMSampleBufferGetPresentationTimeStamp(sample);
        CFRelease(sample);
    }

    return CMTimeGetSeconds(last);
}

double avf_video_track_duration(const char *path) {
    NSString* nsPath = [NSString stringWithUTF8String:path];
    if (!nsPath) return 0;
    NSURL* url = [NSURL fileURLWithPath:nsPath];

    AVAsset* asset = [AVAsset assetWithURL:url];

    NSArray* tracks =
        [asset tracksWithMediaType:AVMediaTypeVideo];

    AVAssetTrack* videoTrack = tracks.firstObject;

    CMTime videoDuration = videoTrack.timeRange.duration;

    return CMTimeGetSeconds(videoDuration);
}

static CVPixelBufferRef
imageToBuffer(const QImage& img, QSize size)
{
    CVPixelBufferRef buffer;

    CVPixelBufferCreate(
        kCFAllocatorDefault,
        size.width(),
        size.height(),
        kCVPixelFormatType_32BGRA,
        NULL,
        &buffer);

    CVPixelBufferLockBaseAddress(buffer, 0);

    uchar* dst =
        (uchar*)CVPixelBufferGetBaseAddress(buffer);

    size_t dstBPR =
        CVPixelBufferGetBytesPerRow(buffer);

    QImage converted =
        img.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    const uchar* src = converted.constBits();
    int srcBPR = converted.bytesPerLine();

    for (int y = 0; y < converted.height(); ++y)
    {
        memcpy(dst + y * dstBPR,
               src + y * srcBPR,
               srcBPR);
    }

    CVPixelBufferUnlockBaseAddress(buffer, 0);

    return buffer;
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
        NSLog(@"cannot add audio input");
    }

    [d->writer startWriting];
    [d->writer startSessionAtSourceTime:kCMTimeZero];

    [d->videoInput requestMediaDataWhenReadyOnQueue:d->videoQueue usingBlock:^{
        while (d->videoInput.readyForMoreMediaData) {
            if (d->videoFrames.empty()) break;
            QImage img = d->videoFrames.front();
            d->videoFrames.pop_front();

            CVPixelBufferRef buffer = imageToBuffer(img, d->size);

            CMTime time = CMTimeMake(d->frameIndex++, d->fps);

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

    NSLog(@"video ready %d", d->videoInput.readyForMoreMediaData);
    NSLog(@"audio ready %d", d->audioInput.readyForMoreMediaData);
}

bool VideoExporter::addFrame(const QImage& img, quint64)
{
    d->videoFrames.push_back(img);
    return true;
}

bool VideoExporter::addAudio(float* b)
{
    int samples = d->sampleRate / d->fps;
    int total = samples * d->channels;

    std::vector<float> block(b, b + total);
    d->audioBlocks.push_back(std::move(block));

    return true;
}

bool VideoExporter::writeAudioBlock(float* b, int frames)
{
    if (d->writer.status == AVAssetWriterStatusFailed) {
        NSLog(@"writer failed: %@", d->writer.error);
        return false;
    }
    CMTime pts = CMTimeMake(d->audioSampleCursor, d->sampleRate);

    d->audioSampleCursor += frames;

    size_t dataSize = frames * d->channels * sizeof(float);

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

    CMTime duration = CMTimeMake(frames, d->sampleRate);

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

    bool ok = [d->audioInput appendSampleBuffer:sampleBuffer];

    CFRelease(sampleBuffer);
    CFRelease(blockBuffer);
    if (!ok) NSLog(@"appendSampleBuffer failed: %@", d->writer.error);
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
    NSLog(@"status: %ld", d->writer.status);
    NSLog(@"error: %@", d->writer.error);
}

VideoExporter::~VideoExporter()
{
    delete d;
}

struct ImageExtractor::Impl
{
    AVAssetImageGenerator* generator = nil;
};

ImageExtractor::ImageExtractor(){
    d = new Impl;
}

ImageExtractor::~ImageExtractor(){
    if (d->generator)
        d->generator = nil;
    delete d;
}

void ImageExtractor::init(const QUrl& URL, const QSize& FrameSize){

        url = URL;
        frameSize = FrameSize;
        NSString* nsPath = [NSString stringWithUTF8String:URL.path().toStdString().c_str()];
        if (!nsPath) return;
        NSURL* Url = [NSURL fileURLWithPath:nsPath];

        AVAsset* asset = [AVAsset assetWithURL:Url];
        if (!asset)
            return;

        d->generator =
            [[AVAssetImageGenerator alloc] initWithAsset:asset];

        d->generator.appliesPreferredTrackTransform = YES;

        d->generator.maximumSize =
            CGSizeMake(frameSize.width(), frameSize.height());

        d->generator.requestedTimeToleranceBefore = kCMTimeZero;
        d->generator.requestedTimeToleranceAfter  = kCMTimeZero;
}

QImage ImageExtractor::getImage(double time)
{
    @autoreleasepool
    {
        CMTime frametime = CMTimeMakeWithSeconds(time, 600);

        NSError* error = nil;
        CGImageRef image =
            [d->generator copyCGImageAtTime:frametime
                                  actualTime:nil
                                       error:&error];

        if (!image)
            return QImage();

        QImage img(frameSize, QImage::Format_RGBA8888);

        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

        CGContextRef context = CGBitmapContextCreate(
            img.bits(),
            frameSize.width(),
            frameSize.height(),
            8,
            img.bytesPerLine(),
            colorSpace,
            kCGImageAlphaPremultipliedLast |
            kCGBitmapByteOrder32Big
        );

        CGContextSetInterpolationQuality(context, kCGInterpolationHigh);

        CGContextDrawImage(
            context,
            CGRectMake(0, 0, frameSize.width(), frameSize.height()),
            image
        );

        CGContextRelease(context);
        CGColorSpaceRelease(colorSpace);
        CGImageRelease(image);

        return img;
    }
}
