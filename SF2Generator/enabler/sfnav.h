     /*********************************************************************
     
     sfnav.h
     
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

/*****************************************************************************
*  @(#)sfnav.h	1.1 12:06:33 3/15/95 12:06:37
* 
* Filename: sfnav.h
*
* Description: The SoundFont Navigator
*
*******************************************************************************
*/

#ifndef SFNAV_H
#define SFNAV_H

#include "sfdata.h"
#include "hydra.h"
#include "sflookup.h"

class SoundFontNavigator 
{
  public:
    SoundFontNavigator() : phfNav(nullptr) {}
    ~SoundFontNavigator() {}
    const std::vector<sfData> Navigate(ushort wSFID, ushort wKey, ushort wVel);
    /*
    void GetHydraFont(HydraClass* pHydra)
    {
        pHydra = phfNav;
        return;
    }
    */
    HydraClass* SetHydraFont(HydraClass* pHydra)
    {
        HydraClass * pOldHydra = phfNav;
        phfNav = pHydra;
        return pOldHydra;
    }
  private:
    void ProcessSampleLinks();
    void AddSoundFonts(sfData*, sfData*);
    HydraClass *phfNav;
    std::vector<sfData> sfVector;  // Specific data for an oscillator
    std::vector<ushort> shdrIndexLinks;
    ushort nextOscLinkCheck;
    std::vector<byte> linkFound;
    CSF2Lookup sfLookup;

};



#endif //  SFNAV_H

