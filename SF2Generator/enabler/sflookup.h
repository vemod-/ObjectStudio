     /*********************************************************************
     
     sflookup.h
     
     Copyright (c) Creative Technology Ltd. 1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
#ifndef SFLOOKUP_H
#define SFLOOKUP_H

#include <stddef.h>
#include "softsynthsdefines.h"
#include "sfenum.h"

typedef ushort SFLOOKUP;

class CSF2Lookup
{
public:
    CSF2Lookup()
    {
        for (ushort count = 0; count < endOper + 5; count++) sfLookup[count] = offsetof(sfData, shNOP);

        sfLookup[0] = offsetof(sfData, dwStart);            /* startAddrs */
        sfLookup[1] = offsetof(sfData, dwEnd);              // endloopAddrs
        sfLookup[2] = offsetof(sfData, dwStartloop);        // endAddrs
        sfLookup[3] = offsetof(sfData, dwEndloop);          // startloopAddrs
        sfLookup[4] = offsetof(sfData, shStartAddrsCoarseOffset);
        sfLookup[5] = offsetof(sfData, shModLfoToPitch);       // lfo1ToPitch
        sfLookup[6] = offsetof(sfData, shVibLfoToPitch);       // lfo2ToPitch
        sfLookup[7] = offsetof(sfData, shModEnvToPitch);       // env1ToPitch

        //// Filter ////
        sfLookup[8] = offsetof(sfData, shInitialFilterFc);   // shnitialFilterFc
        sfLookup[9] = offsetof(sfData, shInitialFilterQ);    // shnitialFilterQ
        sfLookup[10] = offsetof(sfData, shModLfoToFilterFc);    // lfo1ToFilterFc
        sfLookup[11] = offsetof(sfData, shModEnvToFilterFc);    // env1ToFilterFc

        //// Amplifier ////
        sfLookup[12] = offsetof(sfData, shEndAddrsCoarseOffset);
        sfLookup[13] = offsetof(sfData, shModLfoToVolume);      // lfo1ToVolume

        //// Effects ////
        sfLookup[15] = offsetof(sfData, shChorusEffectsSend); // chorusEffectsSend
        sfLookup[16] = offsetof(sfData, shReverbEffectsSend); // reverbEffectsSend
        sfLookup[17] = offsetof(sfData, shPanEffectsSend);    // panEffectsSend

        //// Main LFO1 ////
        sfLookup[21] = offsetof(sfData, shDelayModLfo);         // delayModLfo
        sfLookup[22] = offsetof(sfData, shFreqModLfo);          // freqModLfo

        //// Aux VolEnv ////
        sfLookup[23] = offsetof(sfData, shDelayVibLfo);         // delayVibLfo
        sfLookup[24] = offsetof(sfData, shFreqVibLfo);         // freqVibLfo

        //// Envelope1 ////
        sfLookup[25] = offsetof(sfData, shDelayModEnv);         // delayModEnv
        sfLookup[26] = offsetof(sfData, shAttackModEnv);        // attackModEnv
        sfLookup[27] = offsetof(sfData, shHoldModEnv);          // holdModEnv
        sfLookup[28] = offsetof(sfData, shDecayModEnv);         // decayModEnv
        sfLookup[29] = offsetof(sfData, shSustainModEnv);       // sustainModEnv
        sfLookup[30] = offsetof(sfData, shReleaseModEnv);       // releaseModEnv
        sfLookup[31] = offsetof(sfData, shAutoHoldModEnv);      // autoHoldModEnv
        sfLookup[32] = offsetof(sfData, shAutoDecayModEnv);     // autoDecayModEnv

        //// Envelope2 ////
        sfLookup[33] = offsetof(sfData, shDelayVolEnv);         // delayVolEnv
        sfLookup[34] = offsetof(sfData, shAttackVolEnv);        // attackVolEnv
        sfLookup[35] = offsetof(sfData, shHoldVolEnv);          // holdVolEnv
        sfLookup[36] = offsetof(sfData, shDecayVolEnv);         // decayVolEnv
        sfLookup[37] = offsetof(sfData, shSustainVolEnv);       // sustainVolEnv
        sfLookup[38] = offsetof(sfData, shReleaseVolEnv);       // releaseVolEnv
        sfLookup[39] = offsetof(sfData, shAutoHoldVolEnv);      // autoHoldVolEnv
        sfLookup[40] = offsetof(sfData, shAutoDecayVolEnv);     // autoDecayVolEnv

        //// Preset Data ////
        sfLookup[45] = offsetof(sfData, shStartloopAddrsCoarseOffset);
        sfLookup[46] = offsetof(sfData, shKeynum);
        sfLookup[47] = offsetof(sfData, shVelocity);
        sfLookup[48] = offsetof(sfData, shInstVol);
        sfLookup[50] = offsetof(sfData, shEndloopAddrsCoarseOffset);
        sfLookup[51] = offsetof(sfData, shCoarseTune);
        sfLookup[52] = offsetof(sfData, shFineTune);
        sfLookup[54] = offsetof(sfData, shSampleModes);
        sfLookup[55] = offsetof(sfData, shOrigKeyAndCorr);
        sfLookup[56] = offsetof(sfData, shScaleTuning);
        sfLookup[57] = offsetof(sfData, shKeyExclusiveClass);
        sfLookup[58] = offsetof(sfData, shOverridingRootKey);
    }
    inline SFLOOKUP operator[] (const ushort index) const { return sfLookup[index]; }
private:
    SFLOOKUP sfLookup[endOper + 5];
};

#endif
