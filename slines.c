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

#include <stdio.h>
#include <stdlib.h>
#include "slines.h"
#include "mem.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SLinesData *CreateSLinesData(uint32_t rowBlockSize)
{
	SLinesData *sLinesData = (SLinesData *)Calloc(1, sizeof(SLinesData));
	
	sLinesData->rowBlockSize = rowBlockSize;
	sLinesData->nRows=0;
	sLinesData->nCols=0;			
	sLinesData->data = NULL;
		
	return sLinesData;	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
SLinesData *CreateSLinesDataWith(uint32_t nRows, uint32_t nCols)
{
	uint32_t n;
	SLinesData *sLinesData = (SLinesData *)Calloc(1, sizeof(SLinesData));
	
	// Not used in the decoder
	//sLinesData->rowBlockSize = 0;
	sLinesData->nRows=nRows;
	sLinesData->nCols=nCols;
	
	sLinesData->data = NULL;
	// Only allocate memory if meeded
	if((nRows != 0) && (nCols != 0))
	{
		sLinesData->data = (UChar **)Calloc(nRows, sizeof(UChar *));
		for(n = 0; n != sLinesData->nRows; ++n)
			sLinesData->data[n] = (UChar *)Calloc(nCols, sizeof(UChar));					
	}	
	return sLinesData;	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResizeSLinesData(SLinesData *sLinesData, uint32_t nRows, uint32_t nCols)
{
	uint32_t n;
	if(sLinesData == NULL)
	{	
		fprintf(stderr, "Error (ResizeSLinesData) unable to resize a NULL structure.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		sLinesData->nRows=nRows;
		sLinesData->nCols=nCols;
		
		sLinesData->data = (UChar **)Calloc(nRows, sizeof(UChar *));
		for(n = 0; n != sLinesData->nRows; ++n)
			sLinesData->data[n] = (UChar *)Calloc(nCols, sizeof(UChar));
	}	
}	
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
void FreeSLinesData(SLinesData *sLinesData)
{
	ResetSLinesData(sLinesData);
	Free(sLinesData, sizeof(SLinesData));
	sLinesData = NULL;		
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	
void FreeSLinesDataWith(SLinesData *sLinesData)
{
	ResetSLinesDataWith(sLinesData);
	Free(sLinesData, sizeof(SLinesData));
	sLinesData = NULL;	
}
		
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetSLinesData(SLinesData *sLinesData)
{
	uint32_t n, rowBlockSize = sLinesData->rowBlockSize;
	if( (sLinesData->nRows != 0) && (sLinesData->nCols != 0))
	{
		for(n = 0 ; n != sLinesData->nRows; ++n)
		{
			Free(sLinesData->data[n], 
				(((sLinesData->nCols-1) / rowBlockSize) + 1) *
				rowBlockSize * sizeof(UChar));	
		}
		Free(sLinesData->data, sLinesData->nRows * sizeof(UChar *));
	}	
	
	sLinesData->nRows = 0;
	sLinesData->nCols = 0;
	sLinesData->data = NULL;	
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetSLinesDataWith(SLinesData *sLinesData)
{
	uint32_t n;
	if( (sLinesData->nRows != 0) && (sLinesData->nCols != 0))
	{
		for(n = 0; n != sLinesData->nRows; ++n)
			Free(sLinesData->data[n], sLinesData->nCols*sizeof(UChar));
		Free(sLinesData->data, sLinesData->nRows * sizeof(UChar *));
	}
	
	sLinesData->nRows = 0;
	sLinesData->nCols = 0;
	sLinesData->data = NULL;	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StoreSequenceValue(UChar **sequenceRow, uint32_t sequenceRowSize, 
	uint32_t rowBlockSize, UChar c)
{
	if(sequenceRowSize % rowBlockSize == 0)
	{
		*sequenceRow = (UChar *)Realloc(*sequenceRow, sizeof(UChar) * 
			(sequenceRowSize + rowBlockSize), sizeof(UChar) * rowBlockSize);
	}
	(*sequenceRow)[sequenceRowSize++] = c;
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StoreSequenceRow(SLinesData *sLinesData, UChar *newRow, uint32_t rowSize)
{
	
	if( (sLinesData->nCols) && (rowSize != sLinesData->nCols) )
	{
		fprintf(stderr, "Error (StoreSequenceRow): trying to add a row with "
			"different size.\n");
		fprintf(stderr, "Error (StoreSequenceRow): current sequence MSAB row "
			"size (number of columns) = %"PRIu32".\n", sLinesData->nCols);
		fprintf(stderr, "Error (StoreSequenceRow): try to add a row with = "
			"%"PRIu32" elements.\n", rowSize);
		exit(EXIT_FAILURE);
	}
		
	sLinesData->data = (UChar **)Realloc(sLinesData->data, 
		(sLinesData->nRows + 1) * sizeof(UChar *), sizeof(UChar *));
	sLinesData->data[sLinesData->nRows] = newRow;
	sLinesData->nRows++;
	
	// Set the number of column in case we are storing the first row of the MSA block
	if(!sLinesData->nCols)
		sLinesData->nCols = rowSize;	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

UChar GetSequenceValueCharacter(SLinesData *sLinesData, uint32_t row, 
								uint32_t col)
{
	if(row >= 0 && row < sLinesData->nRows && col >=0 && 
		col < sLinesData->nCols)
		return sLinesData->data[row][col];
	else
		return 'A';
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

UChar GetSequenceValueCharacterV2(SLinesData *sLinesData, int64_t row, 
									int64_t col)
{
	if((row >= 0) && (row < (int64_t)sLinesData->nRows) && (col >=0) && 
		(col < (int64_t)sLinesData->nCols) )
	{
		return sLinesData->data[row][col];
	}
	else
		return 'A';
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SetSequenceValueCharacter(SLinesData *sLinesData, uint32_t row, 
								uint32_t col, UChar c)	
{
	if((row < sLinesData->nRows) && (col < sLinesData->nCols) )
	{
		sLinesData->data[row][col] = c;
	}
	else
	{
		fprintf(stderr, "Error (SetSequenceValueCharacter): unable to set " 
			"character '%c' at position (%"PRIu32", %"PRIu32")\n.", c, row, col);
		fprintf(stderr, "Error (SetSequenceValueCharacter): row range allowed "
			"in this MSAB [0 - %"PRIu32"].\n", sLinesData->nRows-1);
		fprintf(stderr, "Error (SetSequenceValueCharacter): column range allowed "
			"in this MSAB [0 - %"PRIu32"].\n", sLinesData->nCols-1);
		exit(EXIT_FAILURE);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
uint8_t SequenceValueCharacter2Symbol(UChar c)
{
	switch(c)
	{
		case 'a': case 'A': return 0;
		case 'c': case 'C': return 1;
		case 'n': case 'N': return XCHR_SYMBOL; // N's are replaced by C's
		case 'g': case 'G': return 2;
		case 't': case 'T': return 3;
		case '-': return GAP_SYMBOL;			// GAP_SYMBOL = 4
		
		// In case of an unkonw symbol, we end the program.		
		default: 
			fprintf(stderr,"Error (SequenceValueCharacter2Symbol): unexpected "
				"sequence character: '%c' (ASCII: %"PRIu8")\n", c, c);
			exit(EXIT_FAILURE);
	}
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
UChar Symbol2SequenceValueCharacter	(uint8_t s)
{
	switch(s)
	{
		case 0: return 'A';
		case 1: return 'C';
		case 2: return 'G';
		case 3: return 'T';
		case GAP_SYMBOL: return '-';
		
		// In case of decoding other symbol, we end the program.		
		default: 
			fprintf(stderr,"Error (Symbol2SequenceValueCharacter): unexpected "
				"symbol: %"PRIu8"\n", s);
			exit(EXIT_FAILURE);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -		