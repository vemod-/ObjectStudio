#import <AVFoundation/AVFoundation.h>
#import "avfoundation_wrapper.h"

bool avf_read_audio(const char* path,
                    std::vector<std::vector<float>> &outData,
                    double &outSampleRate,
                    int &outChannels)
{
    @autoreleasepool {
        NSString *filePath = [NSString stringWithUTF8String:path];
        NSURL *url = [NSURL fileURLWithPath:filePath];
        AVAudioFile *file = [[AVAudioFile alloc] initForReading:url error:nil];
        if (!file) return false;

        AVAudioFormat *format = file.processingFormat;
        outSampleRate = format.sampleRate;
        outChannels = format.channelCount;

        AVAudioPCMBuffer *buffer = [[AVAudioPCMBuffer alloc]
            initWithPCMFormat:format
            frameCapacity:(AVAudioFrameCount)file.length];

        [file readIntoBuffer:buffer error:nil];

        outData.resize(outChannels);
        for (int c = 0; c < outChannels; ++c)
            outData[c].resize(buffer.frameLength);

        //float **channels = buffer.floatChannelData;
        float **channels = (float **)buffer.floatChannelData;
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
        for (int c = 0; c < channels; ++c) {
            memcpy(channelsData[c], inData[c].data(), frames * sizeof(float));
        }

        [file writeFromBuffer:buffer error:nil];
        return true;
    }
}
