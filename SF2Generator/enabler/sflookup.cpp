     /*********************************************************************
     
     sflookup.cpp
     
     Copyright (c) Creative Technology Ltd. 1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
//-*********************************************************************
// 
//     Copyright (C) 1995 E-mu Systems, Inc. All Rights Reserved.
// 
//-*********************************************************************

//-*********************************************************************
// sflookup.cpp: A lookup which gives a SFENUM to SFVECTOR translation
//               by using simple indexing. 
// 
// NOTE: This is its own module in case other objects need the same
//       lookup table. In the SoundFont enabler, this is not the 
//       case.
//
// WARINING: Make sure your SFENUM lies within the scope of this 
//           lookup table, otherwise you may find yourself indexing
//           into other memory space!
//-*********************************************************************
#include "datatype.h" 
#include "sfdata.h"
#include "sflookup.h"

#ifndef NULL
#define NULL 0
#endif



