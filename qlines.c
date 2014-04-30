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

#include <stdio.h>
#include <stdlib.h>
#include "qlines.h"
#include "mem.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

QLinesData *CreateQLinesData(uint32_t rowBlockSize, uint8_t bufSize)
{
	QLinesData *qLinesData = (QLinesData *)Calloc(1, sizeof(QLinesData));

	qLinesData->buffer = (uint8_t *)Calloc(bufSize, sizeof(uint8_t));
	qLinesData->bufSize = bufSize;
	qLinesData->rowBlockSize = rowBlockSize;	
	qLinesData->nRows=0;
	qLinesData->nCols=0;					
	qLinesData->data = NULL;
	return qLinesData;	
}
		
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

QLinesData 	*CreateQLinesDataWith(uint32_t nRows, uint32_t nCols, 
									uint8_t bufSize)
{
	uint32_t n;
	QLinesData *qLinesData = (QLinesData *)Calloc(1, sizeof(QLinesData));

	qLinesData->buffer = (uint8_t *)Calloc(bufSize, sizeof(uint8_t));
	qLinesData->bufSize = bufSize;
	
	// Not necessary in the decoder
	//qLinesData->rowBlockSize = 0;	
	qLinesData->nRows=nRows;
	qLinesData->nCols=nCols;					
	qLinesData->data = NULL;
	
	// Only allocate memory if necessary
	if( (nRows != 0) && (nCols != 0) )
	{
		qLinesData->data = (uint8_t **)Calloc(nRows, sizeof(uint8_t *));
		for(n = 0; n != qLinesData->nRows; ++n)
			qLinesData->data[n] = (uint8_t *)Calloc(nCols, sizeof(uint8_t));	
	}	
	return qLinesData;		
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResizeQLinesData (QLinesData *qLinesData, uint32_t nRows, uint32_t nCols)
{
	uint32_t n;
	if(qLinesData == NULL)
	{
		fprintf(stderr, "Error (ResizeQLinesData) unable to resize a NULL structure.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		qLinesData->nRows=nRows;
		qLinesData->nCols=nCols;
		
		qLinesData->data = (uint8_t **)Calloc(nRows, sizeof(uint8_t *));
		for(n = 0; n != qLinesData->nRows; ++n)
			qLinesData->data[n] = (uint8_t *)Calloc(nCols, sizeof(uint8_t));
	}
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeQLinesData(QLinesData *qLinesData)
{
	ResetQLinesData(qLinesData);
	Free(qLinesData->buffer, qLinesData->bufSize * sizeof(uint8_t));
	Free(qLinesData, sizeof(QLinesData));	
	qLinesData = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		
void FreeQLinesDataWith(QLinesData *qLinesData)
{
	ResetQLinesDataWith(qLinesData);
	Free(qLinesData->buffer, qLinesData->bufSize * sizeof(uint8_t));	
	Free(qLinesData, sizeof(QLinesData));	
	qLinesData = NULL;
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetQLinesData(QLinesData *qLinesData)
{
	uint32_t n, rowBlockSize = qLinesData->rowBlockSize;
	
	if( (qLinesData != NULL) && (qLinesData->nRows >0) )
	{
		for(n = 0 ; n != qLinesData->nRows; ++n)
		{
			Free(qLinesData->data[n], (((qLinesData->nCols-1) / rowBlockSize) + 1) *
				rowBlockSize * sizeof(uint8_t));	
		}
	}	
	Free(qLinesData->data, qLinesData->nRows * sizeof(uint8_t *));
	
	// Reset buffer
	//for(n = 0; n != qLinesData->bufSize; ++n)
	//	qLinesData->buffer[n] = 0x0;
	
	qLinesData->nRows = 0;
	qLinesData->nCols = 0;
	qLinesData->data = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetQLinesDataWith(QLinesData *qLinesData)
{
	uint32_t n;
	if( (qLinesData != NULL) && (qLinesData->nRows > 0) )
	{
		for(n = 0; n != qLinesData->nRows; ++n)
			Free(qLinesData->data[n], qLinesData->nCols*sizeof(uint8_t));
		Free(qLinesData->data,  qLinesData->nRows * sizeof(uint8_t *));
		
		// Reset buffer
		//for(n = 0; n != qLinesData->bufSize; ++n)
		//	qLinesData->buffer[n] = 0x0;
		
		qLinesData->nRows = 0;
		qLinesData->nCols = 0;
		qLinesData->data = NULL;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetQLinesBuffer(QLinesData *qLinesData)
{
	uint32_t n;
	// Reset buffer
	for(n = 0; n != qLinesData->bufSize; ++n)
		qLinesData->buffer[n] = 0x0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	qualityValuesRow 		- The read quality values
// 	qualityValuesRowSize	- The number of quality values read until now (current row)
//  rowBlockSize			- slot for memory allocation
void StoreQualityValue(uint8_t **qualityValuesRow, uint32_t qualityValuesRowSize, 
	uint32_t rowBlockSize, UChar c)
{
	if(qualityValuesRowSize % rowBlockSize == 0)
	{
		*qualityValuesRow = (uint8_t *)Realloc(*qualityValuesRow, sizeof(uint8_t) *
			(qualityValuesRowSize + rowBlockSize), sizeof(uint8_t) * rowBlockSize);
	}

	// Store the quality score symbol
	(*qualityValuesRow)[qualityValuesRowSize++] = QualityValue2Symbol(c);
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
void StoreQualityRow(QLinesData *qLinesData, uint8_t *newRow, uint32_t rowSize)
{
	if(qLinesData->nCols && rowSize != qLinesData->nCols)
	{
		fprintf(stderr, "Error (StoreQualityRow): trying to add a quality row "
						"with different size.\n");
		fprintf(stderr, "Error (StoreQualityRow): current quality MSAB row "
						"size (number of columns) = %"PRIu32".\n", 
						qLinesData->nCols);
		fprintf(stderr, "Error (StoreQualityRow): try to add a row with = "
						"%"PRIu32" elements.\n", rowSize);	
		exit(EXIT_FAILURE);
	}
		
	qLinesData->data = (uint8_t **)Realloc(qLinesData->data, 
		(qLinesData->nRows + 1) * sizeof(uint8_t *), sizeof(uint8_t *));

	qLinesData->data[qLinesData->nRows] = newRow;
	qLinesData->nRows++;
	
	// Set the number of column in case we are storing the first row
	if(!qLinesData->nCols)
		qLinesData->nCols = rowSize;
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t GetQualityValue(QLinesData *qLinesData, uint32_t row, uint32_t col)
{
	if( (row < qLinesData->nRows) && (col < qLinesData->nCols) )
		return qLinesData->data[row][col];
	else
		return 0;
}		

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
uint8_t QualityValue2Symbol(UChar c)
{
	switch(c)
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':	return c - '0';			// This will return the numeric 
											// correspondent
		case 'F':	return 10;
		case '.':	return 11;				// Sinalize missing entries
		case '-':	return Q_GAP_SYMBOL;	// return 12;
		default:
			fprintf(stderr, "Error (QualityValue2Symbol): Unexpected quality "
							"value character: %c (ascii %"PRIu8")\n", c, c);
			exit(EXIT_FAILURE);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
UChar Symbol2QualityValue(uint8_t s)
{
	switch(s)
	{
		case 0: return '0';
		case 1: return '1';
		case 2: return '2';
		case 3: return '3';
		case 4: return '4';
		case 5: return '5';
		case 6: return '6';
		case 7: return '7';
		case 8: return '8';
		case 9: return '9';
		case 10: return 'F';
		case 11: return '.';
		//case 12: return '-';
		case Q_GAP_SYMBOL: return '-';
		default: 
			fprintf(stderr, "Error (Symbol2QualityValue): Unexpected quality "
							"value symbol: %"PRIu8"\n", s);
			exit(EXIT_FAILURE);
	}
}
			
