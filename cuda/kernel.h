/*==============================================================================
 *$RCSfile: kernel.h,v $
 *
 *   DESC : This file contains the Test Main and other TH support functions
 *
 * AUTHOR : C Tyron
 *
 *  EEMBC : Office Automation Subcommittee 
 *
 *    CVS : $Revision: 1.2 $
 *          $Date: 2002/04/22 22:55:02 $
 *          $Author: rick $
 *          $Source: d:/cvs/eembc2/office/text01/kernel.h,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: kernel.h,v $
 * Revision 1.2  2002/04/22 22:55:02  rick
 * Standard Comment blocks
 *
 *
 *------------------------------------------------------------------------------
 * Copyright (c) 1998-2002 by the EDN Embedded Microprocessor 
 * Benchmark Consortium (EEMBC), Inc. 
 * 
 * All Rights Reserved. This is licensed program product and 
 * is owned by EEMBC. The Licensee understands and agrees that the 
 * Benchmarks licensed by EEMBC hereunder (including methods or concepts 
 * utilized therein) contain certain information that is confidential 
 * and proprietary which the Licensee expressly agrees to retain in the 
 * strictest confidence and to use only in conjunction with the Benchmarks 
 * pursuant to the terms of this Agreement. The Licensee further agrees 
 * to keep the source code and all related documentation confidential and 
 * not to disclose such source code and/or related documentation to any 
 * third party. The Licensee and any READER of this code is subject to 
 * either the EEMBC Member License Agreement and/or the EEMBC Licensee 
 * Agreement. 
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, EEMBC DISCLAIMS ALL 
 * WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, 
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
 * PURPOSE, WITH REGARD TO THE BENCHMARKS AND THE ACCOMPANYING 
 * DOCUMENTATION. LICENSEE ACKNOWLEDGES AND AGREES THAT THERE ARE NO 
 * WARRANTIES, GUARANTIES, CONDITIONS, COVENANTS, OR REPRESENTATIONS BY 
 * EEMBC AS TO MARKETABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR OTHER 
 * ATTRIBUTES, WHETHER EXPRESS OR IMPLIED (IN LAW OR IN FACT), ORAL OR 
 * WRITTEN. 
 * 
 * Licensee hereby agrees by accessing this source code that all benchmark 
 * scores related to this code must be certified by ECL prior to publication 
 * in any media, form, distribution, or other means of conveyance of 
 * information subject to the terms of the EEMBC Member License Agreement 
 * and/or EEMBC Licensee Agreement. 
 * 
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/

#ifndef KERNEL_H_DEFINED
#define KERNEL_H_DEFINED
/**************************************************************************

FILE DESCRIPTION
    Defines for EEMBC Office Automation benchmark algorithms.

**************************************************************************/

/*************************************************************************
        CONSTANTS
**************************************************************************/

#ifndef NULL
#define NULL            ((void *)0)
#endif
#define NULLCHAR        ((char)0)

#define CHAR_BITS       (8)


/*************************************************************************
        COMPILER SWITCHES
**************************************************************************/

/*
    Surround the BENCHMARK define with an "ifndef", just in case the
    flag is also specified on the command line with a "-D" option
*/
#ifndef BENCHMARK
#define BENCHMARK       (1)
#endif


/* Compiler Switches for Bitmap Rotation and Gray Scale Dithering */
/* un-comment "#define OUTPUTIMAGE" to enable the print out of the
 * result (rotated image)
 */
/*  #define OUTPUTIMAGE  */    


/* Compiler Switches for Bezier Curves */
/* un-comment this to enable the print out of the result */
/*  #define PRINTOUTPTS  */    


/* Compiler Switches for Text Processing */
/* un-comment this to enable the print out of the result */
/*  #define PRINTOUTTEXT  */   

#define BIT1_LOW

/*************************************************************************
        TYPE DEFINES
**************************************************************************/

#if BMDEBUG
#define DEBUG0(s)               printf(s)
#define DEBUG1(s,a)             printf(s, a)
#define DEBUG2(s,a,b)           printf(s, a, b)
#define DEBUG3(s,a,b,c)         printf(s, a, b, c)
#else
#define DEBUG0(s)
#define DEBUG1(s,a)
#define DEBUG2(s,a,b)
#define DEBUG3(s,a,b,c)
#endif

#endif /*  KERNEL_H_DEFINED */
