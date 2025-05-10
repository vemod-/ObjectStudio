    /*********************************************************************
     
     riff.h
     
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

#ifndef RIFF_H
#define RIFF_H

#include "softsynthsdefines.h"
#include "QtEndian"
#include <stdio.h>
#include <QFile>
#include <QString>

//*****************************************************************************
// @(#)riff.h	1.1 12:06:31 3/15/95 12:06:36
//                             
// Filename: RIFF.H
//
//********************************************************************

#pragma pack (push,1)

typedef struct ckHeaderTag
{
  uint dwCkID;
  uint dwCkSize;
} ckHeader;


typedef struct ckFormTag
{
  ckHeader ckHdr;
  uint    dwFormID;
} ckForm;


typedef struct ckListTag
{
  ckHeader  ckHdr;
  uint     dwListID;
} ckList;

#pragma pack (pop)

//*********************************************************
//  The methods which open and read the RIFF files
//
//  Note:  When we ReadCkHdr, the file ptr is always
//  left at the beginning of the chunk data.  When we
//  DescendRIFF, the file ptr is left at the beginning
//  of the data as well.  When we use AscendRIFF,
//  the same file ptr positioning applies.  AscendRIFF
//  will take us to the next beginning of the next chunk
//  since we are emerging from the present chunk, but
//  DescendRIFF will not keep descending unless
//  subsequent chunks have a LIST ID, since simple
//  non-RIFF and non-LIST chunks may not contain
//  subchunks.  Id est, there is  nothing more to
//  descend into.
//*********************************************************

class RIFFClass
{
  public:
    RIFFClass()
    {
        //zeroMemory(RiffTokens,RiffCkCount);
        //*********************************************************
        // The RIFF Tokens Array is the collection of RIFF ID tokens that this
        // module recognizes. Here the array is initialized.
        //*********************************************************
        // Riff Header Chunks Tokens...
        RiffTokens[Riff] = RIFF;
        RiffTokens[Sfbk] = SFBK;
        RiffTokens[List] = LIST;
        RiffTokens[Info] = INFO;
        // Info Chunks Tokens...
        RiffTokens[Ifil] = IFIL;
        RiffTokens[Isng] = ISNG;
        RiffTokens[Irom] = IROM;
        RiffTokens[Iver] = IVER;
        RiffTokens[Inam] = INAM;
        RiffTokens[Iprd] = IPRD;
        RiffTokens[Icop] = ICOP;
        RiffTokens[Icrd] = ICRD;
        RiffTokens[Ieng] = IENG;
        RiffTokens[Icmt] = ICMT;
        // Sample Chunk Tokens...
        RiffTokens[Sdta] = SDTA;
        RiffTokens[Snam] = SNAM;
        RiffTokens[Smpl] = SMPL;
        // Preset Chunk Tokens...
        RiffTokens[Pdta] = PDTA;
        RiffTokens[Phdr] = PHDR;
        RiffTokens[Pbag] = PBAG;
        RiffTokens[Pmod] = PMOD;
        RiffTokens[Pgen] = PGEN;
        RiffTokens[Inst] = INST;
        RiffTokens[Ibag] = IBAG;
        RiffTokens[Imod] = IMOD;
        RiffTokens[Igen] = IGEN;
        RiffTokens[Shdr] = SHDR;
    }
    ~RIFFClass() { CloseRIFF(); } /// Close file here ////
    bool  OpenRIFF(const char* pName)
    {
        if (!RIFFOpen(pName)) return false;
        return InitRIFF();
    }
    void  CloseRIFF() { RIFFClose(); }
    bool  FindCk(const char* pID) // Find a subchunk with the pID identity
    {
        Reset();
        ReadCkHdr();
        while (true)
        {
            if (descriptorMatch(&dwLastID,pID) || descriptorMatch(&dwLastFormID,pID)) return true; // We found our chunk
            if (!Descend()) if (!Ascend()) break;// Jump to the next chunk
        }
        return false;
    }
    bool  Reset()
    {
        if (pFile->seek(0)) return ReadCkHdr();
        return false;
    }
    inline long64  GetRIFFSize() { return dwRIFFSize;   }
    inline uint*  GetCkID()     { return &dwLastID;     }
    inline long64  GetCkSize()   { return dwLastSize;   }
    inline uint*  GetCkFormID() { return &dwLastFormID; }

    inline long64 RIFFRead(void* vStream, long long wSize) { return pFile->read(static_cast<char*>(vStream),wSize); }

    inline long64 RIFFTellAbs() { return pFile->pos(); }
    inline char*  GetRIFFToken(ushort wCurToken) { return (RiffTokens[wCurToken]); }

#define RiffCkCount 28
    enum RiffCkTokenType{
    // Riff header Tokens...
         Riff,
         Sfbk,
         List,
         Info,
    // Info Chunk Tokens...
         Ifil,
         Isng,
         Irom,
         Iver,
         Inam,
         Iprd,
         Icop,
         Icrd,
         Ieng,
         Icmt,
    // Sample Chunk Tokens
         Sdta,
         Snam,
         Smpl,
    // Preset Chunk Tokens
         Pdta,
         Phdr,
         Pbag,
         Pmod,
         Pgen,
         Inst,
         Ibag,
         Imod,
         Igen,
         Shdr
    };
private:
    char RIFF[4] = { 'R', 'I', 'F', 'F' };
    char SFBK[4] = { 's', 'f', 'b', 'k' };
    //char SFbk[4] = { 'S', 'F', 'b', 'k' };
    char LIST[4] = { 'L', 'I', 'S', 'T' };
    char INFO[4] = { 'I', 'N', 'F', 'O' };

    char IFIL[4] = { 'i', 'f', 'i', 'l' };
    char ISNG[4] = { 'i', 's', 'n', 'g' };
    char IROM[4] = { 'i', 'r', 'o', 'm' };
    char IVER[4] = { 'i', 'v', 'e', 'r' };
    char INAM[4] = { 'I', 'N', 'A', 'M' };
    char IPRD[4] = { 'I', 'P', 'R', 'D' };
    char ICOP[4] = { 'I', 'C', 'O', 'P' };
    char ICRD[4] = { 'I', 'C', 'R', 'D' };
    char IENG[4] = { 'I', 'E', 'N', 'G' };
    char ICMT[4] = { 'I', 'C', 'M', 'T' };  // new comment subChunk

    char SDTA[4] = { 's', 'd', 't', 'a' };
    char SNAM[4] = { 's', 'n', 'a', 'm' };
    char SMPL[4] = { 's', 'm', 'p', 'l' };

    char PDTA[4] = { 'p', 'd', 't', 'a' };
    char PHDR[4] = { 'p', 'h', 'd', 'r' };
    char PBAG[4] = { 'p', 'b', 'a', 'g' };
    char PMOD[4] = { 'p', 'm', 'o', 'd' };
    char PGEN[4] = { 'p', 'g', 'e', 'n' };
    char INST[4] = { 'i', 'n', 's', 't' };
    char IBAG[4] = { 'i', 'b', 'a', 'g' };
    char IMOD[4] = { 'i', 'm', 'o', 'd' };
    char IGEN[4] = { 'i', 'g', 'e', 'n' };
    char SHDR[4] = { 's', 'h', 'd', 'r' };
    //// Other IDs ////
    //static CHAR WAVEID[4] = { 'W', 'A', 'V', 'E' };
    //static CHAR FMTID[4]  = { 'f', 'm', 't', ' ' };
    //static CHAR DATAID[4] = { 'd', 'a', 't', 'a' };
    //// Macintosh IDs ////
    //static CHAR AIFF[4]   = { 'A', 'I', 'F', 'F' };
    bool RIFFOpen(const char* lPointer)
    {
        RIFFClose();
        pFile = new QFile(QString(lPointer));
        if (pFile)
        {
            if (pFile->open(QFile::ReadOnly)) return true;
            pFile = nullptr;
        }
        return false;
    }
    void RIFFClose()
    {
        if (pFile)
        {
            pFile->close();
            pFile=nullptr;
        }
    }
    bool InitRIFF()
    {
        if (!ReadCkHdr()) return false;
        if (!descriptorMatch(&dwLastID,RIFF)) return false;
        dwRIFFSize = dwLastSize;
        return true;
    }
    bool ReadCkHdr() // Reads the chunk header and retains their values
    {
        ckHeader ckHdr;

        if (RIFFRead(&ckHdr, sizeof(ckHdr)) == 0) return false;

        dwLastID   = ckHdr.dwCkID;
        dwLastSize = qFromLittleEndian<uint>(ckHdr.dwCkSize);
        //SwapDWORD(&dwLastSize);

        if (descriptorMatch(&dwLastID,RIFF) || descriptorMatch(&dwLastID,LIST))
            RIFFRead(&dwLastFormID, sizeof(dwLastFormID));
        else
            dwLastFormID = 0;
        lLastFilePtr = pFile->pos();

        return true;
    }
    bool Descend() // Descend into a chunk, i.e., a set of one or more subchunks contained in a parent chunk
    {
        //// We are at a RIFF or LIST ID ////
        if (descriptorMatch(&dwLastID,RIFF) || descriptorMatch(&dwLastID,LIST)) return ReadCkHdr();
        //// There are no more RIFF or LIST chunks within this chunk ////
        return false;
    }
    bool Ascend() // Ascend to the next chunk, i.e., skip to the very next chunk
    {
        if ((lLastFilePtr + dwLastSize) <= dwRIFFSize)
        {
            //// Our last chunk had a RIFF or LIST ID ////
            if (dwLastFormID) dwLastSize -= sizeof(uint);
            if (pFile->seek(lLastFilePtr + dwLastSize)) return ReadCkHdr();
        }
        return false;
    }
    QFile*   pFile = nullptr;
    long64 lLastFilePtr = 0;    // The chunk in the file where we visited last
    uint dwLastID = 0;        // The ID of the last RIFF/LIST chunk visited
    long64 dwLastSize = 0;      // The size of our last visited chunk
    uint dwLastFormID = 0;    // The last form/list-type we visited
    long64 dwRIFFSize = 0;      // The size of the file - the header ID and Size
    char *RiffTokens[RiffCkCount];
};

#endif // RIFF_H
////////////////////////// End of RIFF.H //////////////////////////
