#ifndef IJACKBASE_H
#define IJACKBASE_H

#include <QColor>

class IJackBase
{
public:
    enum AttachModes
    {Amplitude=1,Pitch=2,Voltage=3,Frequency=4,Trigger=8,Wave=16,Audio=16,Stereo=48,MIDI=64};
    enum Directions
    {In,Out};
    inline IJackBase(IJackBase::AttachModes AttachMode, IJackBase::Directions Direction):attachMode(AttachMode),direction(Direction){}
    inline const QColor JackColor() const
    {
        switch (attachMode)
        {
        case Amplitude:
        case Frequency:
        case Pitch:
        case Voltage:
        case Trigger:
            return Qt::yellow;
        case Wave:
            return Qt::darkRed;
        case Stereo:
            return Qt::red;
        case MIDI:
            return Qt::white;
        }
        return Qt::black;
    }
    inline bool isInJack() const { return (direction==IJackBase::In); }
    inline bool isOutJack() const { return (direction==IJackBase::Out); }
    AttachModes attachMode;
    Directions direction;
};

#endif // IJACKBASE_H
