     /*********************************************************************
     
     sfdata.h
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
#ifndef SFDATA_H
#define SFDATA_H

/******************************************************************************
*
*     Copyright (c) E-mu Systems, Inc. 1994. All rights Reserved.
*                             
*******************************************************************************
*/

/*****************************************************************************
*  @(#)sfdata.h	1.1 12:06:31 3/15/95 12:06:36
*
* Filename: sfdata.h
*
* Description: SoundFont Data Structure
*
*******************************************************************************
*/

/////////////////////////////
//       Includes          //
/////////////////////////////

#include "softsynthsdefines.h"

/////////////////////////////
// SoundFont Data Structure//
/////////////////////////////
#pragma pack (push,1)

typedef enum sfSampleFlagsTag
{
  NOFLAG = 0,
  ZL = 1,
  ZR = 2,
  LINKED = 4,
  FIRST_LINK = 8
} sfSampleFlags;

typedef struct sfDataTag
{
  //// Oscillator ////
  uint dwStart;             //// sample start address 
  uint dwEnd;
  uint dwStartloop;         //// loop start address 
  uint dwEndloop;           //// loop end address 
  uint dwSampleRate; 
  short shOrigKeyAndCorr;
  short shSampleModes;
  short shSampleLink;

  //// Pitch ////
  short  shCoarseTune;
  short  shFineTune;
  short  shScaleTuning;
  short  shModLfoToPitch;            //// main fm: modLfo-> pitch ////
  short  shVibLfoToPitch;            //// aux fm:  vibLfo-> pitch ////
  short  shModEnvToPitch;            //// pitch env: modEnv(aux)-> pitch ////

  //// Filter ////
  short   shInitialFilterFc;        //// initial filter cutoff ////
  short   shInitialFilterQ;         //// filter Q ////
  short   shModLfoToFilterFc;         //// modLfo -> filter * cutoff ////
  short   shModEnvToFilterFc;         //// mod env(aux)-> filter * cutoff ////

  //// Amplifier ////
  short   shInstVol;                //// ////
  short   shModLfoToVolume;           //// tremolo: modLfo-> volume ////

  //// Effects ////
  short   shChorusEffectsSend;      //// chorus ////
  short   shReverbEffectsSend;      //// reverb ////
  short   shPanEffectsSend;         //// pan ////

  //// Modulation Low Frequency Oscillator ////
  short   shDelayModLfo;              //// delay 
  short   shFreqModLfo;               //// frequency ////

  //// Vibrato (Pitch only) Low Frequency Oscillator ////
  short   shDelayVibLfo;              //// delay 
  short   shFreqVibLfo;               //// frequency ////

  //// Modulation Envelope ////
  short   shDelayModEnv;              //// delay
  short   shAttackModEnv;             //// attack ////
  short   shHoldModEnv;               //// hold ////
  short   shDecayModEnv;              //// decay ////
  short   shSustainModEnv;            //// sustain ////
  short   shReleaseModEnv;            //// release ////
  short   shAutoHoldModEnv;
  short   shAutoDecayModEnv;

  //// Attenuation (Volume only) Envelope ////
  short   shDelayVolEnv;              //// delay
  short   shAttackVolEnv;             //// attack ////
  short   shHoldVolEnv;               //// hold ////
  short   shDecayVolEnv;              //// decay ////
  short   shSustainVolEnv;            //// sustain ////
  short   shReleaseVolEnv;            //// release ////
  short   shAutoHoldVolEnv;
  short   shAutoDecayVolEnv;

  //// Miscellaneous ////
  short   shKeyExclusiveClass;

  //// Preserved for informational purposes ////
  short   shKeynum;                 //// ////
  short   shVelocity;               //// ////

  //// These parameters are processed from within navigator ////
  short   shStartAddrsCoarseOffset;
  short   shEndAddrsCoarseOffset;
  short   shStartloopAddrsCoarseOffset;
  short   shEndloopAddrsCoarseOffset;
  short   shOverridingRootKey;

  //// Place holders, not used ////
  short   shNOP;
  short   shEndOper;
} sfData;

#pragma pack(pop)


#endif // SFDATA_H
//////////////////////// End of SFDATA.H ////////////////////////////
