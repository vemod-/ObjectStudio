#include <stdexcept>
#include <ciso646>
#include <limits>
#include <cmath>
//#include <algorithm>
//extern "C" {
//#include <libavfilter/avfilter.h>
//#include <libavutil/avutil.h>
//#include <libavutil/time.h>
//#include <libavutil/timecode.h>
//#include <libavutil/imgutils.h>
//#include <libavutil/samplefmt.h>
//#include <libavutil/timestamp.h>
//#include <libavutil/opt.h>
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#include <libswresample/swresample.h>
//};

#include "audiorw.h"

using namespace audiorw;

std::vector<std::vector<double>> audiorw::read(
    const std::string & filename,
    double & sample_rate,
    double start_seconds,
    double end_seconds) {

    // Get a buffer for writing errors to
    size_t errbuf_size = 200;
    char errbuf[200];

    // Initialize variables
    AVCodecContext * codec_context = NULL;
    AVFormatContext * format_context = NULL;
    SwrContext * resample_context = NULL;
    AVFrame * frame = NULL;
    AVPacket packet;

    // Open the file and get format information
    int error = avformat_open_input(&format_context, filename.c_str(), NULL, 0);
    if (error != 0) {
        av_strerror(error, errbuf, errbuf_size);
        throw std::invalid_argument(
            "Could not open audio file: " + filename + "\n" +
            "Error: " + std::string(errbuf));
    }

    // Get stream info
    if ((error = avformat_find_stream_info(format_context, NULL)) < 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        av_strerror(error, errbuf, errbuf_size);
        throw std::runtime_error(
            "Could not get information about the stream in file: " + filename + "\n" +
            "Error: " + std::string(errbuf));
    }

    // Find an audio stream and its decoder
    AVCodec * codec = NULL;
    int audio_stream_index = av_find_best_stream(
        format_context,
        AVMEDIA_TYPE_AUDIO,
        -1, -1, (const AVCodec**)&codec, 0);
    if (audio_stream_index < 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not determine the best stream to use in the file: " + filename);
    }

    // Allocate context for decoding the codec
    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not allocate a decoding context for file: " + filename);
    }

    // Fill the codecContext with parameters of the codec
    if ((error = avcodec_parameters_to_context(
             codec_context,
             format_context -> streams[audio_stream_index] -> codecpar
             )) != 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not set codec context parameters for file: " + filename);
    }

    // Initialize the decoder
    if ((error = avcodec_open2(codec_context, codec, NULL)) != 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        av_strerror(error, errbuf, errbuf_size);
        throw std::runtime_error(
            "Could not initialize the decoder for file: " + filename + "\n" +
            "Error: " + std::string(errbuf));
    }

    // Make sure there is a channel layout
    if (codec_context -> channel_layout == 0) {
        codec_context -> channel_layout =
            av_get_default_channel_layout(codec_context -> channels);
    }

    // Fetch the sample rate
    sample_rate = codec_context -> sample_rate;
    if (sample_rate <= 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Sample rate is " + std::to_string(sample_rate));
    }

    // Initialize a resampler
    resample_context = swr_alloc_set_opts(
        NULL,
        // Output
        codec_context -> channel_layout,
        AV_SAMPLE_FMT_DBL,
        sample_rate,
        // Input
        codec_context -> channel_layout,
        codec_context -> sample_fmt,
        sample_rate,
        0, NULL);
    if (!resample_context) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not allocate resample context for file: " + filename);
    }

    // Open the resampler context with the specified parameters
    if ((error = swr_init(resample_context)) < 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not open resample context for file: " + filename);
    }

    // Initialize the input frame
    if (!(frame = av_frame_alloc())) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not allocate audio frame for file: " + filename);
    }

    // prepare a packet
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;

    // Get start and end values in samples
    start_seconds = std::max(start_seconds, 0.);
    double duration = (format_context -> duration)/(double)AV_TIME_BASE;
    if (end_seconds < 0) {
        end_seconds = duration;
    } else {
        end_seconds = std::min(end_seconds, duration);
    }
    double start_sample = std::floor(start_seconds * sample_rate);
    double end_sample   = std::floor(end_seconds   * sample_rate);

    // Allocate the output vector
    std::vector<std::vector<double>> audio(codec_context -> channels);

    // Read the file until either nothing is left
    // or we reach desired end of sample
    int sample = 0;
    while (sample < end_sample) {
        // Read from the frame
        error = av_read_frame(format_context, &packet);
        if (error == AVERROR_EOF) {
            break;
        } else if (error < 0) {
            cleanup(codec_context, format_context, resample_context, frame, packet);
            av_strerror(error, errbuf, errbuf_size);
            throw std::runtime_error(
                "Error reading from file: " + filename + "\n" +
                "Error: " + std::string(errbuf));
        }

        // Is this the correct stream?
        if (packet.stream_index != audio_stream_index) {
            // Otherwise move on
            continue;
        }

        // Send the packet to the decoder
        if ((error = avcodec_send_packet(codec_context, &packet)) < 0) {
            cleanup(codec_context, format_context, resample_context, frame, packet);
            av_strerror(error, errbuf, errbuf_size);
            throw std::runtime_error(
                "Could not send packet to decoder for file: " + filename + "\n" +
                "Error: " + std::string(errbuf));
        }

        // Receive a decoded frame from the decoder
        while ((error = avcodec_receive_frame(codec_context, frame)) == 0) {
            // Send the frame to the resampler
            double audio_data[audio.size() * frame -> nb_samples];
            uint8_t * audio_data_ = reinterpret_cast<uint8_t *>(audio_data);
            const uint8_t ** frame_data = const_cast<const uint8_t**>(frame -> extended_data);
            if ((error = swr_convert(resample_context,
                                     &audio_data_, frame -> nb_samples,
                                     frame_data  , frame -> nb_samples)) < 0) {
                cleanup(codec_context, format_context, resample_context, frame, packet);
                av_strerror(error, errbuf, errbuf_size);
                throw std::runtime_error(
                    "Could not resample frame for file: " + filename + "\n" +
                    "Error: " + std::string(errbuf));
            }

            // Update the frame
            for (int s = 0; s < frame -> nb_samples; s++) {
                int index = sample + s - start_sample;
                if ((0 <= index) and (index < end_sample)) {
                    for (int channel = 0; channel < (int) audio.size(); channel++) {
                        audio[channel].push_back(audio_data[audio.size() * s + channel]);
                    }
                }
            }

            // Increment the stamp
            sample += frame -> nb_samples;
        }

        // Check if the decoder had any errors
        if (error != AVERROR(EAGAIN)) {
            cleanup(codec_context, format_context, resample_context, frame, packet);
            av_strerror(error, errbuf, errbuf_size);
            throw std::runtime_error(
                "Error receiving packet from decoder for file: " + filename + "\n" +
                "Error: " + std::string(errbuf));
        }
    }

    // Cleanup
    cleanup(codec_context, format_context, resample_context, frame, packet);

    return audio;
}

void audiorw::cleanup(
    AVCodecContext * codec_context,
    AVFormatContext * format_context,
    SwrContext * resample_context,
    AVFrame * frame,
    AVPacket packet) {
    // Properly free any allocated space
    avcodec_close(codec_context);
    avcodec_free_context(&codec_context);
    avio_closep(&format_context -> pb);
    avformat_free_context(format_context);
    swr_free(&resample_context);
    av_frame_free(&frame);
    av_packet_unref(&packet);
}

void audiorw::write(
    const std::vector<std::vector<double>> & audio,
    const std::string & filename,
    double sample_rate) {

    // Get a buffer for writing errors to
    size_t errbuf_size = 200;
    char errbuf[200];

    AVCodecContext * codec_context = NULL;
    AVFormatContext * format_context = NULL;
    SwrContext * resample_context = NULL;
    AVFrame * frame = NULL;
    AVPacket packet;

    // Open the output file to write to it
    AVIOContext * output_io_context;
    int error = avio_open(
        &output_io_context,
        filename.c_str(),
        AVIO_FLAG_WRITE);
    if (error < 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        av_strerror(error, errbuf, errbuf_size);
        throw std::invalid_argument(
            "Could not open file:" + filename + "\n" +
            "Error: " + std::string(errbuf));
    }

    // Create a format context for the output container format
    if (!(format_context = avformat_alloc_context())) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not allocate output format context for file:" + filename);
    }

    // Associate the output context with the output file
    format_context -> pb = output_io_context;

    // Guess the desired output file type
    if (!(format_context->oformat = av_guess_format(NULL, filename.c_str(), NULL))) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not find output file format for file: " + filename);
    }

    // Add the file pathname to the output context
    if (!(format_context -> url = av_strdup(filename.c_str()))) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not process file path name for file: " + filename);
    }

    // Guess the encoder for the file
    AVCodecID codec_id = av_guess_codec(
        format_context -> oformat,
        NULL,
        filename.c_str(),
        NULL,
        AVMEDIA_TYPE_AUDIO);

    // Find an encoder based on the codec
    AVCodec * output_codec;
    if (!(output_codec = (AVCodec*)avcodec_find_encoder(codec_id))) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not open codec with ID, " + std::to_string(codec_id) + ", for file: " + filename);
    }

    // Create a new audio stream in the output file container
    AVStream * stream;
    if (!(stream = avformat_new_stream(format_context, NULL))) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not create new stream for output file: " + filename);
    }

    // Allocate an encoding context
    if (!(codec_context = avcodec_alloc_context3(output_codec))) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not allocate an encoding context for output file: " + filename);
    }

    // Set the parameters of the stream
    codec_context -> channels = audio.size();
    codec_context -> channel_layout = av_get_default_channel_layout(audio.size());
    codec_context -> sample_rate = sample_rate;
    codec_context -> sample_fmt = output_codec -> sample_fmts[0];
    codec_context -> bit_rate = OUTPUT_BIT_RATE;

    // Set the sample rate of the container
    stream -> time_base.den = sample_rate;
    stream -> time_base.num = 1;

    // Add a global header if necessary
    if (format_context -> oformat -> flags & AVFMT_GLOBALHEADER)
        codec_context -> flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // Open the encoder for the audio stream to use
    if ((error = avcodec_open2(codec_context, output_codec, NULL)) < 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        av_strerror(error, errbuf, errbuf_size);
        throw std::runtime_error(
            "Could not open output codec for file: " + filename + "\n" +
            "Error: " + std::string(errbuf));
    }

    // Make sure everything has been initialized correctly
    error = avcodec_parameters_from_context(stream->codecpar, codec_context);
    if (error < 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not initialize stream parameters for file: " + filename);
    }

    // Initialize a resampler
    resample_context = swr_alloc_set_opts(
        NULL,
        // Output
        codec_context -> channel_layout,
        codec_context -> sample_fmt,
        sample_rate,
        // Input
        codec_context -> channel_layout,
        AV_SAMPLE_FMT_DBL,
        sample_rate,
        0, NULL);
    if (!resample_context) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not allocate resample context for file: " + filename);
    }

    // Open the context with the specified parameters
    if ((error = swr_init(resample_context)) < 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not open resample context for file: " + filename);
    }

    // Write the header to the output file
    if ((error = avformat_write_header(format_context, NULL)) < 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not write output file header for file: " + filename);
    }

    // Initialize the output frame
    if (!(frame = av_frame_alloc())) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not allocate output frame for file: " + filename);
    }
    // Allocate the frame size and format
    if (codec_context -> frame_size <= 0) {
        codec_context -> frame_size = DEFAULT_FRAME_SIZE;
    }
    frame -> nb_samples     = codec_context -> frame_size;
    frame -> channel_layout = codec_context -> channel_layout;
    frame -> format         = codec_context -> sample_fmt;
    frame -> sample_rate    = codec_context -> sample_rate;
    // Allocate the samples in the frame
    if ((error = av_frame_get_buffer(frame, 0)) < 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        av_strerror(error, errbuf, errbuf_size);
        throw std::runtime_error(
            "Could not allocate output frame samples for file: " + filename + "\n" +
            "Error: " + std::string(errbuf));
    }

    // Construct a packet for the encoded frame
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;

    // Write the samples to the audio
    int sample = 0;
    double audio_data[audio.size() * codec_context -> frame_size];
    while (true) {
        if (sample < (int) audio[0].size()) {
            // Determine how much data to send
            int frame_size = std::min(codec_context -> frame_size, int(audio[0].size() - sample));
            frame -> nb_samples = frame_size;

            // Timestamp the frame
            frame -> pts = sample;

            // Choose a frame size of the audio
            for (int s = 0; s < frame_size; s++) {
                for (int channel = 0; channel < (int) audio.size(); channel++) {
                    audio_data[audio.size() * s + channel] = audio[channel][sample+s];
                }
            }

            // Increment
            sample += frame_size;

            // Fill the frame with audio data
            const uint8_t * audio_data_ = reinterpret_cast<uint8_t *>(audio_data);
            if ((error = swr_convert(resample_context,
                                     frame -> extended_data, frame_size,
                                     &audio_data_          , frame_size)) < 0) {
                cleanup(codec_context, format_context, resample_context, frame, packet);
                av_strerror(error, errbuf, errbuf_size);
                throw std::runtime_error(
                    "Could not resample frame for file: " + filename + "\n" +
                    "Error: " + std::string(errbuf));
            }
        } else {
            // Enter draining mode
            av_frame_free(&frame);
            frame = NULL;
        }

        // Send a frame to the encoder to encode
        if ((error = avcodec_send_frame(codec_context, frame)) < 0) {
            cleanup(codec_context, format_context, resample_context, frame, packet);
            av_strerror(error, errbuf, errbuf_size);
            throw std::runtime_error(
                "Could not send packet for encoding for file: " + filename + "\n" +
                "Error: " + std::string(errbuf));
        }

        // Receive the encoded frame from the encoder
        while ((error = avcodec_receive_packet(codec_context, &packet)) == 0) {
            // Write the encoded frame to the file
            if ((error = av_write_frame(format_context, &packet)) < 0) {
                cleanup(codec_context, format_context, resample_context, frame, packet);
                av_strerror(error, errbuf, errbuf_size);
                throw std::runtime_error(
                    "Could not write frame for file: " + filename + "\n" +
                    "Error: " + std::string(errbuf));
            }
        }

        // If we drain to the end, end the loop
        if (error == AVERROR_EOF) {
            break;
            // If there was an error with the decoder
        } else if (error != AVERROR(EAGAIN)) {
            cleanup(codec_context, format_context, resample_context, frame, packet);
            av_strerror(error, errbuf, errbuf_size);
            throw std::runtime_error(
                "Could not encode frame for file: " + filename + "\n" +
                "Error: " + std::string(errbuf));
        }

    }

    // Write the trailer to the output file
    if ((error = av_write_trailer(format_context)) < 0) {
        cleanup(codec_context, format_context, resample_context, frame, packet);
        throw std::runtime_error(
            "Could not write output file trailer for file: " + filename);
    }

    // Cleanup
    cleanup(codec_context, format_context, resample_context, frame, packet);
}
