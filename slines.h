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
//	Description: functions for handling the 's' lines 					     -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef S_LINES_H_INCLUDED
#define S_LINES_H_INCLUDED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Lines starting with 's' -- a sequence within an alignment block
//
// s hg16.chr7 	  27707221 13 + 158545518 gcagctgaaaaca 
// s panTro1.chr6 28869787 13 + 161576975 gcagctgaaaaca 
// s baboon         249182 13 +   4622798 gcagctgaaaaca 
// s mm4.chr6 	  53310102 13 + 151104725 ACAGCTGAAAATA
//
// The 's' lines together with the 'a' lines define a multiple alignment. 
// The 's' lines have the following fields which are defined by position rather 
// than name=value pairs.
//
// src -- The name of one of the source sequences for the alignment. For 
// sequences that are resident in a browser assembly, the form 
// 'database.chromosome' allows automatic creation of links to other assemblies. 
// Non-browser sequences are typically reference by the species name alone.
//
// start -- The start of the aligning region in the source sequence. This is a 
// zero-based number. If the strand field is '-' then this is the start relative 
// to the reverse-complemented source sequence (see Coordinate Transforms).
//
// size -- The size of the aligning region in the source sequence. This number is 
// equal to the number of non- dash characters in the alignment text field below.
//
// strand -- Either '+' or '-'. If '-', then the alignment is to the 
// reverse-complemented source.
//
// srcSize -- The size of the entire source sequence, not just the parts involved 
// in the alignment.
//
// text -- The nucleotides (or amino acids) in the alignment and any insertions 
// (dashes) as well.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Definition of the structure that will hold the 's' lines
typedef struct
	{
	uint32_t	nRows;				// Number of rows
	uint32_t	nCols;				// Number of cols
	UChar		**data;				// DNA bases A, C, G, T, N and - (gaps)
	uint32_t	rowBlockSize;		// Slot size used in Realloc when reading the 's' lines
	}
SLinesData;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SLinesData	*CreateSLinesData				(uint32_t);
SLinesData	*CreateSLinesDataWith			(uint32_t, uint32_t);
void 		ResizeSLinesData				(SLinesData *, uint32_t, uint32_t);
void 		FreeSLinesData					(SLinesData *);
void		FreeSLinesDataWith				(SLinesData *);
void		ResetSLinesData					(SLinesData	*);
void 		ResetSLinesDataWith				(SLinesData *);

void		StoreSequenceValue				(UChar **, uint32_t, uint32_t, UChar);
void		StoreSequenceRow				(SLinesData *, UChar *, uint32_t);

UChar		GetSequenceValueCharacter		(SLinesData	*, uint32_t, uint32_t);
UChar		GetSequenceValueCharacterV2		(SLinesData	*, int64_t, int64_t);
void 		SetSequenceValueCharacter		(SLinesData *, uint32_t, uint32_t, UChar);	
uint8_t		SequenceValueCharacter2Symbol	(UChar);
UChar		Symbol2SequenceValueCharacter	(uint8_t);


//void		StoreSequenceRow				(SLinesData *, UChar *, uint64_t);
//void 		StoreSequenceRow(SLinesData *sLinesData, UChar *newRow, uint64_t rowSize, uint32_t rowBlockSize);
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif