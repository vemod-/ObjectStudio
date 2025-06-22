#ifndef AVFOUNDATION_WRAPPER_H
#define AVFOUNDATION_WRAPPER_H

#include <vector>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

bool avf_read_audio(const char* path,
                    std::vector<std::vector<float>> &outData,
                    double &outSampleRate,
                    int &outChannels);

bool avf_write_audio(const char* path,
                     const std::vector<std::vector<float>> &inData,
                     double sampleRate);

#ifdef __cplusplus
}
#endif

#endif // AVFOUNDATION_WRAPPER_H
