/*********************************************************************

     sfreader.h
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
     PURPOSE.
     
     *********************************************************************/

/*============================================================================
*
*                          Copyright (c) 1994
*               E-mu Systems Proprietary All rights Reserved.
*                             
*============================================================================
*/

#ifndef SFREADER_H
#define SFREADER_H

#include "hydra.h"
#include "riff.h"

/*============================================================================
* @(#)sfreader.h	1.2 13:55:39 3/22/95 13:55:40
*                          
*  FILE :   sfreader.h 
*
*  Description: 
*
*============================================================================
*/

#define PRSTHDR_SIZE      38
#define INST_SIZE         22

// Ok, these values are the size of the data (lumps) structs _in_the_file_
// That is, the storage size of a particular implementations representation
// of a structure is of course, affected by alignment constraints etc. So we
// separate the notition of the size (in bytes) of the data in the file, as
// opposed to what sizeof(sometype) returns. 

#define PRSTBAGNDX_SIZE   4
#define PRSTGENLIST_SIZE  4
#define PRSTMODLIST_SIZE  6
#define INSTBAGNDX_SIZE   4
#define INSTGENLIST_SIZE  4
#define INSTMODLIST_SIZE  6
#define SAMPHDR_SIZE      16
#define SAMPHDRV2_SIZE    46   // new version 2.0 

#define CkCount 27

typedef enum { 

    EHC_SUCCESS = 0,        /* Successful */
    EHC_INVALIDINDEX,       /* The given preset Index is out of range     */
    EHC_NOTINFOTOKEN,       /* The given token is not an info subck token */
    EHC_NOFONT,             /* Attempt to get samples without loading font */
    EHC_ALLOCATE_PMEM_ERROR,/* Memory allocation error */
    EHC_OPENFILEERROR,
    EHC_WRONGWAVETABLE,
    EHC_READFILEERROR,
    EHC_RIFFERROR

}sfReaderErrs; /* Reader Errors not found in emuerrs.h */  

class sfReader {

public:
    std::string pathName;
    sfReader();
    ~sfReader();
    HydraClass* ReadSFBFile(const std::string& sfFileName, const char *chReqdWaveTable = "\0");
    uint GetAllSampleSize() { return dwSampleSize; }
    uint GetSampleOffset() { return dwSampleOffset; }
    void SetupToFillSampleBuckets();
    void FillSampleBucket(byte * pbyBucket, uint * dwSize);

private:
    bool IsValid();
    const char* GetInfoSubCkVal(RIFFClass::RiffCkTokenType ofSubChunk );
    byte bySFStorageStatus, bySFDataLoc;
    RIFFClass tRIFF;
    std::string infoCkValues[CkCount];
    uint dwSampleSize, dwSampleOffset, dwSampleBytesCollected;
    /* Description:
  *
  *  Current number of all info sub-chunks possible as defined in
  *  "SoundFont RIFF File Format" RSC. for the infoCkValues we'll only
  *  use 10 range Ifil..Icmt found in RiffCkTokenType (riff.h)
  *  You _might_ use the others as boolean existance flags if you want...**
  */
    void storeInfoCks();
    void resetInfoCkValues();
    void resetSampleCollector();
    /*
  *  If this class is used to read multiple soundfont banks, we need to
  *  set the info chunks values array back to a known null state.
  *  Naturally, if you want the previous banks info data to remain persistent
  *  you must make a copy of the desired data prior to asking to read the
  *  next bank, since the read routine calls this private routine before
  *  storing the new info data.
  */
    HydraClass*  ReadSFBData(const char *pchReqdWaveTable);
    HydraClass* bailOut(HydraClass* hf);
    bool findToken(ushort wCurToken, const char* errMsg=nullptr);
    void closeRiff();
};

#endif
