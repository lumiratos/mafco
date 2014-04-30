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

#include <stdio.h>
#include <stdlib.h>
#include "elines.h"
#include "mem.h"
#include "common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ELinesData *CreateELinesData(uint8_t bufSize)
{
	ELinesData *eLinesData = (ELinesData *)Calloc(1, sizeof(ELinesData));
	
	eLinesData->statusBuffer = (uint8_t *)Calloc(bufSize, sizeof(uint8_t));
	eLinesData->bufSize = bufSize;
	
	// Initialize the number of 'e' lines
	eLinesData->nRows = 0;
	eLinesData->maxNRows = 0;
	
	return eLinesData;
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
void FreeELinesData(ELinesData *eLinesData)
{
	Free(eLinesData->start, eLinesData->maxNRows*sizeof(uint32_t));
	Free(eLinesData->size, eLinesData->maxNRows*sizeof(uint32_t));
	Free(eLinesData->strand, eLinesData->maxNRows*sizeof(uint8_t));
	Free(eLinesData->srcSize, eLinesData->maxNRows*sizeof(uint32_t));
	Free(eLinesData->status, eLinesData->maxNRows*sizeof(uint8_t));
	Free(eLinesData->statusBuffer, eLinesData->bufSize*sizeof(uint8_t));
	Free(eLinesData, sizeof(ELinesData));	
	eLinesData = NULL;	
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetELinesData(ELinesData *eLinesData)
{
	// This line is mandatory and crucial!
	eLinesData->nRows = 0;		
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetELinesBuffer(ELinesData *eLinesData)
{
	uint8_t n;
	for(n = 0; n != eLinesData->bufSize; ++n)
		eLinesData->statusBuffer[n]=0x0;
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StoreEmtpyRegionRow(ELinesData *eLinesData, uint32_t start, uint32_t size, 
	UChar strand, uint32_t srcSize, UChar status)
{
	StoreEmtpyRegionRowV2(eLinesData, start, size, (strand == '-' ? 0x0 : 0x1),
	srcSize, StatusCharacter2Symbol(status));
}
		
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
void StoreEmtpyRegionRowV2(ELinesData *eLinesData, uint32_t start, 
	uint32_t size, uint8_t strand, uint32_t srcSize, uint8_t status)
{
	uint32_t row = eLinesData->nRows;	
	// Is it necessary to allocate more memory?
	if(row >= eLinesData->maxNRows)
	{
		eLinesData->start = (uint32_t *)Realloc(eLinesData->start, 
			(eLinesData->maxNRows + 1) * sizeof(uint32_t), sizeof(uint32_t));
		eLinesData->size = (uint32_t *)Realloc(eLinesData->size, 
			(eLinesData->maxNRows + 1) * sizeof(uint32_t), sizeof(uint32_t));
		eLinesData->strand = (uint8_t *)Realloc(eLinesData->strand, 
			(eLinesData->maxNRows + 1) * sizeof(uint8_t), sizeof(uint8_t));
		eLinesData->srcSize = (uint32_t *)Realloc(eLinesData->srcSize, 
			(eLinesData->maxNRows + 1) * sizeof(uint32_t), sizeof(uint32_t));
		eLinesData->status = (uint8_t *)Realloc(eLinesData->status, 
			(eLinesData->maxNRows + 1) * sizeof(uint8_t), sizeof(uint8_t));
			
		// Update eLinesData current maximum number of rows	
		eLinesData->maxNRows++;		
	}
	eLinesData->start[row] = start;
	eLinesData->size[row] = size;
	eLinesData->strand[row] = strand;
	eLinesData->srcSize[row] = srcSize;
	eLinesData->status[row] = status;
	eLinesData->nRows++;	
}