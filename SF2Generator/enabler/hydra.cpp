     /*********************************************************************
     
     hydra.cpp
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
//*****************************************************************************
//
//                          Copyright (c) 1994
//               E-mu Systems Proprietary All rights Reserved.
//                             
//*****************************************************************************

/////////////////////////////
//        Includes         //
/////////////////////////////

#include "hydra.h"


//*****************************************************************************
// @(#)hydra.cpp	1.2 09:40:33 3/21/95 09:40:33 
// Description: 
//     This file defines the datatype HydraFont. The datatype HydraFont
// is the base level in memory datatype used to describe a single
// SoundFont bank. 
// 
//*****************************************************************************

///////////////////////////////////////////
// Set all of the default SoundFont values
///////////////////////////////////////////
void  HydraClass::ResetDefault()
{

 // Sample
 sfDefault.dwStart                  =      0;
 sfDefault.dwEnd                    =      0;
 sfDefault.dwStartloop              =      0;
 sfDefault.dwEndloop                =      0;
 sfDefault.dwSampleRate             =      0;
 sfDefault.shOrigKeyAndCorr         = 0x3C00;
 sfDefault.shSampleModes            =      0;

 // Pitch
 sfDefault.shScaleTuning               =    100;  // 100 semitones/keynum
 sfDefault.shCoarseTune                =      0;
 sfDefault.shFineTune                  =      0;
 sfDefault.shModLfoToPitch             =      0;
 sfDefault.shVibLfoToPitch             =      0;
 sfDefault.shModEnvToPitch             =      0;

 // Filter

 sfDefault.shInitialFilterFc         =  13500;    // > 20 kHz
 sfDefault.shInitialFilterQ          =      0;     
 sfDefault.shModLfoToFilterFc        =      0;   
 sfDefault.shModEnvToFilterFc        =      0;  

 // Amplifier...

 sfDefault.shInstVol                 =      0;   // no attenuation
 sfDefault.shModLfoToVolume          =      0; 

 // Effects...

 sfDefault.shChorusEffectsSend       =      0;  
 sfDefault.shReverbEffectsSend       =      0; 
 sfDefault.shPanEffectsSend          =      0;

 // Modulation Low Frequency Oscillator

 sfDefault.shDelayModLfo               = -12000;  // < 1 ms
 sfDefault.shFreqModLfo                =      0;

 // Vibrato (Pitch only) Low Frequency Oscillator

 sfDefault.shDelayVibLfo               = -12000;  // < 1 ms
 sfDefault.shFreqVibLfo                =      0;  

 // Modulation Envelope

 sfDefault.shDelayModEnv               = -12000;  // < 1 ms
 sfDefault.shAttackModEnv              = -12000;  // < 1 ms
 sfDefault.shHoldModEnv                = -12000;  // < 1 ms
 sfDefault.shDecayModEnv               = -12000;  // < 1 ms
 sfDefault.shSustainModEnv             =      0;
 sfDefault.shReleaseModEnv             = -12000;  // < 1 ms
 sfDefault.shAutoHoldModEnv            =      0;
 sfDefault.shAutoDecayModEnv           =      0;

 // Attenuation (Volume only) Envelope

 sfDefault.shDelayVolEnv               = -12000;  // < 1 ms
 sfDefault.shAttackVolEnv              = -12000;  // < 1 ms
 sfDefault.shHoldVolEnv                = -12000;  // < 1 ms
 sfDefault.shDecayVolEnv               = -12000;  // < 1 ms
 sfDefault.shSustainVolEnv             =      0;
 sfDefault.shReleaseVolEnv             = -12000;  // < 1 ms
 sfDefault.shAutoHoldVolEnv            =      0;
 sfDefault.shAutoDecayVolEnv           =      0;

 // Miscellaneous

 sfDefault.shKeyExclusiveClass       =      0;

 // Preserved for informational purposes

 sfDefault.shKeynum                  =      0;
 sfDefault.shVelocity                =      0;
 sfDefault.shStartAddrsCoarseOffset      =      0;
 sfDefault.shEndAddrsCoarseOffset        =      0;
 sfDefault.shStartloopAddrsCoarseOffset  =      0;
 sfDefault.shEndloopAddrsCoarseOffset    =      0;
 sfDefault.shOverridingRootKey           =     -1;

 // Place holders, not used

 sfDefault.shNOP                     =      0;
 sfDefault.shEndOper                 =      0;

}

/*============================================================================
*  VerifySFBData: Given a HydraClass object, search for internal errors.
*============================================================================
*/

bool HydraClass::VerifySFBData(uint sampleRAMSize)
{
    if(!VerifyPDTAIndices()) return false;
    if(!VerifySamplePoints(sampleRAMSize)) return false;
    return true;
}

/*============================================================================
*  VerifyPDTAIndices: Given a HydraClass object, search for bad index values
*                     in the preset data.
*============================================================================
*/
bool HydraClass::VerifyPDTAIndices()
{
    // Step 0: Make sure all struct array counts are at least the minimum size.
    if (pPHdr.size() < 2) return false;
    if (pPBag.empty()) return false;
    if (pPGen.empty()) return false;
    if (pPMod.empty()) return false;
    if (pInst.size() < 2) return false;
    if (pIBag.empty()) return false;
    if (pIGen.empty()) return false;
    if (pIMod.empty()) return false;
    if (pSHdr.empty()) return false;

    // Step 1: Look for generator or modulator enums which are not in the
    //         valid range. This is a fixable corruption, in fact it may
    //         represent a FUTURE SoundFont.
    //         If desired, change those which are exceed the maximum to NOP.
    //         Also, check out the INSTRUMENT and SAMPLE indices and verify
    //         their validity

    // Loop to struct size-2 because: 1. last index = struct size - 1,
    //                                2. last index is the terminal and
    //                                   need not be considered

    // First the PGens...
    for (ushort count = 0; count < pPGen.size() - 2; count++)
    {
        if (pPGen[count].sfGenOper == instrument)
        {
            // We have found an bad instrument index.
            if (pPGen[count].unAmt.shAmount > int(pInst.size()) - 1) return false;
        }
    }
    // Now the IGens...
    for (ushort count = 0; count < pIGen.size() - 2; count++)
    {
        if (pIGen[count].sfGenOper == sampleId)
        {
            // We have found an bad sample index.
            if (pIGen[count].unAmt.shAmount > int(pSHdr.size()) - 1) return false;
        }
    }
    //*************************************************************
    // Future loops for PMOD and IMOD will go here when necessary
    //*************************************************************
    // Step 2: Look for Preset Layer indices which exceed the maximum
    ushort lastPBag = 0;
    for (ushort count = 0; count < pPHdr.size() - 2; count ++)
    {
        // We have found a bad preset layer index.
        if ((pPHdr[count].wBagNdx > pPBag.size() - 1) || (pPHdr[count].wBagNdx < lastPBag)) return false;
        lastPBag = pPHdr[count].wBagNdx;
    }
    // Step 3: Look for bad Generator List and Modulator List Indices
    ushort lastPGen = 0;
    ushort lastPMod = 0;
    for (ushort count = 0; count < pPBag.size() - 2; count ++)
    {
        // We have found a bad Generator List index.
        if ((pPBag[count].wGenNdx > pPGen.size() - 1) || (pPBag[count].wGenNdx < lastPGen)) return false;
        lastPGen = pPBag[count].wGenNdx;
        // We have found a bad Modulator index.
        if ((pPBag[count].wModNdx > pPMod.size() - 1) || (pPBag[count].wModNdx < lastPMod)) return false;
        lastPMod = pPBag[count].wModNdx;
    }
    // Step 3: Look for Instrument split indices which exceed the maximum
    ushort lastIBag = 0;
    for (ushort count = 0; count < pInst.size() - 1; count ++)
    {
        // We have found a bad instrument split index
        if ((pInst[count].wBagNdx > pIBag.size() - 1) || (pInst[count].wBagNdx < lastIBag)) return false;
        lastIBag = pInst[count].wBagNdx;
    }
    // Step 4: Look for Instrument Generator List and Modulator List Indices which exceed the maximum.
    ushort lastIGen = 0;
    ushort lastIMod = 0;
    for (ushort count = 0; count < pIBag.size() - 2; count ++)
    {
        // We have found a bad generator index
        if ((pIBag[count].wGenNdx > pIGen.size() - 1) || (pIBag[count].wGenNdx < lastIGen)) return false;
        lastIGen = pIBag[count].wGenNdx;
        // We have found a bad modulator index
        if ((pIBag[count].wModNdx > pIMod.size() - 1) || (pIBag[count].wModNdx < lastIMod)) return false;
        lastIMod = pIBag[count].wModNdx;
    }
    // We have no index problems.
    return true;
}

/*============================================================================
*  VerifySamplePoints: Given a HydraClass object, search for bad index values
*                      sample points
*============================================================================
*/
bool HydraClass::VerifySamplePoints(uint sampleSizeInBytes)
{
    bool validity=true;
    // Here we are looking for invalid loop points. A valid samples worth
    // of loop points looks like the following (not to scale):
    //
    //        =====================================.......
    //        |       |                 |         |  46  |
    //        =====================================.......
    //       start  startloop         endloop    end   next start
    //
    // Start is the beginning of the sample data, end is the end of the sample
    // data. Start must be less startloop, which must be less than endloop
    // which must be less than end. The distance between startloop and endloop
    // should be at least 32 samples, but is not enforced by this function.
    // Finally, the start point of the following
    // sample must be at least 46 samples away from the end point of the
    // current sample.
    //
    // Sample header data need NOT be in sequencial order, so the last
    // stipulation is checked later.
    for (ushort count = 0; count < pSHdr.size()-1; count++)
    {
        validity = false;
        //DWORD start     = pSHdr[count].dwStart;
        uint end       = pSHdr[count].dwEnd;
        /*
        DWORD startloop = pSHdr[count].dwStartloop;
        DWORD endloop   = pSHdr[count].dwEndloop;

        // SamplePointOffsets may allow some of these conditions, but
        // the sample information itself does not.
        if (startloop < start) break;
        if (endloop < startloop) break;
        if (endloop < startloop) break;
        if (end < endloop) break;
        */
        // If this is a ROM sample, we cannot detect for sample size limitations
        // without specific synthesizer information.
        if (((pSHdr[count].sfSampleType & romSample) == 0) && (end > sampleSizeInBytes/sizeof(short)))
            break;
        validity = true;
    }

    if (!validity) return validity;

    // In this loop, we check for overlapping samples. IE samples which
    // by themselves are OK but lie inside the space of another sample.
    //
    //        =====================================.......
    //        |       |      32         |         |  46  |
    //        =====================================.......
    //       start  startloop         endloop    end   next start
    //
    //                       =====================================.......
    //                       |       |      32         |         |  46  |
    //                       =====================================.......
    //                       start  startloop         endloop    end   next start
    //
    // We must be able to take it for granted that all samples by themselves
    // are valid before making this check.
    // Note that overlap must not occur even in the buffer between end and
    // next start. Finally, since sequential sample header data is not
    // mandatory in a SoundFont bank, we need to use the two loop approach.
    /*
    for (WORD count = 0; count < pSHdr.size()-1; count++)
    {
        validity = FALSE;
        bool validity1=TRUE;
        for (WORD count1 = 0; count1 < pSHdr.size()-1; count1++)
        {
            validity1 = FALSE;
            if ((count != count1)
                    &&
                    (pSHdr[count].dwStart <  pSHdr[count1].dwEnd + SAMPLEBUFFER)
                    &&
                    (pSHdr[count].dwStart >= pSHdr[count1].dwStart))
                break;
            validity1 = TRUE;
        }

        if (validity1 == FALSE) break;
        validity = TRUE;
    }
    */
    return validity;
}

/******************************************************************************
*
* Function: SoundFontNavigator::GetSFNum
*
* Implemetation Notes:  Return value associated with a single bank/preset
*                       pair in the CURRENT SoundFont to outside world
*
*                       This sets up the index which will be used in the
*                       Navigate() routine. With this being a seperate call,
*                       all Navigation which is ONLY required upon a
*                       Bank/Patch select call is done once only.
*
********************************************************************************
*/
bool HydraClass::GetSFNum(ushort wBank, ushort wPatch, ushort* pwSFID)
{
    ulong64 wSize = pPHdr.size()-1;
    for (ushort wNdx = 0; wNdx < wSize; wNdx++)
    {
        if ((pPHdr[wNdx].wPresetBank == wBank) && (pPHdr[wNdx].wPresetNum  == wPatch))
        {
            *pwSFID = wNdx;
            return true;
        }
    }
    return false;
}

///////////////////////// End of HYDRA.CPP //////////////////////////
