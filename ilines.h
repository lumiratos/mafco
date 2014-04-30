// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Copyright 2013/2014 IEETA/DETI - University of Aveiro, Portugal.		 -
//	All Rights Reserved.													 -
//																			 -
//	These programs are supplied free of charge for research purposes only,   -
//	and may not be sold or incorporated into any commercial product. There   -
//	is ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they     -
//	are fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you	 - 
//	do happen to find a bug, or have modifications to suggest, please report -
//	the same to Luis M. O. Matos, luismatos@ua.pt. The copyright notice      -
//	above and this statement of conditions must remain an integral part of   -
//	each and every copy made of these files.								 -
//																			 -
//	Description: functions for handling with the 'i' lines 					 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef I_LINES_H_INCLUDED
#define I_LINES_H_INCLUDED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Lines starting with 'i' -- information about what's happening before and 
// after this block in the aligning species
//
// s hg16.chr7    27707221 13 + 158545518 gcagctgaaaaca
// s panTro1.chr6 28869787 13 + 161576975 gcagctgaaaaca
// i panTro1.chr6 N 0 C 0
// s baboon         249182 13 +   4622798 gcagctgaaaaca
// i baboon       I 234 n 19
//
// The 'i' lines contain information about the context of the sequence lines 
// immediately preceeding them. The following fields are defined by position 
// rather than name=value pairs:
//
// src -- The name of the source sequence for the alignment. Should be the 
// same as the 's' line immediately above this line.
//
// leftStatus -- A character that specifies the relationship between the 
// sequence in this block and the sequence that appears in the previous block.
//
// leftCount -- Usually the number of bases in the aligning species between 
// the start of this alignment and the end of the previous one.
//
// rightStatus -- A character that specifies the relationship between the 
// sequence in this block and the sequence that appears in the subsequent 
// block.
//
// rightCount -- Usually the number of bases in the aligning species between 
// the end of this alignment and the start of the next one.
//
// The status characters can be one of the following values:
// C -- the sequence before or after is contiguous with this block.
//
// I -- there are bases between the bases in this block and the one before or 
// after it.
//
// N -- this is the first sequence from this src chrom or scaffold.
//
// n -- this is the first sequence from this src chrom or scaffold but it is 
// bridged by another alignment from a different chrom or scaffold.
//
// M -- there is missing data before or after this block (Ns in the sequence).
//
// T -- the sequence in this block has been used before in a previous block 
// (likely a tandem duplication)
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Definition of the structure that will hold the 'i' lines
typedef struct
{
	uint8_t		nRows;			// Number of current rows 
	
	uint8_t		maxNRows;		// Number of rows of memory allocated for 'i' lines

	uint8_t		*leftStatus;	// A character that specifies the relationship between the sequence
								// in this block and the sequence that appears in the previous block.
								// C, I, N, n, M and, T	=> 6 symbols
								// 0, 1, 2, 3, 4 and, 5 => Code
	
	uint8_t		*rightStatus;	// A character that specifies the relationship between the sequence 
								// in this block and the sequence that appears in the subsequent block.
								// C, I, N, n, M and, T	=> 6 symbols
								// 0, 1, 2, 3, 4 and, 5 => Code
					
								// C -- the sequence before or after is contiguous with this block.
								// I -- there are bases between the bases in this block and the one 
								//	before or after it.
								// N -- this is the first sequence from this src chrom or scaffold.
								// n -- this is the first sequence from this src chrom or scaffold 
								//	but it is bridged by another alignment from a different chrom or scaffold.
								// M -- there is missing data before or after this block (Ns in the sequence).
								// T -- the sequence in this block has been used before in a previous 
								//	block (likely a tandem duplication)

	
	uint32_t	*leftCount;		// Usually the number of bases in the aligning species between 
								// the start of this alignment and the end of the previous one. 

	uint32_t	*rightCount;	// Usually the number of bases in the aligning species between 
								// the end of this alignment and the start of the next one.
	
	uint8_t		*statusBuffer;	// This buffer will contain the last <bufSize> status symbols read
	
	uint8_t		bufSize;		// Size of statusBuffer	
} ILinesData;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ILinesData 	*CreateILinesData	(uint8_t);
void 		FreeILinesData		(ILinesData *);
void		ResetILinesData		(ILinesData *);
void 		ResetILinesBuffer	(ILinesData *);
void 		StoreInfoRow		(ILinesData *, UChar, UChar, uint32_t, 
								uint32_t);
void 		StoreInfoRowV2		(ILinesData *, uint8_t, uint8_t, uint32_t, 
								uint32_t);


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif