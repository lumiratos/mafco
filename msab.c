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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msab.h"
#include "mem.h"
//#include "element.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
MSAB *CreateMSAB(uint32_t rowBlockSize, ModelsOrder *modelsOrder, 
	HashTableInfo *hashTableInfo, uint32_t maxLineSize)

{
	MSAB *msab = (MSAB *)Calloc(1, sizeof(MSAB));
	
	msab->sourceName = (char *)Calloc(hashTableInfo->maxSrcNameSize, sizeof(char));
	msab->maxSrcNameSize = hashTableInfo->maxSrcNameSize;
	msab->hashPosition = (HashPosition *)Calloc(1, sizeof(HashPosition));
	msab->line = (char *)Calloc(maxLineSize, sizeof(char));	
		
	msab->score = 0.0;
	msab->lowerCaseCount = 0;
	msab->xchrCount = 0;
	msab->rowBlockSize = rowBlockSize;
	msab->maxLineSize = maxLineSize;	
		
	msab->sLinesData = CreateSLinesData(rowBlockSize);		
	msab->qLinesData = CreateQLinesData(rowBlockSize, modelsOrder->qLinesModelOrder);	
	msab->iLinesData = CreateILinesData(modelsOrder->statusModelOrder);
	msab->eLinesData = CreateELinesData(modelsOrder->statusModelOrder);
	
	msab->linesInfo = CreateLineInfo(MEM_BLOCK_SIZE, 0x1);
	msab->eLinesInfo = CreateLineInfo(MEM_BLOCK_SIZE, 0x1);
	msab->prevELinesInfo = CreateLineInfo(MEM_BLOCK_SIZE, 0x0);
	
	msab->hashTable = CreateHashTable(hashTableInfo);
	
	return msab;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MSAB *CreateEmptyMSAB(ModelsOrder *modelsOrder, HashTableInfo *hashTableInfo)
{
	MSAB *msab = (MSAB *)Calloc(1, sizeof(MSAB));
	
	msab->sourceName = (char *)Calloc(hashTableInfo->maxSrcNameSize, sizeof(char));
	msab->maxSrcNameSize = hashTableInfo->maxSrcNameSize;
	msab->hashPosition = (HashPosition *)Calloc(1, sizeof(HashPosition));
	
	msab->line = NULL;		// Not necessary in the decoder	
	
	msab->maxLineSize = 0;	
	msab->score = 0.0;
	msab->lowerCaseCount = 0;
	msab->xchrCount = 0;
	
	// Not used in the decoder...
	//msab->rowBlockSize = 0;
	
	msab->sLinesData = CreateSLinesDataWith(0, 0);
	msab->qLinesData = CreateQLinesDataWith(0, 0, modelsOrder->qLinesModelOrder);
	msab->iLinesData = CreateILinesData(modelsOrder->statusModelOrder);
	msab->eLinesData = CreateELinesData(modelsOrder->statusModelOrder);
	
	msab->linesInfo = CreateLineInfo(MEM_BLOCK_SIZE, 0x0);
	msab->eLinesInfo = CreateLineInfo(MEM_BLOCK_SIZE, 0x0);
	msab->prevELinesInfo = CreateLineInfo(MEM_BLOCK_SIZE, 0x0);
	
	msab->hashTable = CreateHashTable(hashTableInfo);
		
	return msab;
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeMSAB(MSAB *msab)
{
	
	FreeSLinesData(msab->sLinesData);
	FreeQLinesData(msab->qLinesData);
	FreeILinesData(msab->iLinesData);
	FreeELinesData(msab->eLinesData);
	
	FreeLineInfo(msab->linesInfo);
	FreeLineInfo(msab->eLinesInfo);
	FreeLineInfo(msab->prevELinesInfo);
	
	FreeHashTable(msab->hashTable);
	Free(msab->sourceName, msab->maxSrcNameSize*sizeof(char));
	Free(msab->hashPosition, sizeof(HashPosition));
	Free(msab->line, msab->maxLineSize*sizeof(char));
	
	Free(msab, sizeof(MSAB));
	msab = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeMSABWith(MSAB *msab)
{
	FreeSLinesDataWith(msab->sLinesData);
	FreeQLinesData(msab->qLinesData);
	FreeILinesData(msab->iLinesData);
	FreeELinesData(msab->eLinesData);
	
	FreeLineInfo(msab->linesInfo);
	FreeLineInfo(msab->eLinesInfo);
	FreeLineInfo(msab->prevELinesInfo);
	
	FreeHashTable(msab->hashTable);
	
	Free(msab->sourceName, msab->maxSrcNameSize*sizeof(char));
	Free(msab->hashPosition, sizeof(HashPosition));
	Free(msab->line, msab->maxLineSize*sizeof(char));
		
	Free(msab, sizeof(MSAB));
	msab = NULL;
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		
void ResetMSAB(MSAB *msab)
{
	ResetSLinesData(msab->sLinesData);
	ResetQLinesData(msab->qLinesData);			
	ResetILinesData(msab->iLinesData);
	ResetELinesData(msab->eLinesData);
		
	msab->lowerCaseCount=0;
	msab->xchrCount=0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetMSABWith(MSAB *msab)
{
	ResetSLinesDataWith(msab->sLinesData);
	ResetQLinesDataWith(msab->qLinesData);
	ResetILinesData(msab->iLinesData);
	ResetELinesData(msab->eLinesData);
	
	msab->lowerCaseCount=0;
	msab->xchrCount=0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetBuffers(MSAB *msab)
{
	ResetQLinesBuffer(msab->qLinesData);
	ResetILinesBuffer(msab->iLinesData);
	ResetELinesBuffer(msab->eLinesData);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetPrevELinesInfo(MSAB *msab)
{
	uint32_t row;

	// Reset closeELine flag
	for(row = 0; row != msab->prevELinesInfo->currentNRows; ++row)
	{
		// Set to 0x0 this flag
		msab->prevELinesInfo->elements[row]->closeELine = 0x0;	
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetELinesInfo(MSAB *msab)
{
	uint32_t row;
	
	ResetPrevELinesInfo(msab);

	// Allocate memory if necessary
	IncreaseLineInfo(msab->prevELinesInfo, msab->eLinesInfo->currentNRows);

	for(row = 0; row != msab->eLinesInfo->currentNRows; ++row)
	{
		// Set to 0x1 the elements of each 'e' line
		msab->eLinesInfo->elements[row]->closeELine = 0x1;		
	
		// Store the 'e' line elements (only the pointers)
		msab->prevELinesInfo->elements[row] = msab->eLinesInfo->elements[row];
	}
	// Update the current number of rows
	msab->prevELinesInfo->currentNRows = msab->eLinesInfo->currentNRows;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		
void GetNPlaces(MSAB *msab, uint8_t *nPlaces)
{
	uint8_t nP0=0, nP1=0, nP2=0, nP3=0, row;
	nPlaces[0] = nPlaces[1] = nPlaces[2] = nPlaces[3] = 0;
	
	for(row = 0; row != msab->sLinesData->nRows; ++row)
	{
		nP0 = Strlen(msab->linesInfo->elements[row]->sourceName);
		nP1 = GetNumberOfDigits(msab->linesInfo->elements[row]->start);
		nP2 = GetNumberOfDigits(msab->linesInfo->elements[row]->seqSize);
		nP3 = GetNumberOfDigits(msab->linesInfo->elements[row]->sourceSize);
		
		nPlaces[0] = (nP0 > nPlaces[0] ? nP0 : nPlaces[0]);
		nPlaces[1] = (nP1 > nPlaces[1] ? nP1 : nPlaces[1]);
		nPlaces[2] = (nP2 > nPlaces[2] ? nP2 : nPlaces[2]);
		nPlaces[3] = (nP3 > nPlaces[3] ? nP3 : nPlaces[3]);
	}
	
	for(row = 0; row != msab->eLinesInfo->currentNRows; ++row)
	{
		nP0 = Strlen(msab->eLinesInfo->elements[row]->sourceName);
		nP1 = GetNumberOfDigits(msab->eLinesInfo->elements[row]->start);
		//nP2 = GetNumberOfDigits(msab->eLinesInfo->elements[row]->seqSize);
		nP3 = GetNumberOfDigits(msab->eLinesInfo->elements[row]->sourceSize);
		
		nPlaces[0] = (nP0 > nPlaces[0] ? nP0 : nPlaces[0]);
		nPlaces[1] = (nP1 > nPlaces[1] ? nP1 : nPlaces[1]);
		//nPlaces[2] = (nP2 > nPlaces[2] ? nP2 : nPlaces[2]);
		nPlaces[3] = (nP3 > nPlaces[3] ? nP3 : nPlaces[3]);
		
	}

	// Compute the total of places
	nPlaces[4] = nPlaces[0] + nPlaces[1] + nPlaces[2] + nPlaces[3];
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -