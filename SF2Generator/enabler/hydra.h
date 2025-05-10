     /*********************************************************************
     
     hydra.h
     
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
#ifndef HYDRA_H_
#define HYDRA_H_

#include <stddef.h>
#include "sfdata.h"
#include "sfenum.h"
#include <vector>
#include <string>

//*****************************************************************************
// @(#)hydra.h	1.2 09:40:05 3/21/95 09:40:06
// Description: 
//
//     This file defines the datatype HydraClass. 
// HydraClass is the base level in memory datatype used to describe
// a single SoundFont bank. It also contains the Global default parameters for
// a SoundFontObject. 
// 
//*****************************************************************************

#define SF_DATASTRUCTS  9       // The # of SF data structs
#define SAMPLENAMESIZE  20      // The size of the strings in the Hydra

#define PRESETNAMESIZE  20      
#define INSTNAMESIZE    20

#define SAMPLEBUFFER    46

enum SoundFontDS {
  prstHdr = 0,
  prstBagNdx,
  prstGenList,
  prstModList,
  inst,
  instBagNdx,
  instGenList,
  instModList,
  sampHdr

};


/////////////////////////////
//SoundFont Data Structures//
/////////////////////////////

#pragma pack (push,1)

typedef struct sfVersionTag
{
  ushort wMajor;               // Reads the major ver (if present) -was uiHi
  ushort wMinor;               // Reads the minor ver              -was uiLo
} sfVersion;


typedef struct sfINFOTag
{
  sfVersion sfSFVer;
  char*     pchSoundEngine;
  char*     pchROMName;
  sfVersion sfROMVer;
  char*     pchBankName;
  char*     pchProduct;
  char*     pchCopyright;
  char*     pchDate;
  char*     pchEngineer;
} sfINFO;

// The stRanges data structure is applicable in the Generator List only.
// They are used as values for key ranges and velocity ranges.
// Typically, a Generator value is a 16 bit signed entity, whose bytes must be
// swapped on Big Endian (Byte Incoherent) machines. In other words,
// the byte ordering for _typical_ generators is NOT the same 
// between Big Endian and Little Endian systems.

// However the byte ordering for _ranges_ IS the same regardless of the
// Endian nature of the system. 

// So instead of having the sfReader detect the relatively few key ranges 
// and NOT swapping those, it swaps bytes for ALL generator values 
// and these data structures are re-designed to compensate for that.

#ifdef SF_BYTE_ORDER_BIG_ENDIAN
struct stRanges
{
  BYTE byHi;
  BYTE byLo;
};
#else
struct stRanges            // This is for both key and velocity ranges
{
  byte byLo;
  byte byHi;
};

#endif

typedef union unGenAmtTag
{
  struct stRanges stRange;    // Sometimes the generator is an stRange
  short             shAmount; //   and sometimes it is a short
} unGenAmt;

bool inline outsideRange(const ushort v, const unGenAmt& range)
{
    if (v < range.stRange.byLo) return true;
    if (v > range.stRange.byHi) return true;
    return false;
}

//// Standard list used in Phdr (Preset lists) ////
typedef struct sfPresetHdrTag
{
  char   achPresetName[PRESETNAMESIZE];      // Name of preset  
  ushort   wPresetNum;                   // The number of the preset
  ushort   wPresetBank;                 // The preset bank number
  ushort   wBagNdx;                    // The index to sfPresetBag
  uint  dwLibrary;
  uint  dwGenre;
  uint  dwMorphology;

} sfPresetHdr;



//// Standard list used in Pbag, Ibag (Layer, split lists) ////
typedef struct sfBagNdxTag

{
  ushort wGenNdx;          // Index to generator list
  ushort wModNdx;          // Index to modulator list

} sfBagNdx;


//// Standard list used in Pgen, Igen (Generator lists) ////
typedef struct sfGenListTag
{
  ushort         sfGenOper; // Single generator 
  unGenAmt     unAmt;     // Amount applied to that generator
} sfGenList;


//// Standard list used in Pmod, Imod (Modulator lists) ////
typedef struct sfModListTag
{
  ushort    wModSrcOper;     // Source modulation operator
  ushort    wModDestOper;    // Destination modulation operator
  short   shAmount;        // Source modulates destination by this amount
  ushort    wModAmtSrcOper;  // If used, source modulates destination by 
                           // THIS source's current value instead.
  ushort    wModTransOper;   // Send source through defined transform first
} sfModList;


//// Standard list used in Inst (Instrument lists) ////
typedef struct sfInstTag
{
  char  achInstName[INSTNAMESIZE];     // Name of the instrument
  ushort  wBagNdx;                      // Index to sfInstrumentBag
  //WORD  wRefCount; // This field is used in EDIT ENGINE for reference counts

  //// For Windows 3.1 and Borland's huge pointers //// 
  #ifdef EMU_WINDOWS
  DWORD dwPad0;
  DWORD dwPad1;
  #endif
} sfInst;

// A flag in the sfSampleType field of the Sample Header data structure.
// This flag indicates that the sampled waveform it uses
// is expected to be on a WaveTable ROM, and is NOT in the file.
#define romSample 0x8000

// The other defined values for the sfSampleType field of the Sample Header
// data structure.
typedef enum {

  monoSample   = 1, // The sample is a MONO sample
  rightSample  = 2, // The sample is the RIGHT sample of a stereo pair 
                    // or the first in a group of linked samples
  leftSample   = 4, // The sample is the LEFT sample of a stereo pair
  linkedSample = 8  // The sample is another in a group of linked samples
                    // the last of which should point to the sample with
                    // rightSample set.

}SFSampleLink; 


//const WORD smplHdrV2ExtensionSize    =  (3*sizeof(WORD)+sizeof(DWORD));
//const WORD smplHdrV2Size             =  36 + smplHdrV2ExtensionSize;

//// Standard list used in Shdr (Sample header lists) ////
typedef struct sfSampleHdrTag
{

  char          achSampleName[SAMPLENAMESIZE];
  uint         dwStart;          // Sample addresses
  uint         dwEnd;
  uint         dwStartloop;
  uint         dwEndloop;
  uint         dwSampleRate;     // In Hz, IE 44100, 22050, etc
  byte          byOriginalKey;    // MIDI Key, 0 to 127
  char          chFineCorrection; // Tuning correction in cents
  ushort          wSampleLink;   // index to 'next' sample in chain
  ushort          sfSampleType;  // the kind (values: see enum SFSampleLink )

  //bool          bSampleLoaded; // This field is used in RUNTIME ENGINE ONLY
                               // as a convenient flag to indicate loaded vs
                               // unloaded samples.

  //WORD  wRefCount; // This field is used in EDIT ENGINE for reference counts

} sfSampleHdr;


////////////////////////////////////////////////////
// Typedefs for working with segmented architecture
////////////////////////////////////////////////////

#if defined (EMU_WINDOWS) || defined (DOS_MEMORY)

typedef sfPresetHdr* SFPRESETHDRPTR;
typedef sfInst*       SFINSTPTR;
typedef sfBagNdx*     SFBAGNDXPTR;
typedef sfGenList*    SFGENLISTPTR;
typedef sfModList*    SFMODLISTPTR;
typedef sfSampleHdr*  SFSAMPLEHDRPTR;

///////////////////////////////////////////////////////
// Typedefs for working with non segmented architecture
///////////////////////////////////////////////////////

#else

typedef sfPresetHdr* SFPRESETHDRPTR;
typedef sfInst*      SFINSTPTR;
typedef sfBagNdx*    SFBAGNDXPTR;
typedef sfGenList*   SFGENLISTPTR;
typedef sfModList*   SFMODLISTPTR;
typedef sfSampleHdr* SFSAMPLEHDRPTR;

#endif // Memory typedef dependencies

#pragma pack(pop)

/////////////////////////////////
// DS for holding Sample Name  //
/////////////////////////////////

class HydraClass 
{
  public:
    //// Methods ////
    HydraClass()
    {
        HydraVersion.wMajor = HydraVersion.wMinor = 0;
        ResetDefault();
    }
    ~HydraClass()
    {
    }
    //const sfData* getDefault() { return &sfDefault; }
    void  SetVersion(ushort majorVersion, ushort minorVersion) 
    {
        HydraVersion.wMajor = majorVersion;
        HydraVersion.wMinor = minorVersion;
    }
    void  GetVersion(ushort *majorVersion, ushort *minorVersion) 
    {
        *majorVersion = HydraVersion.wMajor;
        *minorVersion = HydraVersion.wMinor;
    }
    std::string achBankName;

    //// The public data ////
    
    // These 9 pointers point to the in-memory image of the SoundFont file data.
    // The only differences between this image of the SoundFont data 
    // and the data as it exists in the file itself are 
    // (1) The existence of 'reference count' fields in pInst and pSHdr
    // (2) The existence of 'sample loaded' boolean field in pSHdr
    // (3) SFBanks loaded on BIG_ENDIAN drivers have appropriate bytes
    //     pre-swapped.

    std::vector<sfPresetHdr> pPHdr;
    std::vector<sfBagNdx> pPBag;
    std::vector<sfGenList> pPGen;
    std::vector<sfModList> pPMod;
    std::vector<sfInst> pInst;
    std::vector<sfBagNdx> pIBag;
    std::vector<sfGenList> pIGen;
    std::vector<sfModList> pIMod;
    std::vector<sfSampleHdr> pSHdr;

    // This contains the default SoundFont generator vector, IE what each 
    // generator value should be if NO generator of a particular type exists 
    // within all of the appropriate key/velocity SPLITS of given INSTRUMENT.
    sfData sfDefault;
    sfVersion HydraVersion;
    bool VerifySFBData(uint sampleRAMSizeInBytes);
    bool GetSFNum(ushort wBank, ushort wPatch, ushort* pwSFID);
private:
    void  ResetDefault();
    bool VerifyPDTAIndices();
    bool VerifySamplePoints(uint sampleRAMSizeInBytes);
};

#endif // HYDRA_H
