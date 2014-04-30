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
//	Description: functions for handling with the 'e' lines 					 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef E_LINES_H_INCLUDED
#define E_LINES_H_INCLUDED

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Lines starting with 'e' -- information about empty parts of the alignment 
// block
//
// s hg16.chr7    27707221 13 + 158545518 gcagctgaaaaca
// e mm4.chr6     53310102 13 + 151104725 I
//
// The 'e' lines indicate that there isn't aligning DNA for a species but that 
// the current block is bridged by a chain that connects blocks before and after 
// this block. The following fields are defined by position rather than 
// name=value pairs.
//
// src -- The name of one of the source sequences for the alignment.
//
// start -- The start of the non-aligning region in the source sequence. This is 
// a zero-based number. If the strand field is '-' then this is the start 
// relative to the reverse-complemented source sequence (see Coordinate Transforms).
//
// size -- The size in base pairs of the non-aligning region in the source sequence.
//
// strand -- Either '+' or '-'. If '-', then the alignment is to the 
// reverse-complemented source.
//
// srcSize -- The size of the entire source sequence, not just the parts involved 
// in the alignment. alignment and any insertions (dashes) as well.
//
// status -- A character that specifies the relationship between the non-aligning 
// sequence in this block and the sequence that appears in the previous and 
// subsequent blocks.
// 
// The status character can be one of the following values:
//
// C -- the sequence before and after is contiguous implying that this region was 
// either deleted in the source or inserted in the reference sequence. The browser 
// draws a single line or a '-' in base mode in these blocks.
//
// I -- there are non-aligning bases in the source species between chained 
// alignment blocks before and after this block. The browser shows a double 
// line or '=' in base mode.
//
// M -- there are non-aligning bases in the source and more than 90% of them 
// are Ns in the source. The browser shows a pale yellow bar.
//
// n -- there are non-aligning bases in the source and the next aligning block
// starts in a new chromosome or scaffold that is bridged by a chain between 
// still other blocks. The browser shows either a single line or a double line
// based on how many bases are in the gap between the bridging alignments.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// Definition of the structure that will hold the 'e' lines
typedef struct
{
	uint8_t		nRows;			// Current number of rows stored
	
	uint8_t		maxNRows;		// Current number of rows allowed (memory allocated) 
	
	uint32_t	*start;			// The start of the non-aligning region in the source sequence. This is 
								// a zero-based number. If the strand field is '-' then this is the start 
								// relative to the reverse-complemented source sequence.

	uint32_t	*size;			// The size in base pairs of the non-aligning region in the source sequence.

	uint8_t		*strand;		// Either '+' or '-'. If '-', then the alignment is to the reverse-complemented source.

								// This can be obtained by the 's' line
	uint32_t	*srcSize;		// The size of the entire source sequence, not just the parts involved 
								// in the alignment. alignment and any insertions (dashes) as well.

	uint8_t		*status;		// A character that specifies the relationship between the non-aligning sequence 
								// in this block and the sequence that appears in the previous and subsequent blocks.

								// The status character can be one of the following values:

								// C -- the sequence before and after is contiguous implying that this region was either 
								// deleted in the source or inserted in the reference sequence. The browser draws a single 
								// line or a '-' in base mode in these blocks.

								// I -- there are non-aligning bases in the source species between chained alignment blocks 
								// before and after this block. The browser shows a double line or '=' in base mode.

								// M -- there are non-aligning bases in the source and more than 90% of them are Ns in the 
								// source. The browser shows a pale yellow bar.

								// n -- there are non-aligning bases in the source and the next aligning block starts 
								// in a new chromosome or scaffold that is bridged by a chain between still other blocks. 
								// The browser shows either a single line or a double line based on how many bases are in 
								// the gap between the bridging alignments.

								// C, I, M, and, n => 4 symbols
								// 0, 1, 2, and, 3 => Code
	
	uint8_t		*statusBuffer;	// This buffer will contain the last <bufSize> status symbols read
	
	uint8_t		bufSize;		// Size of statusBuffer	
	
	/*
	FIXME
	// Will store the correspondig species IDs of the current species.chr
	uint32_t 	*specIds;	
	
	// This array will store the previous species IDs of the previous 
	uint32_t 	*previousSpecIds;
	
	// This field will contain the number of e lines of the previous MSA block 
	uint8_t		previousNRows; 
	
	// The previous maximum number of e lines
	uint8_t		previousMaxNRows;
	*/
	
} ELinesData;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ELinesData 	*CreateELinesData		(uint8_t);
void 		FreeELinesData			(ELinesData *);
void		ResetELinesData			(ELinesData *);
void 		ResetELinesBuffer		(ELinesData *);
void	 	StoreEmtpyRegionRow		(ELinesData *, uint32_t, uint32_t, UChar, 
									uint32_t, UChar);
void 		StoreEmtpyRegionRowV2	(ELinesData *, uint32_t, uint32_t, uint8_t, 
									uint32_t, uint8_t);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif