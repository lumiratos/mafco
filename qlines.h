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
//	Description: functions for handling the 'q' lines 					     -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef Q_LINES_H_INCLUDED
#define Q_LINES_H_INCLUDED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Lines starting with 'q' -- information about the quality of each aligned 
// base for the species
//
// s hg18.chr1                  32741 26 + 247249719 TTTTTGAAAAACAAACAACAAGTTGG
// s panTro2.chrUn            9697231 26 +  58616431 TTTTTGAAAAACAAACAACAAGTTGG
// q panTro2.chrUn                                   99999999999999999999999999
// s dasNov1.scaffold_179265     1474  7 +      4584 TT----------AAGCA---------
// q dasNov1.scaffold_179265                         99----------32239--------- 
//
// The 'q' lines contain a compressed version of the actual raw quality data, 
// representing the quality of each aligned base for the species with a single 
// character of 0-9 or F. The following fields are defined by position rather
// than name=value pairs:
//
// src -- The name of the source sequence for the alignment. Should be the same 
// as the 's' line immediately preceding this line.
//
// value -- A MAF quality value corresponding to the aligning nucleotide acid 
// in the preceding 's' line. Insertions (dashes) in the preceding 's' line are 
// represented by dashes in the 'q' line as well. The quality value can be 'F' 
// (finished sequence) or a number derived from the actual quality scores 
// (which range from 0-97) or the manually assigned score of 98. These numeric 
// values are calculated as:
//
//	MAF quality value = min( floor(actual quality value/5), 9 )
//
// This results in the following mapping:
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// MAF quality value	|	Raw quality score range		|	Quality level	 |
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//		   0-8			|		  0-44					|	    Low
//	 		9			|		 45-97					|	    High
//	 		0			|		  98					|   Manually assigned
//			F			|		  99					|	  Finished
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Definition of the structure that will hold the 'q' lines
typedef struct
	{
	uint32_t	nRows;			// Number of rows
	uint32_t	nCols;			// Number of cols
	uint8_t		**data;			// This structure will hold the quality values of the current MSAB	
	uint8_t		*buffer;		// This buffer will contain the last MSA_QVALUE_BUF_SIZE quality values
	uint8_t		bufSize;		// Size of the previous declared buffer
	uint32_t	rowBlockSize;	// Slot size used in Realloc when reading the 'q' lines
	}
QLinesData;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

QLinesData 	*CreateQLinesData		(uint32_t, uint8_t);
QLinesData 	*CreateQLinesDataWith	(uint32_t, uint32_t, uint8_t);
void 		ResizeQLinesData		(QLinesData *, uint32_t, uint32_t);
void 		FreeQLinesData			(QLinesData *);
void 		FreeQLinesDataWith		(QLinesData *);
void		ResetQLinesData			(QLinesData *);
void 		ResetQLinesDataWith		(QLinesData *);
void 		ResetQLinesBuffer		(QLinesData *);

void 		StoreQualityValue		(uint8_t **, uint32_t, uint32_t, UChar);
void 		StoreQualityRow			(QLinesData *, uint8_t *, uint32_t);

uint8_t 	GetQualityValue			(QLinesData *, uint32_t, uint32_t);
uint8_t 	QualityValue2Symbol		(UChar);
UChar 		Symbol2QualityValue		(uint8_t);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif