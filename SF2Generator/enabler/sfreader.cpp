/*********************************************************************
  
     sfreader.cpp
     
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

#include "sfreader.h"

#define DEBUG

#ifdef DEBUG
#include <QDebug>
#endif

//#include <stdlib.h>
/*============================================================================
* @(#)sfreader.cpp	1.2 13:56:04 3/22/95 13:56:06
*                          
*  FILE :   sfreader.c
*
*  Description: 
* 
* Load SoundFont articulation data, (w/o sample download) and return the address
* of an allocated HydraClass object. It also maintains
* an array of character strings read from the info chunk of the RIFF file.
*
*============================================================================
*/

#define SFR_NO_DATA    0xFF
#define SFR_ONDISK     0
#define SFR_ONMACDISK  3

#define SFR_CLOSED     0
#define SFR_OPEN       1

#define MAX_READ_SIZE 0x7FFE

////////////////////////////
//  The first Constructor
////////////////////////////
sfReader::sfReader()
{
    /* Private members */
    bySFDataLoc = SFR_NO_DATA;       // Flag to indicate storage medium
    bySFStorageStatus = SFR_CLOSED; // Flag to indicate if storage medium is open
    pathName.clear();
    
    for (std::string& s : infoCkValues)  // Insures we are not 'delete'-ing any data
        s.clear();   // which was not 'new'-ed.
    
    resetSampleCollector();
} // end constructor 1


////////////////////////////
//  The Destructor
////////////////////////////
sfReader::~sfReader()
{
    closeRiff();
} /* end destructor */

/*
*****************************************************************************
*
* OpenSFB: Open up a SoundFont without loading or reading any data
*
*****************************************************************************
*/
HydraClass *sfReader::ReadSFBFile(const std::string& pFilename, const char * pchReqdWaveTable)
/*
*****************************************************************************
*
* ReadSFBFile: Setup for reading SoundFont from a file.
*****************************************************************************
*/
{
    closeRiff();
    pathName=pFilename;
    resetInfoCkValues(); // dealloc InfoCkValues from last read file, (if any).
    if (!tRIFF.OpenRIFF(pFilename.c_str())) return nullptr;
    bySFDataLoc       = SFR_ONDISK;
    bySFStorageStatus = SFR_OPEN;
    return (ReadSFBData(pchReqdWaveTable));
}

void sfReader::closeRiff()
{
    if (bySFStorageStatus == SFR_OPEN)
    {
        tRIFF.CloseRIFF();
        bySFStorageStatus = SFR_CLOSED;
    }
}

HydraClass* sfReader::bailOut(HydraClass *hf)
{
    delete hf;
    closeRiff();
    return nullptr;
}

bool sfReader::findToken(ushort wCurToken,const char* errMsg)
{
    if (!tRIFF.FindCk(tRIFF.GetRIFFToken(wCurToken)))
    {
#ifdef DEBUG
        if (errMsg) qDebug()<< errMsg << " in " << pathName.c_str();
#endif
        return false;
    }
    return true;
}

HydraClass* sfReader::ReadSFBData (const char * pchReqdWaveTable)
/*****************************************************************************
* 
* Implementation Notes: 
*
*    This file reads the given SoundfontBank file into memory as a HydraClass
* object. Uses the class RIFFClass object used to read up the file and
* move through the various Chunks and sub chunks. It stores all the info
* chunk stuff in a global infoCkValues array, and the preset data into 
* a hydraClass object, it sticks the names into the last SampleHdr array 
* of the hydrafont.
*
*****************************************************************************
*/

{
    HydraClass *hf;
    
    bool      bROMSamples = false;
    
    if ((hf = new HydraClass) == nullptr) return nullptr;
    tRIFF.Reset();
    resetSampleCollector();
    /*
   * Verify SoundFont data format
   */
    if ( (!descriptorMatch(tRIFF.GetCkID(),tRIFF.GetRIFFToken(RIFFClass::Riff) ) &&
          (!descriptorMatch(tRIFF.GetCkFormID(),tRIFF.GetRIFFToken(RIFFClass::Sfbk)) )))
    {
        if(!descriptorMatch(tRIFF.GetCkFormID(),"SFBK")) /* all caps is a no no accept it anyway   */
        {  /* really was a format error */
            return bailOut(hf);
        }
    }
    storeInfoCks(); /* stores all present infoSub chunks in the array */
    /* infoCkValues...                                */
    auto fversion = reinterpret_cast<const sfVersion*>(infoCkValues[RIFFClass::Ifil].data());
    if(fversion == nullptr) {
        // can't set the version of our hydraClass object. ifil is a _manditory_
        // info sub-chunk. We looked for "ifil" and "IFIL" (the second is illegal,
        // but does exist in otherwise valid banks, so we accept it as well....
#ifdef DEBUG
        qDebug()<<"%ReadSFBFile()-E-Bad Format, ifil (file version)"
               <<" info subchunk missing";
#endif
        return bailOut(hf);
    }
    hf->SetVersion(fversion->wMajor, fversion->wMinor);
    // Only reads SoundFont 2.x banks
    if (fversion->wMajor != 2) return bailOut(hf);
    // Inam is a REQUIRED Info sub-chunk!
    hf->achBankName=infoCkValues[RIFFClass::Inam];
    /*
   *  Read SoundFont data into the 'nine heads'
   */
    /* Move to the Articulation Data chunk... */
    if (!findToken(RIFFClass::Pdta,"ReadSFBFile-E-Error finding PDTA chunk")) return bailOut(hf);
    
    /* Allocate system memory for the Preset Table  */
    if (!findToken(RIFFClass::Phdr,"ReadSFBFile-E-Error finding PHDR chunk")) return bailOut(hf);
    long64 sz=tRIFF.GetCkSize();
    hf->pPHdr.resize(sz/sizeof(sfPresetHdr));
    tRIFF.RIFFRead(hf->pPHdr.data(),sz);
    
    /*  Move to pbag */
    if (!findToken(RIFFClass::Pbag,"ReadSFBFile-E-Error finding PBAG chunk")) return bailOut(hf);
    sz=tRIFF.GetCkSize();
    hf->pPBag.resize(sz/sizeof(sfBagNdx));
    tRIFF.RIFFRead(hf->pPBag.data(),sz);
    
    /* Move to pmod  */
    if (!findToken(RIFFClass::Pmod,"ReadSFBFile-E-Error finding PMOD chunk")) return bailOut(hf);
    sz=tRIFF.GetCkSize();
    hf->pPMod.resize(sz/sizeof(sfModList));
    tRIFF.RIFFRead(hf->pPMod.data(),sz);
    
    /*  Move to pgen  */
    if (!findToken(RIFFClass::Pgen,"ReadSFBFile-E-Error finding PGEN chunk")) return bailOut(hf);
    sz=tRIFF.GetCkSize();
    hf->pPGen.resize(sz/sizeof(sfGenList));
    tRIFF.RIFFRead(hf->pPGen.data(),sz);
    
    /* Move to inst */
    if (!findToken(RIFFClass::Inst,"ReadSFBFile-E-Error finding INST chunk")) return bailOut(hf);
    sz=tRIFF.GetCkSize();
    hf->pInst.resize(sz/sizeof(sfInst));
    tRIFF.RIFFRead(hf->pInst.data(),sz);
    
    /* Move to ibag  */
    if (!findToken(RIFFClass::Ibag,"ReadSFBFile-E-Error finding IBAG chunk")) return bailOut(hf);
    sz=tRIFF.GetCkSize();
    hf->pIBag.resize(sz/sizeof(sfBagNdx));
    tRIFF.RIFFRead(hf->pIBag.data(),sz);
    
    /*  Move to imod  */
    if (!findToken(RIFFClass::Imod,"ReadSFBFile-E-Error finding IMOD chunk")) return bailOut(hf);
    sz=tRIFF.GetCkSize();
    hf->pIMod.resize(sz/sizeof(sfModList));
    tRIFF.RIFFRead(hf->pIMod.data(),sz);
    
    /*  Move to igen  */
    if (!findToken(RIFFClass::Igen,"ReadSFBFile-E-Error finding IGEN chunk")) return bailOut(hf);
    sz=tRIFF.GetCkSize();
    hf->pIGen.resize(sz/sizeof(sfGenList));
    tRIFF.RIFFRead(hf->pIGen.data(),sz);
    
    /*  Move to shdr */
    if (!findToken(RIFFClass::Shdr,"ReadSFBFile-E-Error finding SHDR chunk")) return bailOut(hf);
    sz=tRIFF.GetCkSize();
    hf->pSHdr.resize(sz/sizeof(sfSampleHdr));
    tRIFF.RIFFRead(hf->pSHdr.data(),sz);
    /*
   *  Store sample size and/or offset for future use
   */
    if (!findToken(RIFFClass::Smpl,"ReadSFBFile-E-Error finding SMPL chunk"))
    {
        dwSampleSize = 0;
        dwSampleOffset = 0;
    }
    else
    {
        dwSampleSize = tRIFF.GetCkSize();
        dwSampleOffset = uint(tRIFF.RIFFTellAbs()); // defaults to RIFFTell for all but
    }
    
    closeRiff();
    /*
   * Do Processing of SoundFont data based on system and audio hardware
   */
#ifdef SF_BYTE_ORDER_BIG_ENDIAN
    
    /* Swap the order of the bytes in the data structs */
    
    for (uiCount = 0; uiCount < hf->pPHdr.size(); uiCount++) {
        
        tRIFF.SwapBytes(hf->pPHdr[uiCount].wPresetNum);
        tRIFF.SwapBytes(hf->pPHdr[uiCount].wPresetBank);
        tRIFF.SwapBytes(hf->pPHdr[uiCount].wBagNdx);
        
        /* We now need to swap the DWORDS  */
        
    }
    for (uiCount = 0; uiCount < hf->pPBag.size(); uiCount++) {
        
        tRIFF.SwapBytes(hf->pPBag[uiCount].wGenNdx);
        tRIFF.SwapBytes(hf->pPBag[uiCount].wModNdx);
    }
    for (uiCount = 0; uiCount < hf->pPGen.size(); uiCount++){
        tRIFF.SwapBytes(hf->pPGen[uiCount].sfGenOper);
        tRIFF.SwapBytes((WORD*)(&hf->pPGen[uiCount].unAmt));
    }
    for (uiCount = 0; uiCount < hf->pPMod.size(); uiCount++) {
        tRIFF.SwapBytes(hf->pPMod[uiCount].wModSrcOper);
        tRIFF.SwapBytes(hf->pPMod[uiCount].wModDestOper);
        tRIFF.SwapBytes((WORD*)(&hf->pPMod[uiCount].shAmount));
    }
    /* The instrument layers */
    for (uiCount = 0; uiCount < hf->pInst.size(); uiCount++) {
        tRIFF.SwapBytes(hf->pInst[uiCount].wBagNdx);
    }
    for (uiCount = 0; uiCount < hf->pIBag.size(); uiCount++) {
        tRIFF.SwapBytes(hf->pIBag[uiCount].wGenNdx);
        tRIFF.SwapBytes(hf->pIBag[uiCount].wModNdx);
    }
    for (uiCount = 0; uiCount < hf->pIGen.size(); uiCount++) {
        tRIFF.SwapBytes(hf->pIGen[uiCount].sfGenOper);
        tRIFF.SwapBytes((WORD*)(&hf->pIGen[uiCount].unAmt));
    }
    for (uiCount = 0; uiCount < hf->pIMod.size(); uiCount++) {
        tRIFF.SwapBytes(hf->pIMod[uiCount].wModSrcOper);
        tRIFF.SwapBytes(hf->pIMod[uiCount].wModDestOper);
        tRIFF.SwapBytes((WORD*)(&hf->pIMod[uiCount].shAmount));
    }
    //// Sample Headers ////
    for (uiCount = 0; uiCount < hf->pSHdr.size(); uiCount++) {
        
        
        tRIFF.SwapDWORD(hf->pSHdr[uiCount].dwStart);
        tRIFF.SwapDWORD(hf->pSHdr[uiCount].dwEnd);
        tRIFF.SwapDWORD(hf->pSHdr[uiCount].dwStartloop);
        tRIFF.SwapDWORD(hf->pSHdr[uiCount].dwEndloop);
        
        tRIFF.SwapDWORD(hf->pSHdr[uiCount].dwSampleRate);
        
        //twoByteUnion temp;
        //temp.byVals.by1 = hf->pSHdr[uiCount].byOriginalKey;
        //temp.byVals.by0 = hf->pSHdr[uiCount].chFineCorrection;
        //tRIFF.SwapBytes(&temp.wVal);
        //hf->pSHdr[uiCount].byOriginalKey    = temp.byVals.by1;
        //hf->pSHdr[uiCount].chFineCorrection = temp.byVals.by0;
        
        tRIFF.SwapBytes(hf->pSHdr[uiCount].wSampleLink);
        tRIFF.SwapBytes(hf->pSHdr[uiCount].sfSampleType);
        
        if ((hf->pSHdr[uiCount].sfSampleType & romSample) == romSample)
            bROMSamples = TRUE;
        
        //hf->pSHdr[uiCount].bSampleLoaded = FALSE;
    }
#endif
    for (sfGenList& g : hf->pPGen) {//for (ushort uiCount = 0; uiCount < hf->pPGen.size(); uiCount++){
        // The SoundFont 2.0 specification dictates that unknown generators
        // be ignored. We are doing that by changing its operator value to the
        // 'nop' operator for the purposes of the enabler. These lines may not
        // be desirable for an edit engine, which would want to leave data in
        // tact, and would instead want to be in a seperate verification routine.
        if (g.sfGenOper > endOper)
            g.sfGenOper = nop;
    }
    for (sfGenList& g : hf->pIGen) {//for (ushort uiCount = 0; uiCount < hf->pIGen.size(); uiCount++) {
        // The SoundFont 2.0 specification dictates that unknown generators
        // be ignored. We are doing that by changing its operator value to the
        // 'nop' operator for the purposes of the enabler. These lines may not
        // be desirable for an edit engine, which would want to leave data in
        // tact, and would instead want to be in a seperate verification routine.
        if (g.sfGenOper > endOper)
            g.sfGenOper = nop;
    }
    
    /* It is here that we can do a valid WaveTable ID Check */
    const char* pchIROMChunk = GetInfoSubCkVal(RIFFClass::Irom);
    /* ROM samples in a bank with no IROM chunk. Illegal bank. */
    if ((bROMSamples) && (pchIROMChunk == nullptr)) return bailOut(hf);
    
    if ((pchReqdWaveTable != nullptr)  && (pchIROMChunk != nullptr))
    {
        
        /* If the CLIENT did NOT demand that loaded banks using ROM data
       match a particular WaveTable ID, bypass.
       If the ROM ID in the SoundFont bank is a null string, there is no
       required WaveTable for the SF, so bypass.
       If neither is a null string, AND if the SoundFont bank DOES INDEED
       contain samples for a particular a WaveTable ROM, do a string
       comparison between the CLIENT's ID and the SoundFont bank's ID.
    */
        if ((pchReqdWaveTable[0] != '\0')                   &&
                (pchIROMChunk[0]     != '\0')                   &&
                (bROMSamples)                           &&
                (strcmp(pchReqdWaveTable, pchIROMChunk) != 0))
        {
            
            /* Non-matching wavetable ID when matching ID demanded, AND the
         Loaded SoundFont bank does contain samples expected to be
         in WaveTable ROM. This is an invalid bank for the current synth.
      */
            return bailOut(hf);
        }
    }
    /* Return the hydraClass ptr. */
    return hf;
} // end ReadSFBFile

void sfReader::storeInfoCks()
/*
===============================================================================
* 
*  Implementation Notes: 
*   Called from ReadSFBData(), this routine is responsible for allocating and 
* storing the info sub-chunks values that are preset in the currently loaded
* SoundFont Bank file, (if any). Many of these sub-chunks are optional, these
* values will remain nullptr. These values are stored in the static file global
* object "infoCkValues" which is an array with extent infoSubCkCount of 
* CHAR pointers. 
*
* We just iterate throught the list of defined 'tokens' asking to 'findCk'
* from RIFF, and if found, we allocate space for it and place it in the 
* list, if the value is one of the exceptional non-string values, we 
* cast it into a string anyway. The caller of getInfoCkVal() will utilize
* the size out value to correctly cast the sucker back into whatever struct
* type it really wants to be.
*
===============================================================================
*/
{
    /*  range is Ifil..Icmt */
    resetInfoCkValues();   /* Make sure we are playing with a clean slate */
    for (ushort curToken = RIFFClass::Ifil; curToken <= RIFFClass::Icmt; curToken++) {
        
        if (findToken(curToken)) {
            infoCkValues[curToken].resize(tRIFF.GetCkSize());
            auto d=const_cast<char*>(infoCkValues[curToken].data());
            tRIFF.RIFFRead(d, tRIFF.GetCkSize());
            if ( (curToken == RIFFClass::Ifil) || ( curToken == RIFFClass::Iver )) { /*its a struct...*/
                auto ver = reinterpret_cast<sfVersion*>(d);
                ver->wMajor = qFromLittleEndian<ushort>(ver->wMajor);
                ver->wMinor = qFromLittleEndian<ushort>(ver->wMinor);
                //tRIFF.SwapBytes(ver->wMajor);
                //tRIFF.SwapBytes(ver->wMinor);
#ifdef DEBUG
                qDebug() <<"StoreInfoCks-I-saw token "<< QByteArray(tRIFF.GetRIFFToken(curToken),4) <<" and stored the value  " << ver->wMajor<< "."<< ver->wMinor;
            }
            else { /* its just a string... */
                qDebug()<<"StoreInfoCks-I-saw token "<< QByteArray(tRIFF.GetRIFFToken(curToken),4) <<" and stored the value "<< infoCkValues[curToken].c_str();
#endif
            }
        }//
        else { // we didn't find the current chunk, but wait! is it the ifil?
            if (curToken == RIFFClass::Ifil) { // check if the uppercase equiv it present...
                if (tRIFF.FindCk("IFIL")) {
                    infoCkValues[curToken].resize(tRIFF.GetCkSize());
                    auto d=const_cast<char*>(infoCkValues[curToken].data());
                    tRIFF.RIFFRead(d, tRIFF.GetCkSize());
                    auto ver = reinterpret_cast<sfVersion*>(d);
                    ver->wMajor = qFromLittleEndian<ushort>(ver->wMajor);
                    ver->wMinor = qFromLittleEndian<ushort>(ver->wMinor);
                    //tRIFF.SwapBytes(ver->wMajor);
                    //tRIFF.SwapBytes(ver->wMinor);
                }
            }
        }//end else
        
    }// end for all chunks
    /*  ok we make sure there is some kind of name for the bank, if there was
   *  was no INAM chunk found, we'll put in a default of SoundFont Bank
   */
    if (infoCkValues[RIFFClass::Inam].empty()) infoCkValues[RIFFClass::Inam]="SoundFont Bank";
}/*  end storeInfoCks; */ 



const char* sfReader::GetInfoSubCkVal(RIFFClass::RiffCkTokenType token)
{
    if(( token < RIFFClass::Ifil)||(token > RIFFClass::Icmt)) { return nullptr; }
    return infoCkValues[token].c_str();
} /*  end GetSubkVal */ 


void sfReader::resetInfoCkValues()
{
    for (std::string& s : infoCkValues) s.clear();
}/* end resetInfoCkValues; */ 

/*****************************************************************************
* 
* sfReader::SetupToFillSampleBuckets
*
* Implementation Notes:  
*
*  The reader is being called upon for the sample data. The data storage
* medium must be reopened (the reader should have the keys required to do 
* so) and the variables for the sample dump routine must be set up properly.
*
******************************************************************************
*/
void sfReader::SetupToFillSampleBuckets()
{
    if (bySFStorageStatus == SFR_CLOSED)
    {
        if (!tRIFF.OpenRIFF(pathName.c_str())) return;
    }
    bySFDataLoc       = SFR_ONDISK;
    bySFStorageStatus = SFR_OPEN;

    if (!findToken(RIFFClass::Smpl)) {
        closeRiff();
        return;
    }

    dwSampleSize = tRIFF.GetCkSize();
    dwSampleOffset = uint(tRIFF.RIFFTellAbs()); // defaults to RIFFTell for all but
    // SoundROM data.
    dwSampleBytesCollected = 0L;
}

/*****************************************************************************
*
* sfReader::FillSampleBucket
*
* Implementation Notes:
*
*  The reader is being called upon for the sample data. This fills the
* buffer with the amount of data specified by the *pdwSize. The actual
* amount of data (in BYTES) filled is stuffed into *pdwSize in this routine.
* If all data has been collected, *pdwSize is set to 0.
*
*  If SetupToFillSampleBuckets() was never called, the variables used would
* indicate that all data has already been collected. This allows
* this routine to exit smoothly if called inappropriately.
*
*  Note there are 3 sizes to consider on any read:
* 1. dwThisBucketSize: The size in bytes of the bucket requested by the client.
* 2. dwSampleBytesCollected: The size in bytes of the total sample data space
*                            of this reader's SoundFont
* 3. MAX_READ_SIZE: The size in bytes of the largest possible data read.
*
* So the algorythm tries to fill up the bucket (specified by dwThisBucketSize)
* in blocks of MAX_READ_SIZE until either the bucket is full or until
* the total number of bytes collected equals the total sample data space of
* the SoundFont. When the latter condition is met, the SoundFont data storage
* medium is closed.
*
******************************************************************************
*/
void sfReader::FillSampleBucket(byte *pbyBucket, uint * pdwSize)
{
    uint dwThisBucketSize = *pdwSize;
    if (dwSampleBytesCollected >= dwSampleSize)
    {
        *pdwSize = 0;
        return;
    }
    if ((dwSampleSize - dwSampleBytesCollected) < dwThisBucketSize) dwThisBucketSize = dwSampleSize - dwSampleBytesCollected;
    *pdwSize = 0;
    dwSampleBytesCollected+=tRIFF.RIFFRead(pbyBucket,dwThisBucketSize);
    *pdwSize+=dwThisBucketSize;
#ifdef SF_BYTE_ORDER_BIG_ENDIAN
    //*******************************************
    // Yes, samples need to be byte-swapped too!
    //*******************************************
    for (ulong64 uiCount = 0; uiCount < dwThisBucketSize; uiCount +=2)
    {
        tRIFF.SwapBytes((WORD *)&pbyBucket[uiCount]);
    }
#endif
    if (dwSampleBytesCollected >= dwSampleSize)
    {
        resetSampleCollector();
        closeRiff();
    }
}

/*****************************************************************************
* 
* sfReader::IsValid
*
* Implementation Notes:  
*
*  A simple boolean function to query whether or not the reader has actually
* read something yet.
*
******************************************************************************
*/
bool sfReader::IsValid()
{
    return (bySFDataLoc != SFR_NO_DATA);
}

/*****************************************************************************
* 
* sfReader::resetSampleCollector
*
* Implementation Notes:  
*
*  Resets the sample data collection variables
*
******************************************************************************
*/
void sfReader::resetSampleCollector()
{
    dwSampleSize = 0;
    dwSampleBytesCollected = 0;
    dwSampleOffset = 0;
}

