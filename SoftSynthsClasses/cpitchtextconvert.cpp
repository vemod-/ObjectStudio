#include "cpitchtextconvert.h"

const QStringList CPitchTextConvert::naturalpitches = {"C","C#","D","Eb","E","F","F#","G","Ab","A","Bb","B"};
const QStringList CPitchTextConvert::sharppitches = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
const QStringList CPitchTextConvert::sharppitchesx = {"B#","C#","D","D#","E","E#","F#","G","G#","A","A#","B"};
const QStringList CPitchTextConvert::flatpitches = {"C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb","B"};
const QStringList CPitchTextConvert::flatpitchesx = {"C","Db","D","Eb","Fb","F","Gb","G","Ab","A","Bb","Cb"};
const QStringList CPitchTextConvert::notenames = naturalpitches + sharppitches + sharppitchesx + flatpitches + flatpitchesx;
