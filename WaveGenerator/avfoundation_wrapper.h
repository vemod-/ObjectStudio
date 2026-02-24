#ifndef AVFOUNDATION_WRAPPER_H
#define AVFOUNDATION_WRAPPER_H

#include <vector>

#ifdef __cplusplus
//extern "C" {
#endif

bool avf_read_audio(const char* path,
                    std::vector<std::vector<float>> &outData,
                    double &outSampleRate,
                    int &outChannels);

bool avf_write_audio(const char* path,
                     const std::vector<std::vector<float>> &inData,
                     double sampleRate);
bool avf_has_video(const char* path);
bool avf_is_valid(const char* path);
bool avf_extract_thumbnail(const char* path,
                           double seconds,
                           std::vector<uint8_t>& outRGBA,
                           int& width,
                           int& height);


#ifdef __cplusplus
//}
#endif

#endif // AVFOUNDATION_WRAPPER_H
