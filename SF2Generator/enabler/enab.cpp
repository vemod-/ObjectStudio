/*********************************************************************

     enab.cpp
     
     Copyright (c) Creative Technology Ltd. 1995. All Rights Reserved.
     
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
#define DEBUG_ENAB
#ifdef DEBUG_ENAB
#include <QDebug>
#endif

#include "enab.h"
#include "sfreader.h"

CSF2Enabler::CSF2Enabler()
{
    m_Hydra=nullptr;
}

CSF2Enabler::~CSF2Enabler()
{
    UnloadSFBank();
}

bool CSF2Enabler::ReadSFBFile(const std::string& bankFileName)
{
    sfReader             reader;

    UnloadSFBank();
    HydraClass *pHF = reader.ReadSFBFile(bankFileName);
    if(!pHF)
    {
#ifdef DEBUG_ENAB
        qDebug()<<"Enab-ReadBank-E- error returned from reader - ";
#endif
        return false;
    }
    if (!pHF->VerifySFBData(reader.GetAllSampleSize()))
    {
#ifdef DEBUG_ENAB
        qDebug()<<"Enab-VerifyBank-E- error returned from error detector - ";
#endif
        return false;
    }

    m_Hydra = pHF;

    uint readSize=reader.GetAllSampleSize();
    sampleData.resize(readSize/sizeof(short));
    reader.SetupToFillSampleBuckets();
    reader.FillSampleBucket(reinterpret_cast<byte*>(sampleData.data()),&readSize);

#ifdef DEBUG_ENAB
    qDebug()<<"File Name     : " << bankFileName.c_str();
    qDebug()<<"Bank Name     : " << m_Hydra->achBankName.c_str();
    qDebug()<<"SMPL Offset   : " << reader.GetSampleOffset();
    qDebug()<<"SMPL Size     : " << reader.GetAllSampleSize();
    qDebug()<<"Hydra class   : " << m_Hydra;
#endif
    return true;
}

bool CSF2Enabler::UnloadSFBank()
{
    if (HydraLoaded()) { // we have it.
#ifdef DEBUG_ENAB
        qDebug()<<"Hydra class   : " << m_Hydra;
#endif
        delete m_Hydra;
        m_Hydra=nullptr;
        sampleData.clear();
    }
    return true;
}

const std::vector<sfSampleHdr> CSF2Enabler::GetSampHdrs()
{
    std::vector<sfSampleHdr> v;
    if (!HydraLoaded()) return v;
    v = m_Hydra->pSHdr;
    v.pop_back();
    return v;
}

const std::vector<sfPresetHdr> CSF2Enabler::GetPresetHdrs()
{
    std::vector<sfPresetHdr> v;
    if (!HydraLoaded()) return v;
    v = m_Hydra->pPHdr;
    v.pop_back();
    return v;
}

const std::vector<sfData> CSF2Enabler::Nav(ushort MidiBank, ushort MidiPreset, ushort MidiNote, ushort KeyVelo)
{
    if (!HydraLoaded()) return std::vector<sfData>();
    navit.SetHydraFont(m_Hydra);   // would be bank select
    ushort sfPrstIndex;
    if (!m_Hydra->GetSFNum(MidiBank, MidiPreset, &sfPrstIndex)) // would be patch change
    {  // could be sfBank...Thats all the errors defined for GetSFNum()
        return std::vector<sfData>();
    } // anything else must be some kind of internal error,
    return navit.Navigate(sfPrstIndex, MidiNote, KeyVelo);   // _the_ note on event
}

const std::vector<sfData> CSF2Enabler::Nav(ushort MidiProgram, ushort MidiNote, ushort KeyVelo)
{
    if (!HydraLoaded()) return std::vector<sfData>();
    navit.SetHydraFont(m_Hydra);   // would be bank select
    return navit.Navigate(MidiProgram, MidiNote, KeyVelo);   // _the_ note on event
}


