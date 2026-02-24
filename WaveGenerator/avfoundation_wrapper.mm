#import <AVFoundation/AVFoundation.h>
#import "avfoundation_wrapper.h"

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
