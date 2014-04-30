// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Copyright 2013/2014 IEETA/DETI - University of Aveiro, Portugal.		 -
//	All Rights Reserved.													 -
//																			 -
//	These programs are supplied free of charge for research purposes only,   -
//	and may not be sold or incorporated into any commercial product. There   -
//	is ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they     -
//	are fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you	 - 
//	do happen to find a bug, or have modifications to suggest, please 		 -
//	report the same to Armando J. Pinho, ap@ua.pt. The copyright notice      -
//	above and this statement of conditions must remain an integral part of   -
//	each and every copy made of these files.								 -
//																			 -
//	Description: functions for handling finite-context models.				 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "defs.h"
#include "mem.h"
#include "context.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//	11    	7		3
//	10      6 		2
//	9		5		1
//	8   	4		X
static Coords templateA[] = {	
								{-1,  0}, {-2,  0}, {-3,  0},
								{ 0, -1}, {-1, -1}, {-2, -1}, {-3, -1},
								{ 0, -2}, {-1, -2}, {-2, -2}, {-3, -2}
							};

//							3
//					6		2
//			8		5		1
//	9		7		4		X
static Coords templateB[] = {	
								{-1,  0}, {-2,  0}, {-3,  0},
								{ 0, -1}, {-1, -1}, {-2,  -1},
								{ 0, -2}, {-1, -2},
								{ 0, -3}
							};

//							4
//					8		3
//			11		7		2
//	13		10		6		1
//	12		9		5		X
static Coords templateC[] = {	
								{-1,  0}, {-2,  0}, {-3,  0}, {-4,  0},
								{ 0, -1}, {-1, -1}, {-2, -1}, {-3, -1},
								{ 0, -2}, {-1, -2}, {-2, -2}, 
								{ 0, -3}, {-1, -3} 
							};

//					5
//			10		4
//	14		9		3
//	13		8		2
//	12		7		1
//	11		6		X
static Coords templateD[] = {	
								{-1,  0}, {-2,  0}, {-3,  0}, {-4,  0}, {-5,  0},
								{ 0, -1}, {-1, -1}, {-2, -1}, {-3, -1}, {-4, -1},
								{ 0, -2}, {-1, -2}, {-2, -2}, {-3, -2}
							};

//				12
//	   	11	10	6   9   
//		8   4	2   3   7
//	13	5   1   X
static Coords templateE[] = {	
								{ 0, -1}, {-1,  0}, {-1,  1}, {-1, -1}, 
								{ 0, -2}, {-2,  0}, {-1,  2}, {-1, -2}, 
								{-2,  1}, {-2, -1}, {-2, -2}, {-3,  0}, {0, -3}
							};




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CModel *CreateCModel(uint32_t ctxSize, uint32_t nSymbols, uint32_t deltaNum, 
	uint32_t deltaDen)
{
	uint32_t  n, prod = 1;
	CModel    *cModel;

	if(ctxSize < 1)
	{
		fprintf(stderr, "Error (CreateCModel): context size must be greater than 0!\n");
		fprintf(stderr, "Error (CreateCModel): context size read = %"PRIu32"\n", ctxSize);
		exit(EXIT_FAILURE);
	}
	
	if(nSymbols < 1)
	{
		fprintf(stderr, "Error (CreateCModel): number of symbols must be greater than 0!\n");
		fprintf(stderr, "Error (CreateCModel): number of symbols read = %"PRIu32"\n", nSymbols);
		exit(EXIT_FAILURE);
	}

	cModel               = (CModel   *) Calloc(1,       sizeof(CModel  ));
	cModel->multipliers  = (uint32_t *) Calloc(ctxSize, sizeof(uint32_t));
	cModel->nPModels     = (uint64_t)   pow(nSymbols, ctxSize);
	cModel->ctxSize      = ctxSize;
	cModel->nSymbols     = nSymbols;
	cModel->pModelIdx    = 0;
	cModel->deltaNum = deltaNum;
	cModel->deltaDen = deltaDen;

	switch(nSymbols)
	{
    	case 2:
			cModel->kMinusOneMask = (0x01 << (ctxSize - 1)) - 1;
			break;

		case 4:
			cModel->kMinusOneMask = (0x01 << 2 * (ctxSize - 1)) - 1;
			break;
    }

	for(n = 0 ; n != ctxSize ; ++n)
    {
		cModel->multipliers[n] = prod;
    	prod                  *= nSymbols;
    }
	
	cModel->counters = (ACCounter *) Calloc(cModel->nPModels * nSymbols,
		sizeof(ACCounter));
	return cModel;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
void FreeCModel(CModel *cModel)
{
	Free(cModel->counters, cModel->nPModels * cModel->nSymbols * sizeof(ACCounter));
	Free(cModel->multipliers, cModel->ctxSize * sizeof(uint32_t));
	Free(cModel, sizeof(CModel));
	cModel = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetCModel(CModel *cModel)
{
	uint32_t idx;
	for(idx = 0; idx < (cModel->nPModels * cModel->nSymbols); ++idx)
			cModel->counters[idx] = 0;
	
	// This is crucial for the binary models
	cModel->pModelIdx = 0x0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void	GetPModelIdx(MSAB *msab, uint32_t row, uint32_t col, 
	CTemplate *cTemplate, CModel *cModel)
{
	UChar c;
	uint8_t s, n;
	uint32_t idx=0;
	for(n=0; n != cTemplate->size; ++n) 
	{
		c = GetSequenceValueCharacterV2(msab->sLinesData, 
			(int64_t)row+(int64_t)cTemplate->position[n].row, 
			(int64_t)col+(int64_t)cTemplate->position[n].col);
		s = SequenceValueCharacter2Symbol(c);
		idx += s * cModel->multipliers[n];
	}
	cModel->pModelIdx = idx;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void GetPModelIdx2(UChar lastSymbol, CModel *cModel)
{
	cModel->pModelIdx = ((cModel->pModelIdx & cModel->kMinusOneMask) << 1) + lastSymbol;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void GetPModelIdx3(uint8_t *buf, uint8_t bufSize, CModel *cModel)
{
	uint32_t idx=0;
	uint8_t n, symbol;
  
	for(n=0; n != bufSize; ++n)
	{
		symbol=buf[bufSize - n - 1];
		// The folowing if should be activated for debugging purposes
		/*
		if(symbol >= cModel->nSymbols)
		{
			fprintf(stderr, "Error(GetPModelIdx3): something went very wrong!!!\n");
			fprintf(stderr, "Error(GetPModelIdx3): buffer symbol %"PRIu8" at position %"PRIu8"\n", symbol, n);
			fprintf(stderr, "Error(GetPModelIdx3): number of symbols for this model = %"PRIu32"\n", cModel->nSymbols);
			exit(EXIT_FAILURE);
		}
		*/
		idx += symbol * cModel->multipliers[n];
    }
  
	cModel->pModelIdx = idx; 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void ComputePModel(CModel *cModel, ac_model *acm)
{
	ACCounter *ACounters;  
	uint8_t nSymbols = cModel->nSymbols - 1, n;

	ACounters = &cModel->counters[cModel->pModelIdx*cModel->nSymbols];	
	acm->cfreq[nSymbols] = cModel->deltaNum + cModel->deltaDen * ACounters[nSymbols];

	for(n = nSymbols ; n-- ; )
		acm->cfreq[n] = acm->cfreq[n+1] + cModel->deltaNum + cModel->deltaDen * ACounters[n];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void ComputePModel2(CModel *cModel, ac_model *acm)
{
	ACCounter *ACounters;  
	
	ACounters = &cModel->counters[cModel->pModelIdx << 1]; // same as multiplying by 2	
	acm->cfreq[1] =                 cModel->deltaNum + cModel->deltaDen * ACounters[1];
	acm->cfreq[0] = acm->cfreq[1] + cModel->deltaNum + cModel->deltaDen * ACounters[0];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void UpdateCModelCounter(CModel *cModel, UChar symbol)
{
	ACCounter *ACounters = NULL;  
	uint8_t n;

	ACounters = &cModel->counters[cModel->pModelIdx*cModel->nSymbols];	

	// Update symbol counter
	if(++ACounters[symbol] == MAXCNT)		// Check max counter overflow
		for(n = cModel->nSymbols ; n-- ; )
        	ACounters[n] >>= 1;    		// x/2 & update
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void	UpdateCModelIdx(CModel *cModel, UChar symbol)
{
	cModel->pModelIdx = ((cModel->pModelIdx & cModel->kMinusOneMask) << 1) + symbol; 
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CTemplate *InitTemplate(UChar templateId, uint8_t templateSize)
{
	CTemplate *cTemplate = NULL;
	cTemplate = Malloc(sizeof(CTemplate));

	switch(templateId)
  	{
  		case 'A':
			cTemplate->position = templateA;
	  
			if(templateSize > (sizeof(templateA) / sizeof(templateA[0])) )
				cTemplate->size = sizeof(templateA) / sizeof(templateA[0]);
			else
				cTemplate->size = templateSize;
			break;
		
		case 'B':
			cTemplate->position = templateB;
	  
			if(templateSize > (sizeof(templateB) / sizeof(templateB[0])) )
				cTemplate->size = sizeof(templateB) / sizeof(templateB[0]);
			else
				cTemplate->size = templateSize;
			break;
	
		case 'C':
			cTemplate->position = templateC;
	  
			if(templateSize > (sizeof(templateC) / sizeof(templateC[0])) )
				cTemplate->size = sizeof(templateC) / sizeof(templateC[0]);
			else
				cTemplate->size = templateSize;
			break;
			
		case 'D':
			cTemplate->position = templateD;
	  
			if(templateSize > (sizeof(templateD) / sizeof(templateD[0])) )
				cTemplate->size = sizeof(templateD) / sizeof(templateD[0]);
			else
				cTemplate->size = templateSize;
			break;
				
		case 'E':
			cTemplate->position = templateE;
	  
			if(templateSize > (sizeof(templateE) / sizeof(templateE[0])) )
				cTemplate->size = sizeof(templateE) / sizeof(templateE[0]);
			else
				cTemplate->size = templateSize;
			break;
		
		default:
			// FIXME
			// Latter put here the best template ID and its size
			fprintf(stderr, "Error(InitTemplate): invalid template id (%c)\n", templateId);
			exit(EXIT_FAILURE);
	}  
	return cTemplate;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeTemplate(CTemplate *cTemplate)
{
	Free(cTemplate, sizeof(CTemplate));	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ShowTemplate(CTemplate *cTemplate)
{
	int8_t minRow, maxRow, minCol, maxCol, n, row, col;
	int8_t **templateMatrix=NULL;
	minRow = maxRow = cTemplate->position[0].row;
	minCol = maxCol = cTemplate->position[0].col;
  
	for(n = 1 ; n != cTemplate->size ; ++n)
 	{
		if(cTemplate->position[n].row > maxRow)
			maxRow = cTemplate->position[n].row;

		if(cTemplate->position[n].row < minRow)
			minRow = cTemplate->position[n].row;

		if(cTemplate->position[n].col > maxCol)
			maxCol = cTemplate->position[n].col;

		if(cTemplate->position[n].col < minCol)
			minCol = cTemplate->position[n].col;
	}

	templateMatrix = (int8_t **)Calloc(maxRow - minRow + 2, sizeof(int8_t *));

	for(row = 0 ; row != maxRow - minRow + 2 ; ++row)
		templateMatrix[row] = (int8_t *)Calloc(maxCol - minCol + 2, sizeof(int8_t));
	
	for(n = 0 ; n < cTemplate->size ; n++)
		templateMatrix[cTemplate->position[n].row - minRow][cTemplate->position[n].col - minCol] = n + 1;

	templateMatrix[-minRow][-minCol] = -1;
	for(row = 0 ; row != maxRow - minRow + 2 ; ++row)
	{
		for(col = 0 ; col != maxCol - minCol + 2 ; ++col)
			if(templateMatrix[row][col])
			{
				if(templateMatrix[row][col] == -1)
					printf("  X");
				else
					printf("%3"PRIu8"", templateMatrix[row][col]);
			}
			else printf("   ");

		putchar('\n');
	}
	
  	for(row = 0 ; row != maxRow - minRow + 2 ; ++row)
  		Free(templateMatrix[row], (maxCol-minCol+2)* sizeof(int8_t));
  	Free(templateMatrix, (maxRow - minRow + 2) * sizeof(int8_t *));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -