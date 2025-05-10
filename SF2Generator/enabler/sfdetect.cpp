/*********************************************************************

     sfdetect.cpp
     
     Copyright (c) Creative Technology Ltd. 1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
     PURPOSE.
     
     *********************************************************************/
/*============================================================================
*
*                          Copyright (c) 1995
*               E-mu Systems Proprietary All rights Reserved.
*                             
*============================================================================
*/

/*============================================================================
* @(#)sfdetect.cpp	1.2 13:56:04 3/22/95 13:56:06
*                          
*  FILE :   sfdetect.cpp
*
*  Description: 
* 
*  Search the Loaded HydraClass object for internal errors, repair if
*  desired and possible
*
*============================================================================
*/
#include "sfdetect.h"
#include "sfenum.h"

/*============================================================================
*  VerifySFBData: Given a HydraClass object, search for internal errors.
*============================================================================
*/

BOOL sfDetectErrors::VerifySFBData(HydraClass *pHF, DWORD sampleRAMSize)
{
    if(VerifyPDTAIndices(pHF) == FALSE) return FALSE;
    if(VerifySamplePoints(pHF, sampleRAMSize) == FALSE) return FALSE;
    return TRUE;
}

/*============================================================================
*  VerifyPDTAIndices: Given a HydraClass object, search for bad index values
*                     in the preset data.
*============================================================================
*/
BOOL sfDetectErrors::VerifyPDTAIndices(HydraClass *pHF)
{
    // Step 0: Make sure all struct array counts are at least the minimum size.
    if (pHF->pPHdr.size() < 2) return FALSE;
    if (pHF->pPBag.size() < 1) return FALSE;
    if (pHF->pPGen.size() < 1) return FALSE;
    if (pHF->pPMod.size() < 1) return FALSE;
    if (pHF->pInst.size() < 2) return FALSE;
    if (pHF->pIBag.size() < 1) return FALSE;
    if (pHF->pIGen.size() < 1) return FALSE;
    if (pHF->pIMod.size() < 1) return FALSE;
    if (pHF->pSHdr.size() < 1) return FALSE;

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
    for (WORD count = 0; count < pHF->pPGen.size() - 2; count++)
    {
        if (pHF->pPGen[count].sfGenOper == instrument)
        {
            // We have found an bad instrument index.
            if (pHF->pPGen[count].unAmt.shAmount > (SHORT)pHF->pInst.size() - 1) return FALSE;
        }
    }
    // Now the IGens...
    for (WORD count = 0; count < pHF->pIGen.size() - 2; count++)
    {
        if (pHF->pIGen[count].sfGenOper == sampleId)
        {
            // We have found an bad sample index.
            if (pHF->pIGen[count].unAmt.shAmount > (SHORT)pHF->pSHdr.size() - 1) return FALSE;
        }
    }
    //*************************************************************
    // Future loops for PMOD and IMOD will go here when necessary
    //*************************************************************
    // Step 2: Look for Preset Layer indices which exceed the maximum
    WORD lastPBag = 0;
    for (WORD count = 0; count < pHF->pPHdr.size() - 2; count ++)
    {
        // We have found a bad preset layer index.
        if ((pHF->pPHdr[count].wBagNdx > pHF->pPBag.size() - 1) || (pHF->pPHdr[count].wBagNdx < lastPBag)) return FALSE;
        lastPBag = pHF->pPHdr[count].wBagNdx;
    }
    // Step 3: Look for bad Generator List and Modulator List Indices
    WORD lastPGen = 0;
    WORD lastPMod = 0;
    for (WORD count = 0; count < pHF->pPBag.size() - 2; count ++)
    {
        // We have found a bad Generator List index.
        if ((pHF->pPBag[count].wGenNdx > pHF->pPGen.size() - 1) || (pHF->pPBag[count].wGenNdx < lastPGen)) return FALSE;
        lastPGen = pHF->pPBag[count].wGenNdx;
        // We have found a bad Modulator index.
        if ((pHF->pPBag[count].wModNdx > pHF->pPMod.size() - 1) || (pHF->pPBag[count].wModNdx < lastPMod)) return FALSE;
        lastPMod = pHF->pPBag[count].wModNdx;
    }
    // Step 3: Look for Instrument split indices which exceed the maximum
    WORD lastIBag = 0;
    for (WORD count = 0; count < pHF->pInst.size() - 1; count ++)
    {
        // We have found a bad instrument split index
        if ((pHF->pInst[count].wBagNdx > pHF->pIBag.size() - 1) || (pHF->pInst[count].wBagNdx < lastIBag)) return FALSE;
        lastIBag = pHF->pInst[count].wBagNdx;
    }
    // Step 4: Look for Instrument Generator List and Modulator List Indices which exceed the maximum.
    WORD lastIGen = 0;
    WORD lastIMod = 0;
    for (WORD count = 0; count < pHF->pIBag.size() - 2; count ++)
    {
        // We have found a bad generator index
        if ((pHF->pIBag[count].wGenNdx > pHF->pIGen.size() - 1) || (pHF->pIBag[count].wGenNdx < lastIGen)) return FALSE;
        lastIGen = pHF->pIBag[count].wGenNdx;
        // We have found a bad modulator index
        if ((pHF->pIBag[count].wModNdx > pHF->pIMod.size() - 1) || (pHF->pIBag[count].wModNdx < lastIMod)) return FALSE;
        lastIMod = pHF->pIBag[count].wModNdx;
    }
    // We have no index problems.
    return TRUE;
}

/*============================================================================
*  VerifySamplePoints: Given a HydraClass object, search for bad index values
*                      sample points
*============================================================================
*/
BOOL sfDetectErrors::VerifySamplePoints(HydraClass *pHF, DWORD sampleSizeInBytes)
{
    BOOL validity=TRUE;
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
    for (WORD count = 0; count < pHF->pSHdr.size()-1; count++)
    {
        validity = FALSE;
        //DWORD start     = pHF->pSHdr[count].dwStart;
        DWORD end       = pHF->pSHdr[count].dwEnd;
        /*
        DWORD startloop = pHF->pSHdr[count].dwStartloop;
        DWORD endloop   = pHF->pSHdr[count].dwEndloop;

        // SamplePointOffsets may allow some of these conditions, but
        // the sample information itself does not.
        if (startloop < start) break;
        if (endloop < startloop) break;
        if (endloop < startloop) break;
        if (end < endloop) break;
        */
        // If this is a ROM sample, we cannot detect for sample size limitations
        // without specific synthesizer information.
        if (((pHF->pSHdr[count].sfSampleType & romSample) == 0) && (end > sampleSizeInBytes/sizeof(SHORT)))
            break;
        validity = TRUE;
    }

    if (validity == FALSE) return validity;

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
    for (WORD count = 0; count < pHF->pSHdr.size()-1; count++)
    {
        validity = FALSE;
        BOOL validity1=TRUE;
        for (WORD count1 = 0; count1 < pHF->pSHdr.size()-1; count1++)
        {
            validity1 = FALSE;
            if ((count != count1)
                    &&
                    (pHF->pSHdr[count].dwStart <  pHF->pSHdr[count1].dwEnd + SAMPLEBUFFER)
                    &&
                    (pHF->pSHdr[count].dwStart >= pHF->pSHdr[count1].dwStart))
                break;
            validity1 = TRUE;
        }

        if (validity1 == FALSE) break;
        validity = TRUE;
    }
    */
    return validity;
}


