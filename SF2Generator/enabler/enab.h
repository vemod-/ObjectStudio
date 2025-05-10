     /*********************************************************************
     
     enab.h
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
//-****************************************************************************
//
//                          Copyright (c) 1995
//               E-mu Systems Proprietary All rights Reserved.
//                             
//-****************************************************************************

#include "sfdata.h" 
#include "hydra.h"
#include "sfnav.h"

//-****************************************************************************
// @(#)enab.h	1.2 13:56:10 5/31/95 13:56:10 
//
// Description: 
//  
//   This module defines utility routines to load, navigate, and obtain
// various pieces of information from, SoundFont banks. Collectively known as
// the "enabler", these routines can be used to prototype SoundFont playback
// implementations. 
// 
//   Functionality exists to load the articulation portion (preset data) of 
// a SoundFont bank into main system memory, and to navigate the data as 
// a MIDI note on event would, and return the SoundFont parameters gathered 
// for that particular event. The units are SoundFont units and would be 
// translated into units digestable by the  target sound engine by the client.
// That is, once navigated, the rest is up to the client caller. 
// 
//   Sampled wave form data is left in the src file. The client must write
// routines to gather the sample data, and write it someplace their target
// sound engine(s) can reference it. They must then update the in memory 
// articulation data to reflect where the data has gone. Routines exist in 
// this module to find the location within the src file of any particular
// "sample", and how big it is. Armed with this knowledge the client can
// write the routines to download all of a banks sample data in short order.
// For instance, a simple method might be to start at the beginning an write
// all sample data to on board RAM of a sound board, and add a constant offset
// to all the sample address in the banks sampleHdrs. By definition, the 
// sampleHdrs are 'normalized' to the src file, zero'ed to the SampleCkOffset.
// (ie, the first Sample has a dwStart value of 0 (zero)). Functionality 
// exists in this module to obtain _the_ sample Hdrs of the articulation data, 
// as well as obtaining the SampleCkOffset from the src File. Once you
// know where the sample chunk starts, and have the i'th sample hdr, a quick
// fseek() will take you to the first sample element of the i'th sampled wave
// form data. Next, once you write the sampled wave form data to its new home
// you know the new 'real' address, and having the sampleHdr in front of you, 
// you can update the hdr. During the navigation of the Note On Event, you 
// are returned this new address in the SoundFont vector. See the routines
// sfGetSampleHDdrs() and sfGetSampleCkOffset prologues for additional details.
// 
// NOTE: 
//   Most functions return an indication of error, but place the actual 
// error value in a private object. Use sfGetError() to retrieve if desired.
// sfClearError() can be used to clear the error prior to a new call. 
// 
// Routines: 
//
//   sfReadSFBFile()
//   sfUnloadSFBank()
//   sfGetSampHdrs()
//   sfGetPresetHdrs()
//   sfGetSMPLOffset() 
//   sfNav()
//   sfGetError()
//   sfClearError()
// 
// 
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
// MW                     May '95      Initial Creation, Enabler
//-***************************************************************************

#ifndef ENAB_H
#define ENAB_H

class CSF2Enabler
{
public:
    CSF2Enabler();
    ~CSF2Enabler();
    bool  ReadSFBFile(const std::string& bankFileName);
    bool UnloadSFBank();
    const std::vector<sfSampleHdr> GetSampHdrs();
    const std::vector<sfPresetHdr> GetPresetHdrs();
    const std::vector<sfData> Nav(ushort MidiBank, ushort MidiPreset, ushort MidiNote, ushort KeyVelo);
    const std::vector<sfData> Nav(ushort MidiProgram, ushort MidiNote, ushort KeyVelo);
    std::vector<short> sampleData;
private:
    bool HydraLoaded() { return (m_Hydra != nullptr); }
    SoundFontNavigator navit;
    HydraClass* m_Hydra;
};

#endif
