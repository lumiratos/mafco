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

#include "defs.h"
#include "mem.h"
#include "ilines.h"
#include "common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ILinesData *CreateILinesData(uint8_t bufSize)
{
	ILinesData *iLinesData = (ILinesData *)Calloc(1, sizeof(ILinesData));
	
	iLinesData->statusBuffer = (uint8_t *)Calloc(bufSize, sizeof(uint8_t));
	iLinesData->bufSize = bufSize;
	
	// Initialize the number of 'i' lines
	iLinesData->nRows = 0;
	iLinesData->maxNRows = 0;
	
	return iLinesData;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeILinesData(ILinesData *iLinesData)
{
	Free(iLinesData->leftStatus, iLinesData->maxNRows*sizeof(uint8_t));
	Free(iLinesData->rightStatus, iLinesData->maxNRows*sizeof(uint8_t));
	Free(iLinesData->leftCount, iLinesData->maxNRows*sizeof(uint32_t));
	Free(iLinesData->rightCount, iLinesData->maxNRows*sizeof(uint32_t));
	Free(iLinesData->statusBuffer, iLinesData->bufSize*sizeof(uint8_t));
	Free(iLinesData, sizeof(ILinesData));	
	iLinesData = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
void ResetILinesData(ILinesData *iLinesData)
{
	// This is mandatory and crucial!
	iLinesData->nRows = 0;	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetILinesBuffer(ILinesData *iLinesData)
{
	uint8_t n;
	
	for(n = 0; n != iLinesData->bufSize; ++n)
			iLinesData->statusBuffer[n]=0x0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StoreInfoRow(ILinesData *iLinesData, UChar leftStatus, 
	UChar rightStatus, uint32_t leftCount, uint32_t rightCount)
{
	StoreInfoRowV2(iLinesData, StatusCharacter2Symbol(leftStatus), 
		StatusCharacter2Symbol(rightStatus), leftCount, rightCount);
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	
void StoreInfoRowV2(ILinesData *iLinesData, uint8_t leftStatus, 
	uint8_t rightStatus, uint32_t leftCount, uint32_t rightCount)
{
	uint32_t row = iLinesData->nRows;
	
	// Is it necessary to allocate more memory?
	if(row >= iLinesData->maxNRows)
	{
		iLinesData->leftStatus = (uint8_t *)Realloc(iLinesData->leftStatus, 
			(iLinesData->maxNRows + 1) * sizeof(uint8_t), sizeof(uint8_t));
		iLinesData->rightStatus = (uint8_t *)Realloc(iLinesData->rightStatus, 
			(iLinesData->maxNRows + 1) * sizeof(uint8_t), sizeof(uint8_t));
		iLinesData->leftCount = (uint32_t *)Realloc(iLinesData->leftCount, 
			(iLinesData->maxNRows + 1) * sizeof(uint32_t), sizeof(uint32_t));
		iLinesData->rightCount = (uint32_t *)Realloc(iLinesData->rightCount, 
			(iLinesData->maxNRows + 1) * sizeof(uint32_t), sizeof(uint32_t));	
		
		// Update iLinesData current maximum number of rows	
		iLinesData->maxNRows++;	
		
	}
				
	// Store the 'i' row information
	iLinesData->leftStatus[row] = leftStatus;
	iLinesData->rightStatus[row] = rightStatus;
	iLinesData->leftCount[row] = leftCount;
	iLinesData->rightCount[row] = rightCount;
	iLinesData->nRows++;	
}
		
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	