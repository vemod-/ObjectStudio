#ifndef CWAVEGENERATOR_H
#define CWAVEGENERATOR_H

#include "cwavefile.h"
#include "qdomlite.h"
#include <QGraphicsScene>
#include <QtGui/qpainterpath.h>
#include "cpresets.h"
#include <QGraphicsPathItem>
#include <QPainter>

#include <vector>
#include <cmath>
#include <algorithm>

class MinMaxPyramid
{
public:
    struct MinMax {
        float min = 0.f;
        float max = 0.f;
    };
    using Level = std::vector<MinMax>;
public:
    MinMaxPyramid() = default;
    template<typename SampleProvider>
    void build(SampleProvider&& sampleAt, size_t sampleCount, size_t channelCount) {
        m_channels = channelCount;
        m_levels.clear();
        if (sampleCount == 0 || channelCount == 0) return;
        buildLevel0(sampleAt, sampleCount);
        buildHigherLevels();
    }
    size_t levelCount() const { return m_levels.size(); }
    size_t channels()   const { return m_channels; }
    const Level& level(size_t level, size_t channel) const { return m_levels[level][channel]; }
    size_t levelForSamplesPerPixel(double samplesPerPixel) const {
        if (samplesPerPixel <= 1.0) return 0;
        size_t level = (size_t)std::floor(std::log2(samplesPerPixel));
        return std::min(level, m_levels.size() - 1);
    }
    const MinMax& value(size_t level, size_t channel, size_t index) const { return m_levels[level][channel][index]; }
    size_t levelSize(size_t level) const { return m_levels[level][0].size(); }
    size_t blockSize(size_t level) const { return size_t(1) << level; }
private:
    std::vector<std::vector<Level>> m_levels;
    size_t m_channels = 0;
private:
    template<typename SampleProvider>
    void buildLevel0(SampleProvider& sampleAt, size_t sampleCount) {
        m_levels.resize(1);
        m_levels[0].resize(m_channels);
        for (size_t ch = 0; ch < m_channels; ++ch) {
            auto& lvl = m_levels[0][ch];
            lvl.resize(sampleCount);
            for (size_t i = 0; i < sampleCount; ++i) {
                float v = sampleAt(i, ch);
                lvl[i] = { v, v };
            }
        }
    }
    void buildHigherLevels() {
        size_t prevSize = m_levels[0][0].size();
        while (prevSize > 1) {
            const size_t newSize = prevSize / 2;
            if (newSize == 0) break;
            const size_t newLevelIndex = m_levels.size();
            m_levels.emplace_back();
            m_levels.back().resize(m_channels);
            for (size_t ch = 0; ch < m_channels; ++ch) {
                const auto& prev = m_levels[newLevelIndex - 1][ch];
                auto& cur        = m_levels[newLevelIndex][ch];
                cur.resize(newSize);
                for (size_t i = 0; i < newSize; ++i) {
                    const auto& a = prev[i * 2];
                    const auto& b = prev[i * 2 + 1];
                    cur[i].min = std::min(a.min, b.min);
                    cur[i].max = std::max(a.max, b.max);
                }
            }
            prevSize = newSize;
        }
    }
};

class WaveformItem : public QGraphicsItem
{
public:
    WaveformItem(const QVector<QLineF> lines, const QRect r, QGraphicsItem* parent = nullptr)
        : QGraphicsItem(parent) {
        m_lines = lines;
        m_bounds = r;
        setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }

    QRectF boundingRect() const override { return m_bounds; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter->setPen(Qt::black);
        painter->drawLines(m_lines);
        painter->restore();
    }
private:
    QVector<QLineF> m_lines;
    QRectF m_bounds;
};

class CWaveGenerator : protected IPresetRef
{
public:
    enum LoopTypeEnum
    {ltForward,ltAlternate,ltXFade};
    enum SampleStates
    {ssNone,ssSilent,ssStarting,ssLooping,ssEnding};
    class LoopParameters
    {
    public:
        LoopParameters() { reset(); }
        ulong64 Start;
        ulong64 End;
        ulong64 LoopStart;
        ulong64 LoopEnd;
        int MIDIKey;
        int MIDICents;
        ulong64 FadeIn;
        ulong64 FadeOut;
        int Volume;
        ulong64 VideoFadeIn;
        ulong64 VideoFadeOut;
        int VideoOpacity;
        int XFade;
        LoopTypeEnum LoopType;
        uint origRate;
        double Speed = 1;
        double PitchShift;
        void reset(ulong64 len=0)
        {
            Start=0;
            End=len;
            LoopStart=0;
            LoopEnd=0;
            MIDIKey=69;
            MIDICents=0;
            LoopType=CWaveGenerator::ltForward;
            FadeIn=0;
            FadeOut=0;
            XFade=0;
            Volume=100;
            VideoFadeIn=0;
            VideoFadeOut=0;
            VideoOpacity=100;
            Speed = 1;
            PitchShift = 0;
            origRate=CPresets::presets().SampleRate;
        }
        void convertRate(double oldSampleRate=0)
        {
            if (isZero(oldSampleRate)) oldSampleRate=origRate;
            if (origRate == CPresets::presets().SampleRate) return;
            double newSampleRate=CPresets::presets().SampleRate;
            const ldouble rateFactor=newSampleRate/oldSampleRate;
            Start*=rateFactor;
            End*=rateFactor;
            LoopStart*=rateFactor;
            LoopEnd*=rateFactor;
            FadeIn*=rateFactor;
            FadeOut*=rateFactor;
            VideoFadeIn*=rateFactor;
            VideoFadeOut*=rateFactor;
            origRate=CPresets::presets().SampleRate;
        }
        void unserialize(const QDomLiteElement* xml,ulong64 end=0)
        {
            if (!xml) return;
            Volume=xml->attributeValueInt("Volume",100);
            Start=xml->attributeValueULongLong("Start",0);
            End=xml->attributeValueULongLong("End",end);
            LoopStart=xml->attributeValueULongLong("LoopStart",end);
            LoopEnd=xml->attributeValueULongLong("LoopEnd",end);
            FadeIn=xml->attributeValueULongLong("FadeIn",0);
            FadeOut=xml->attributeValueULongLong("FadeOut",0);
            VideoFadeIn=xml->attributeValueULongLong("VideoFadeIn",0);
            VideoFadeOut=xml->attributeValueULongLong("VideoFadeOut",0);
            VideoOpacity=xml->attributeValueInt("VideoOpacity",100);
            MIDIKey=xml->attributeValueInt("MIDINote",60);
            MIDICents=xml->attributeValueInt("Tune",0);
            LoopType=CWaveGenerator::LoopTypeEnum(xml->attributeValueInt("LoopType",0));
            XFade=xml->attributeValueInt("XFade",0);
            Speed=xml->attributeValue("Speed",1);
            PitchShift=xml->attributeValue("PitchShift",0);
            origRate=xml->attributeValueInt("OrigRate",44100);
            convertRate();
        }
        void serialize(QDomLiteElement* xml) const
        {
            xml->setAttribute("Volume",Volume);
            xml->setAttribute("Start",Start);
            xml->setAttribute("End",End);
            xml->setAttribute("LoopStart",LoopStart);
            xml->setAttribute("LoopEnd",LoopEnd);
            xml->setAttribute("FadeIn",FadeIn);
            xml->setAttribute("FadeOut",FadeOut);
            xml->setAttribute("VideoFadeIn",VideoFadeIn);
            xml->setAttribute("VideoFadeOut",VideoFadeOut);
            xml->setAttribute("VideoOpacity",VideoOpacity);
            xml->setAttribute("MIDINote",MIDIKey);
            xml->setAttribute("Tune",MIDICents);
            xml->setAttribute("LoopType",int(LoopType));
            xml->setAttribute("XFade",XFade);
            xml->setAttribute("Speed",Speed);
            xml->setAttribute("PitchShift",PitchShift);
            xml->setAttribute("OrigRate",int(CPresets::presets().SampleRate));
        }
        inline ulong64 playLength() const { return ldouble(End-Start)/Speed; }
        inline float fadeVolume(ldouble Counter) const
        {
            float Vol=Volume*0.01f;
            if (Counter < FadeIn/Speed)
            {
                Vol*=Counter/(FadeIn/Speed);
            }
            if (Counter>playLength()-(FadeOut/Speed))
            {
                Vol*=(playLength()-Counter)/(FadeOut/Speed);
            }
            return lin2expf(Vol);
        }
        inline float fadeOpacity(ldouble Counter, int round = 0) const
        {
            float Opacity = VideoOpacity * 0.01f;
            if (Counter < VideoFadeIn / Speed)
            {
                Opacity *= Counter / (VideoFadeIn / Speed);
            }
            if (Counter > playLength() - (VideoFadeOut / Speed))
            {
                Opacity *= (playLength() - Counter) / (VideoFadeOut / Speed);
            }
            if (round) return std::round(Opacity * round) / round;
            return Opacity;
        }
    };
    CWaveGenerator();
    ~CWaveGenerator();
    bool load(const QString& path, uint SampleRate=CPresets::presets().SampleRate, uint BufferSize=CPresets::presets().ModulationRate);
    float* getNext();
    float* getNextSpeed(const double Speed);
    float* getNextRate(const double RateOverride);
    float* getNextFreq(const double Frequency=0);
    LoopParameters LP;
    inline ulong64 size() { return m_Size; }
    void reset();
    void release();
    inline float* channelPointer(const uint Channel) const {
        return (!WF) ? nullptr : WF->data.channelPointer(Channel);
    }
    inline CChannelBuffer* buffer() const {
        return (!WF) ? nullptr : &WF->data;
    }
    void skipTo(const ulong64 Ptr);
    ulong64 currentSample() {
        return m_Pointer;
    }
    inline uint channels() const { return m_Audio.channels(); }
    inline uint origRate() const { return WF->frequency; }
    QGraphicsItem* waveFormItem(QRect waveRect, QRect visibleRect, double zoom, LoopParameters* LP) {
        if (!waveRect.intersects(visibleRect)) return nullptr;
        QRect r = waveRect.intersected(visibleRect);
        r.setTop(waveRect.top());
        r.setHeight(waveRect.height());
        QPen p(Qt::black);
        if (size())
        {
            ulong64 Start = 0;
            ulong64 End = size();
            double ZoomValue = 1.0 / zoom;
            if (LP) {
                Start = LP->Start;
                End = LP->End;
                ZoomValue *= LP->Speed;
            }
            if (r.left() > waveRect.left()) Start += ZoomValue * (r.left() - waveRect.left());
            if ((Start == oldStart) && (End == oldEnd) && closeEnough(ZoomValue,oldZoom)) {
                if (r == oldRect) {
                    return new WaveformItem(lines,r);
                }
                if (r.size() == oldRect.size()) {
                    WaveformItem* i = new WaveformItem(lines,oldRect);
                    i->setPos(r.topLeft() - oldRect.topLeft());
                    return i;
                }
            }
            oldStart = Start;
            oldEnd = End;
            oldZoom = ZoomValue;
            oldRect = r;
            fillLines(Start,ZoomValue,r);
        }
        return new WaveformItem(lines,r);
    }
    QUrl videoURL;
    bool hasVideo() {
        return (!videoURL.isEmpty());
    }
private:
    void fillLines(const ulong64 Start, const double ZoomValue, const QRect& r) {
        lines.clear();
        lines.reserve(r.width() * channels());
        double samplesPerPixel = ZoomValue;
        const size_t level = pyramid.levelForSamplesPerPixel(samplesPerPixel);
        const size_t blockSize = pyramid.blockSize(level);
        for (uint channel = 0; channel < channels(); channel++)
        {
            const MinMaxPyramid::Level& lvl = pyramid.level(level, channel);
            const float YFactor = r.height() / (2 * channels());
            const int HalfHeight = (YFactor + (YFactor * channel * 2)) + r.top();
            //ldouble sample = Start;
            //long zeroCount = 0;
            //long64 lastSample = sample;
            int pixel = 0;
            for (int x = r.left(); x < r.right(); x++) {
                const size_t sampleIndex = Start + (pixel++ * samplesPerPixel);
                const size_t idx = sampleIndex / blockSize;
                if (idx < lvl.size()) {
                    const MinMaxPyramid::MinMax& mm = lvl[idx];
                    lines.emplace_back(QLineF(x, HalfHeight - (std::max(0.0f, mm.max) * YFactor), x, HalfHeight - (std::min(0.0f, mm.min) * YFactor)));
                }
                /*
                if (sample >= End) break;
                float max = 0;
                float min = 0;
                for (long64 s = lastSample; s < sample; s++) {
                    float v = Buffer->at(s, channel);
                    if (v > 0)
                        max = std::max(max, v);
                    else
                        min = std::min(min, v);
                }
                lastSample = sample;
                const int iMax = max * YFactor;
                const int iMin = min * YFactor;
                if (iMax | iMin) {
                    if (zeroCount) {
                        lines.emplace_back(QLineF(x - zeroCount, HalfHeight,x, HalfHeight));
                        //path.moveTo(x - zeroCount, HalfHeight);
                        //path.lineTo(x, HalfHeight);
                        zeroCount = 0;
                    }
                    lines.emplace_back(QLineF(x, HalfHeight - iMax,x, HalfHeight - iMin));
                    //path.moveTo(x, HalfHeight - iMax);
                    //path.lineTo(x, HalfHeight - iMin);
                }
                else {
                    zeroCount++;
                }
                sample += ZoomValue;
*/
            }
            /*
            if (zeroCount) {
                lines.emplace_back(QLineF(x - zeroCount, HalfHeight,x - 1, HalfHeight));
                //path.moveTo(x - zeroCount, HalfHeight);
                //path.lineTo(x - 1, HalfHeight);
            }
*/
        }
    }
    MinMaxPyramid pyramid;
    QVector<QLineF> lines;
    double oldZoom = 0;
    ulong64 oldStart = 0;
    ulong64 oldEnd = 0;
    QRect oldRect = QRect();
    void finishBuffer(const uint fromPtr);
    uint m_BufferSize;
    ldouble m_Pointer;
    CWaveFile* WF;
    QString m_Path;
    bool m_Finished;
    ulong64 m_Size;
    int AlternateDirection;
    float XFadeFactor;
    bool XFadeStarted;
    ulong64 XFadeStart;
    ulong64 XFadeEnd;
    double XFadePosition;
    ldouble m_Position;
    double m_OrigFreq;
    SampleStates m_SampleState;
    void inline Init();
    CChannelBuffer m_Audio;
    void Unref();
    QRecursiveMutex mutex;
};

#endif // CWAVEGENERATOR_H
