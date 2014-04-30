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
//	Description: functions for handling the MSAB		 					 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef MSAB_H_INCLUDED
#define MSAB_H_INCLUDED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "defs.h"
#include "slines.h"
#include "qlines.h"
#include "ilines.h"
#include "elines.h"
#include "common.h"
#include "models.h"
#include "hash.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Definition of a Multiple Sequence Alignment Block (MSAB)
typedef struct
{
		char 		*sourceName;		// Source name string
		
		uint32_t	maxSrcNameSize;		// Max size of the source name
		
		double 		score;				// Alignment score
		
		uint32_t	lowerCaseCount;		// Number of lower case bases in the alignment block
		
		uint32_t	xchrCount;			// Number of extra bases (N's or n's) in the alignment block
		
		uint32_t	rowBlockSize;		// Slot size used in Realloc in the reading of the 's' and 'q' lines
		
		SLinesData 	*sLinesData;		// Sequences within the alignment block
		
		QLinesData 	*qLinesData;		// Information about the quality of each aligned 
										// base for the specie
		
		ILinesData 	*iLinesData;		// Information about what is happening before and
										// after this block in the aligning species
		
		ELinesData 	*eLinesData;		// Information about empty parts of the alignment block
		
		
		LineInfo 	*linesInfo;			// Will save the lines information to be encoded latter
										// Is an array of pointer to each element
		
		LineInfo	*eLinesInfo;		// Will save e lines information to be encoded latter
										// The sources here are different from the previous 
										// 's', 'q' and 'i' lines
		
		LineInfo	*prevELinesInfo;	// This will contain the information of the previous block 
										// 'e' lines
		
		char		*line;
		uint32_t	maxLineSize;
		
		HashTable	*hashTable;			// Hash table to store multiple information such as source name, 
										// start, chromosome size, etc.
		
		HashPosition *hashPosition;		// Current hash position (hash key and element ID)
} MSAB;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MSAB	*CreateMSAB			(uint32_t, ModelsOrder *, HashTableInfo *, uint32_t);
MSAB	*CreateEmptyMSAB	(ModelsOrder *, HashTableInfo *);
void	FreeMSAB			(MSAB *);
void	FreeMSABWith		(MSAB *);
void 	ResetMSAB			(MSAB *);
void 	ResetMSABWith		(MSAB *);
void	ResetBuffers		(MSAB *);
void 	ResetPrevELinesInfo	(MSAB *);
void	ResetELinesInfo		(MSAB *);

void	GetNPlaces			(MSAB *, uint8_t *);
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif