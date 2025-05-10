/*********************************************************************

     sfnav.cpp
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
     PURPOSE.
     
     *********************************************************************/
/******************************************************************************
*
*     Copyright (c) E-mu Systems, Inc. 1994. All rights Reserved.
*
*******************************************************************************
*/

/******************************************************************************
*  @(#)sfnav.cpp	1.1 12:06:45 3/15/95 12:06:47
*
* Filename: sfnav.cpp
*
* Description:  The SoundFont Navigator and related methods
*
******************************************************************************
*/

/**************
* Includes
**************/

#include "sfnav.h"
#include "sfenum.h"
#include "sfdata.h"
#include "hydra.h"

/******************************************************************************
*
* Function: SoundFontNavigator::Navigate
*                               For SoundFont 2.0
*
* Implemetation Notes:  Get articulation data from a SoundFont in memory
*
*******************************************************************************
*/

const std::vector<sfData> SoundFontNavigator::Navigate(ushort uiSFID, ushort uiKey, ushort uiVel)
{
    sfVector.clear();
    shdrIndexLinks.clear();
    linkFound.clear();
    if (phfNav == nullptr) return sfVector;
    if (uiSFID > (phfNav->pPHdr.size() - 1)) return sfVector;

    //////////////////////////////////////////////////////////
    // We begin navigation at the Preset Header index
    // set up by GetSFNum() and stored in uiSFID, in the SoundFont
    // instance (or HydraClass pointer) setup by SetHydraFont()
    // and stored in phfNav. See SetHydraFont(), GetSFNum()
    //////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////
    // The distance from pPHdr[uiSFID] to pPHdr[uiSFID + 1]
    // indicates the number of LAYERS for the current PRESET
    //////////////////////////////////////////////////////////
    const ushort uiPBagNdxL = phfNav->pPHdr[uiSFID].wBagNdx;     // Set the indices for this
    const ushort uiPBagNdxH = phfNav->pPHdr[uiSFID + 1].wBagNdx; //   preset number
    //WORD wOsc = 0;                          // Reset the # of Osc's needed

    // Assign the key and velocity values to the default vector
    phfNav->sfDefault.shKeynum   = short(uiKey);
    phfNav->sfDefault.shVelocity = short(uiVel);

    //// Preset Layers ////

    //////////////////////////////////////////////////////////
    // Clear current layer data
    // Being ADDITIVE RELATIVE generators, the default value for all
    // PRESET generators MUST be the additive identity (0)
    //////////////////////////////////////////////////////////
    sfData sfPresetData; // The preset global layer data
    zeroMemory(&sfPresetData, sizeof(sfPresetData));
    nextOscLinkCheck = 0;

    for (ushort pbagIndex = uiPBagNdxL; pbagIndex < uiPBagNdxH; pbagIndex++)
    {
        //////////////////////////////////////////////////////////
        // The distance from pPBag[pbagIndex] to
        // pPBag[pbagIndex + 1] indicates the
        // number of GENERATORS for the current LAYER
        //////////////////////////////////////////////////////////
        const ushort uiPGenNdxL = phfNav->pPBag[pbagIndex].wGenNdx;
        const ushort uiPGenNdxH = phfNav->pPBag[pbagIndex + 1].wGenNdx;
        bool bGlobalPrstParams = true;

        //// Set the Current SoundFont Layer to the Default ////
        sfData sfCurrPreset = sfPresetData;

        //// Preset Generator operators ////

        for (ushort pgenIndex = uiPGenNdxL; pgenIndex < uiPGenNdxH; pgenIndex++)
        {
            // Assign a pointer to the position of the current generator.
            // Reference the data from this pointer in the future code.
            // Makes for smaller and faster code.
            SFGENLISTPTR psfCurrPGen = &(phfNav->pPGen[pgenIndex]);
            const ushort uiGenOper = psfCurrPGen->sfGenOper;
            const short iGenAmt   = psfCurrPGen->unAmt.shAmount;

            /////////////////////////////////////////////////////
            // Overriding keynum/velocity found at LAYER level
            // Overriding keynum/velocity here affects LAYER and
            // future INSTRUMENT split decisions ONLY! They are NOT
            // RELATIVE values to be added to the INSTRUMENT
            // or DEFAULT keynum/velocity
            /////////////////////////////////////////////////////
            if (uiGenOper == keynum)
                ;//uiKey = iGenAmt;

            else if (uiGenOper == velocity)
                ;//uiVel = iGenAmt;

            /////////////////////////////////////////////////////
            // Check the key and velocity ranges
            /////////////////////////////////////////////////////

            else if ((uiGenOper == keyRange) && (outsideRange(uiKey,psfCurrPGen->unAmt)))// ((uiKey < psfCurrPGen->unAmt.stRange.byLo) || (uiKey > psfCurrPGen->unAmt.stRange.byHi)))
            {
                /////////////////////////////////////////////////////
                // A keyrange was detected, however the current key does not lie
                // within that range. Off to the next PBAG.
                /////////////////////////////////////////////////////
                bGlobalPrstParams = false;
                break;
            }

            else if ((uiGenOper == velRange) && (outsideRange(uiVel,psfCurrPGen->unAmt)))// ((uiVel < psfCurrPGen->unAmt.stRange.byLo) || (uiVel > psfCurrPGen->unAmt.stRange.byHi)))

            {
                /////////////////////////////////////////////////////
                // A velocity range was detected, however the current velocity does
                // not lie within that range. Off to the next PBAG.
                /////////////////////////////////////////////////////
                bGlobalPrstParams = false;
                break;
            }

            else if (uiGenOper == instrument)
            {

                //////////////////////////////////////////////////////////
                // sGenAmt equals the Instrument token, that means
                // an instrument was found within the layer. This is the terminal
                // token which contains information for all samples and split level
                // articulation parameters
                //////////////////////////////////////////////////////////

                //////////////////////////////////////////////////////////
                // The distance from pInst[iGenAmt] to
                // pInst[iGenAmt + 1] indicates the
                // number of GENERATOR SPLITS for the current INSTRUMENT
                //////////////////////////////////////////////////////////
                const ushort uiIBagNdxL = phfNav->pInst[ulong64(iGenAmt)].wBagNdx;
                const ushort uiIBagNdxH = phfNav->pInst[ulong64(iGenAmt) + 1].wBagNdx;

                //// Assign the current vector pointer ////
                //sfVector.resize(wOsc+1);
                //sfData* psfVectorCurrOsc = &sfVector[wOsc];

                // Set the current vector to the DEFAULT vector
                sfData psfVectorCurrOsc = phfNav->sfDefault;
                ushort shdrIndexLink=0;

                // Set overriding key/vel given at the layer
                psfVectorCurrOsc.shKeynum   = short(uiKey);
                psfVectorCurrOsc.shVelocity = short(uiVel);

                // Copy current to default instrument
                // This data vector contains (or will contain) current default
                // generator values plus the possibly upcoming Global Split generators.
                sfData sfInstData = psfVectorCurrOsc;

                for (ushort ibagIndex = uiIBagNdxL; ibagIndex < uiIBagNdxH; ibagIndex++)
                {

                    //////////////////////////////////////////////////////////
                    // The distance from pInst[iGenAmt] to
                    // pInst[iGenAmt + 1] indicates the
                    // number of GENERATORS for the current SPLIT
                    //////////////////////////////////////////////////////////
                    const ushort uiIGenNdxL = phfNav->pIBag[ibagIndex].wGenNdx;
                    const ushort uiIGenNdxH = phfNav->pIBag[ibagIndex + 1].wGenNdx;
                    bool bGlobalInstParms = true;

                    //// Check for instrument Multisamples Generator Operators ////

                    for (ushort igenIndex = uiIGenNdxL; igenIndex < uiIGenNdxH; igenIndex++)
                    {
                        // Assign a pointer to the position of the current generator.
                        // Reference the data from this pointer in the future code.
                        // Makes for smaller and faster code.
                        SFGENLISTPTR psfCurrIGen = &(phfNav->pIGen[igenIndex]);

                        //// uiIBagNdx is an index to the phfNav->pIGen array ////
                        const ushort uiInstGenOper = psfCurrIGen->sfGenOper;
                        const short iInstGenAmt   = psfCurrIGen->unAmt.shAmount;

                        // Check for overriding key/vel

                        if (uiInstGenOper == keynum)
                            uiKey = ushort(iGenAmt);
                        else if (uiInstGenOper == velocity)
                            uiVel = ushort(iGenAmt);

                        //// Check for appropriate key and velocity ranges ////

                        if ((uiInstGenOper == keyRange) || (uiInstGenOper == velRange))
                        {
                            ///////////////////////////////////////////////
                            // If we have several keyRanges/velRanges,   //
                            // and the range does not qualify, we do NOT //
                            // want the sfVector[wOsc] copied to the    //
                            // sfInstData vector, hence:                 //
                            // bGlobalInstParms = FALSE;                 //
                            ///////////////////////////////////////////////

                            if (uiInstGenOper == keyRange)
                            {
                                if (outsideRange(uiKey,psfCurrIGen->unAmt))// ((uiKey < psfCurrIGen->unAmt.stRange.byLo) || (uiKey > psfCurrIGen->unAmt.stRange.byHi))
                                {
                                    bGlobalInstParms = false;
                                    break;
                                }
                            }

                            else if (outsideRange(uiVel,psfCurrIGen->unAmt))// ((uiVel < psfCurrIGen->unAmt.stRange.byLo) || (uiVel > psfCurrIGen->unAmt.stRange.byHi))
                            {
                                bGlobalInstParms = false;
                                break;
                            }

                            //////////////////////////////////////////////////////
                            // An instrument listed in Preset layer was found
                            // Since an instrument is DEFINED as the terminal
                            // token for a single layer, this assignment avoids
                            // redundant future loops
                            //////////////////////////////////////////////////////
                            pgenIndex = uiPGenNdxH;
                        }

                        // Check for sampleId
                        else if (uiInstGenOper == sampleId)
                        {

                            //////////////////////////////////////////////////////
                            // A sample ID was found. This is the terminal token for a
                            // single split which contains the index to the sample
                            // information.
                            //////////////////////////////////////////////////////

                            // Assign a pointer to the position of the current sample header.
                            // Reference the data from this pointer in the future code.
                            // Makes for smaller and faster code.
                            SFSAMPLEHDRPTR pshSHdrCurrSmpl = &phfNav->pSHdr[ulong64(iInstGenAmt)];

                            //////////////////////////////////////////////////////
                            // Establish sample links
                            // Build an array of links based on SoundFont
                            // sample header indices, which will (later) be
                            // converted to an array based on OUTPUT VECTOR
                            // indices. (ProcessSampleLinks)
                            //////////////////////////////////////////////////////
                            if ((pshSHdrCurrSmpl->sfSampleType&0x7FFF) > monoSample)
                            {
                                psfVectorCurrOsc.shSampleLink = short(pshSHdrCurrSmpl->wSampleLink);
                                shdrIndexLink = ushort(iInstGenAmt);
                                psfVectorCurrOsc.shSampleModes |= LINKED;
                                if (pshSHdrCurrSmpl->sfSampleType == rightSample) psfVectorCurrOsc.shSampleModes |= FIRST_LINK;
                            }
                            else
                            {
                                psfVectorCurrOsc.shSampleLink = 0;
                                psfVectorCurrOsc.shSampleModes &= ~(FIRST_LINK | LINKED);
                            }

                            // Set the sample addresses accounting for coarse and fine
                            // address offsets

                            psfVectorCurrOsc.dwStart += (ushort(psfVectorCurrOsc.shStartAddrsCoarseOffset) * 0x8000) + pshSHdrCurrSmpl->dwStart;

                            psfVectorCurrOsc.dwEnd += (ushort(psfVectorCurrOsc.shEndAddrsCoarseOffset) * 0x8000) + pshSHdrCurrSmpl->dwEnd;

                            psfVectorCurrOsc.dwStartloop += (ushort(psfVectorCurrOsc.shStartloopAddrsCoarseOffset) * 0x8000) + pshSHdrCurrSmpl->dwStartloop;

                            psfVectorCurrOsc.dwEndloop += (ushort(psfVectorCurrOsc.shEndloopAddrsCoarseOffset) * 0x8000) + pshSHdrCurrSmpl->dwEndloop;

                            // Obtain the sample rate, root key and sample tuning correction

                            psfVectorCurrOsc.dwSampleRate = pshSHdrCurrSmpl->dwSampleRate;

                            short originalKey;

                            if ((psfVectorCurrOsc.shOverridingRootKey >= 0) && (psfVectorCurrOsc.shOverridingRootKey < 128))
                                originalKey = psfVectorCurrOsc.shOverridingRootKey;
                            else if (pshSHdrCurrSmpl->byOriginalKey < 128)
                                originalKey = pshSHdrCurrSmpl->byOriginalKey;
                            else
                                originalKey = 60;

                            psfVectorCurrOsc.shOrigKeyAndCorr = (byte(originalKey) << 8) | (pshSHdrCurrSmpl->chFineCorrection & 0xFF);

                            // Add in preset level changes
                            // Result is in psfVectorCurrOsc.
                            AddSoundFonts(&psfVectorCurrOsc, &sfCurrPreset);

                            //wOsc++;
                            sfVector.push_back(psfVectorCurrOsc);
                            shdrIndexLinks.push_back(shdrIndexLink);
                            linkFound.push_back(0);

                            // Let's not do this if we are certain of not getting another
                            // vector in this bag.
                            if (ibagIndex != (uiIBagNdxH - 1))
                            {
                                // Copy the contents of the DEFAULT INSTRUMENT generators
                                // to the NEXT Vector.
                                psfVectorCurrOsc = sfInstData;
                            }

                            // Reset previous overriding keynumbers
                            uiKey = ushort(sfInstData.shKeynum);
                            uiVel = ushort(sfInstData.shVelocity);

                            ///////////////////////////////////////////////////
                            // This navigator automatically handles sample loop
                            // point offsets and root key definitions. So we now
                            // zero out those parameters in previous vector which
                            // have been already processed
                            ///////////////////////////////////////////////////

                            sfVector.back().shStartAddrsCoarseOffset = 0;
                            sfVector.back().shEndAddrsCoarseOffset = 0;
                            sfVector.back().shStartloopAddrsCoarseOffset = 0;
                            sfVector.back().shEndloopAddrsCoarseOffset = 0;
                            sfVector.back().shOverridingRootKey = -1;

                            bGlobalInstParms = false;

                        }
                        else
                        {
                            ///////////////////////////////////////////////////
                            // A single step lookup/assignment to get the
                            // generator value into the output vector.
                            // NOTE this will cause problems if the generator
                            // value exceeds the maximum defined value within
                            // this software! Hence the sanity checking at load
                            // time for illegal generators.
                            ///////////////////////////////////////////////////

                            // This step necessary for Big Endian systems
                            if (uiInstGenOper <= endloopAddrsOffset)
                                *reinterpret_cast<int*>(reinterpret_cast<byte*>(&psfVectorCurrOsc) + sfLookup[uiInstGenOper]) = int(iInstGenAmt);
                            else
                                *reinterpret_cast<short*>(reinterpret_cast<byte*>(&psfVectorCurrOsc) + sfLookup[uiInstGenOper]) = iInstGenAmt;
                        }
                    } // Loop through all Generators in a SPLIT

                    /////////////////////////////////////////////////////////////
                    // A future loop for SoundFont MODULATOR SPLITS
                    // will be inserted here
                    // IE from uiIModNdxL = phfNav->pIBag[ibagIndex].wModNdx;
                    //    to   uiIModNdxH = phfNav->pIBag[ibagIndex + 1].wModNdx;
                    /////////////////////////////////////////////////////////////

                    ///////////////////////////////////////////////////
                    // If the split we found was a GLOBAL split, then
                    // its parameters must be INCLUDED in the DEFAULT
                    // INSTRUMENT vector also.
                    ///////////////////////////////////////////////////
                    if (bGlobalInstParms) sfInstData = psfVectorCurrOsc;
                } // Loop through all SPLITS of Generators and Modulators

                ///////////////////////////////////////////////////////////////////
                // Here the sample links in the previous instrument are converted
                // from Sample Header indices (0 to number of SAMPLES in the BANK)
                // to output vector indices (0 to MAX_SAMPLES-1)
                // Note only samples within a single instrument may be linked!
                ///////////////////////////////////////////////////////////////////
                ProcessSampleLinks();

                // If we found an Instrument, this is NOT the global layer.
                bGlobalPrstParams = false;

            } // Instrument found

            else
            {
                ///////////////////////////////////////////////////
                // A single step lookup/assignment to get the
                // generator value into the output vector.
                // NOTE this will cause problems if the generator
                // value exceeds the maximum defined value within
                // this software! Hence the sanity checking at load
                // time for illegal generators.
                ///////////////////////////////////////////////////

                // Necessary for 'byte-incoherent' systems
                if (uiGenOper <= endloopAddrsOffset)
                    *reinterpret_cast<int*>(reinterpret_cast<byte*>(&sfCurrPreset) + sfLookup[uiGenOper]) = int(iGenAmt);
                else
                    *reinterpret_cast<short*>(reinterpret_cast<byte*>(&sfCurrPreset) + sfLookup[uiGenOper]) = iGenAmt;
            }

        } // Loop through all Generators in a LAYER

        //// Preset Modulator operators ////

        ///////////////////////////////////////////////////////////////
        // A future loop for SoundFont MODULATOR LAYERS
        // will be inserted here
        // IE from uiPModNdxL = phfNav->pPBag[pbagIndex].wModNdx;
        //    to   uiPModNdxH = phfNav->pPBag[pbagIndex + 1].wModNdx;
        ///////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////
        // If the layer we found was the GLOBAL layer, then
        // its parameters must be INCLUDED in the DEFAULT
        // LAYER vector also.
        // Overriding key/vel NOT found at the GLOBAL
        // LAYER should be restored to the input values.
        // Overriding key/vel FOUND at the GLOBAL LAYER
        // should REPLACE the input values
        ///////////////////////////////////////////////////
        if (bGlobalPrstParams) sfPresetData = sfCurrPreset;

    } // Loop through all LAYERS of Generators and Modulators

    // The navigation process is finished.
    return sfVector;
}

/******************************************************************************
*
* Function: SoundFontNavigator::ProcessSampleLinks
*
* Implemetation Notes:  Convert sample ID based links to vector index based
*                       links
*******************************************************************************
*/

void SoundFontNavigator::ProcessSampleLinks()
{
    ushort saveStart = nextOscLinkCheck;
    std::vector<ushort> vectorIndexLinks(sfVector.size());
    //
    // The idea here is that a LINKED sampled group is defined as
    // two or more samples which WERE FOUND within the SPLIT or LAYER lists in
    // a preset and whose sfSampleTypes field was set to leftSample, or
    // rightSample, or linkSample, AND whose sfSampleLink fields circularly
    // point to each other.
    //
    // A STEREO Pair is a Linked sample group of TWO samples.
    // Surround-sound could be implemented as a linked sample group of
    // FIVE (or six) samples (if supporting hardware exists...)
    //
    // The ONLY audio feature that the STEREO/LINKED samples specifies is
    // that their pitch amounts must be exactly the same at all times.
    // (Equal sample rates, NO phase difference between the samples!)
    //
    // This means one oscillator controls the pitch amounts and pitch
    // modulation for all of the linked oscillators. Stereo/Linked
    // samples do NOT need to have specific pan values (although typically
    // a stereo pair would be panned right/left) or filter values or whatever.
    // So you CAN have a "stereo" pair where one sample is being filtered
    // differently or has more trememlo (LFO->volume) than the other.
    //
    // In a stereo or linked sample list, the RIGHT sample controls the pitch,
    // the LEFT or LINKED samples follows the pitch of the RIGHT sample.
    // So in effect, any generators or modulators found in a split with
    // a LEFT or LINKED sample which modulate pitch in any way  (IE:
    // LFO->pitch, Env->pitch, etc) are NOT used if the stereo pair or the
    // entire link is found in naviagtion.
    //
    // This is a linked sample group.
    // Two vectors (with one sample each) were found upon naviation. (wOsc == 2)
    // Therefore it is a stereo pair.
    // Sample1 is Right sample, link points to Sample2
    // Sample2 is Left sample,  link points to Sample1
    //
    // This is NOT a linked sample group (nor is it a stereo pair)
    // Two vectors (with one sample each) were found upon naviation. (wOsc == 2)
    // Sample1 is Right sample, link points to Sample2
    // Sample3 is Mono sample,  link point is undefined
    // The samples do not point to each other
    //
    // This is NOT a linked sample group (nor is it a stereo pair)
    // Two vectors (with one sample each) were found upon naviation. (wOsc == 2)
    // There are two vectors (wOsc == 2), one with Sample2, other with Sample4
    // Sample2 is Left sample,  link points to Sample1
    // Sample4 is Right sample,  link points to Sample5
    // The samples are Left and Right samples but NOT to each other
    //
    // IE: This is a LINKED sample group
    // Four vectors (with one sample each) were found upon naviation. (wOsc == 4)
    // Sample10 is Right sample, link points to Sample11
    // Sample11 is Linked sample, link points to Sample12
    // Sample12 is Linked sample, link points to Sample13
    // Sample13 is Linked sample,  link points to Sample10
    //
    // IE: This is NOT a LINKED sample group
    // Three vectors (with one sample each) were found upon naviation. (wOsc == 3)
    // Sample10 is Right sample, link points to Sample11
    // Sample11 is Linked sample, link points to Sample12
    // Sample12 is Linked sample, link points to Sample13
    // The circle is not complete.
    //

    // What is happening here is that all vectors are being scanned to see
    // if stereo and/or linked samples exist AND point to each other

    // The array 'shdrIndexLinks' was filled by the Navigate() routine. It
    // contains a list of which SAMPLE INDEX each
    // vector's sample points to. A second array called 'vectorIndexLinks' is
    // stuffed with the corresponding VECTOR indices, thus leaving all data
    // in tact until all processing is completed.

    for (ushort count = nextOscLinkCheck; count < sfVector.size(); count++, nextOscLinkCheck++)
    {
        bool found = false;
        // Is this a linked sample?
        if (sfVector[count].shSampleModes & LINKED)
        {
            // If so, find its link
            for (ushort count1 = saveStart; count1 < sfVector.size(); count1++)
            {
                if ((sfVector[count1].shSampleModes & LINKED)              &&
                        (shdrIndexLinks[count1] == sfVector[count].shSampleLink) &&
                        (linkFound[count1] == 0))
                {
                    // Temporarily store the vector link points
                    vectorIndexLinks[count] = count1;
                    // Flag the fact that this vector has been found as a link!
                    linkFound[count1] = 1;
                    found=true;
                    break;
                }
            }

            // If there was no link, then turn off the LINK flags on the CURRENT
            // sample under consideration. Note that if a chain of linked samples
            // is found, this may not follow and reset the entire chain
            // ALL of the way back to the beginning of a list. Thus misleading
            // vectors may be produced.
            //
            // Say x points to y and y points to z and z points to x. But in this
            // navigation only x and y were found. This process will return saying
            // x is a valid link to y, but y is an invalid link.
            //
            // There is no issue with an two unlinked samples which are linked
            // elsewhere.
            //
            // Say a points to b and c points to d. In this navigation a and d
            // were found. Both a and d will come out saying they have no valid
            // links.
            if (!found)
            {
                sfVector[count].shSampleModes &= ~(FIRST_LINK|LINKED);
                sfVector[count].shSampleLink = 0;
            }
        }
    }
    // Now assign the each VECTOR's link value to the values which the
    // above stored in the 'vectorIndexLinks' array.
    for (ushort count = saveStart; count < sfVector.size(); count++)
    {
        if ((sfVector[count].shSampleModes & LINKED) == LINKED)
            sfVector[count].shSampleLink = short(vectorIndexLinks[count]);
    }
    // Now, all vectors have shSampleLink set to the values of the
    // corresponding VECTOR indices instead of the sample indices.
    // Each vector's shSampleType field has the 3rd bit set indicating
    // that it IS a link, and one vector's shSampleType field ALSO has the
    // 4th bit set indicating that is the FIRST link.

    // If the search above found that vectors with linked samples did NOT
    // have their counterparts in the final array of vectors, then all
    // the shSampleType field get bits 3 and 4 reset.
}

/*******************************************************************
* AddSoundFonts: The layer/split addition operation function. Simply
*                take all elements in the final layer vector and add 
*                them to the final split vector.
*
*                The summed vectors is returned in sfSoundFontReturned
*********************************************************************
*/
void SoundFontNavigator::AddSoundFonts(sfData * sfSoundFontReturned, sfData * sfSFPresetAdd)
{
    for (ushort wCount = 0; wCount < endOper; wCount++)
    {
        const ushort uiOffset = sfLookup[wCount];
        *reinterpret_cast<short*>(reinterpret_cast<byte*>(sfSoundFontReturned) + uiOffset) += *reinterpret_cast<short*>(reinterpret_cast<byte*>(sfSFPresetAdd) + uiOffset);
    }
}

