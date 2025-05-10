     /*********************************************************************
     
     emuerrs.h
     
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
*  @(#)emuerrs.h	1.1 12:06:28 3/15/95 12:06:35
*
* Filename: emuerrs.h
*
* Description: Header file for Enabler Error Codes                 
*
*******************************************************************************
*/

#ifndef __EMUERRS_H
#define __EMUERRS_H

/************
** Defines
************/

#define sfSUCCESS  0

/***********************
** Enumeration Tables
***********************/

/* last value 0 0x0 */

enum RIFFERRORTAG
{
  RIFF_IDERROR = sfSUCCESS + 1,
  RIFF_ERROR,
  RIFF_FINDERROR,
  RIFF_OPENFILEERROR,
  RIFF_READFILEERROR,
  RIFF_WRITEFILEERROR
};

/* last value 6 0x6 */

enum SFERRORTAG
{
  SF_ERRORNUM = RIFF_WRITEFILEERROR + 1,
  SF_BUFFERERROR,
  SF_MEMORYERROR,
  SF_INVALIDBANK,
  SF_BANKLINKERROR,
  SF_BANKLOADERROR,
  SF_WRONGWAVETABLE,
  SF_PRESETNOTFOUND
};

extern char* emuErrors[];

#endif /* __EMUERRS_H */

