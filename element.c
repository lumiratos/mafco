// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Copyright 2014 IEETA/DETI - University of Aveiro, Portugal.		 		 -
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
//	Description: functions and structure for the hash table.	 		 	 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "defs.h"
#include "mem.h"
#include "element.h"
#include "common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Element *CreateEmptyElement(uint8_t srcNameMaxSize)
{
	Element *element = (Element *)Calloc(1, sizeof(Element));
	
	element->sourceName = (char *)Calloc(srcNameMaxSize, sizeof(char));
	element->srcNameMaxSize = srcNameMaxSize;
	
	// This is very important for the encoder know that this element is empty 
	element->sourceName[0] = '\0';
	return element;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Element *CreateElement(uint8_t srcNameMaxSize, const char *sourceName,
	uint32_t start, uint32_t seqSize, uint8_t strand, uint32_t sourceSize)
{
	Element *element = CreateEmptyElement(srcNameMaxSize);

	Strcpy(element->sourceName, sourceName, srcNameMaxSize);
	element->start = start;
	//element->prevStart = 0;
	element->seqSize = seqSize;
	element->strand = strand;
	element->sourceSize = sourceSize;
	
	return element;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeElement(Element *element)
{
	Free(element->sourceName, element->srcNameMaxSize * sizeof(char));
	Free(element, sizeof(Element));
	element = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CopyElement(Element *destElement, Element *srcElement)
{
	uint8_t additionalSpace=0;
	//if(Strlen(srcElement->sourceName) > destElement->srcNameMaxSize)
	if(srcElement->srcNameMaxSize > destElement->srcNameMaxSize)
	{
		additionalSpace = (srcElement->srcNameMaxSize - destElement->srcNameMaxSize);
		// Reallocate enought memory to perform the string copy
		Realloc(destElement->sourceName, destElement->srcNameMaxSize * sizeof(char), additionalSpace);
		destElement->srcNameMaxSize = srcElement->srcNameMaxSize;
	}
	Strcpy(destElement->sourceName, srcElement->sourceName, srcElement->srcNameMaxSize);
	destElement->start = srcElement->start;
	destElement->prevStart = srcElement->prevStart;
	destElement->seqSize = srcElement->seqSize;
	destElement->strand = srcElement->strand;
	destElement->sourceSize = srcElement->sourceSize;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Element	*CloneElement(Element *element)
{
	return CreateElement(element->srcNameMaxSize, element->sourceName,
		element->start, element->seqSize, element->strand, element->sourceSize);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SetElement(Element *element, const char *sourceName,
	uint32_t start, uint32_t seqSize, uint8_t strand, uint32_t sourceSize)
{
	if(element == NULL)
	{
		fprintf(stderr, "Error(SetElement): unable to set fields of a NULL element.\n");
		exit(EXIT_FAILURE);
	}
	
	Strcpy(element->sourceName, sourceName, element->srcNameMaxSize);
	element->start = start;
	//element->prevStart = 0;
	element->seqSize = seqSize;
	element->strand = strand;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t IsElementEmpty (Element *element)
{
	if(element == NULL)
	{
		fprintf(stderr, "Error (IsElementEmpty): NULL element pointer!\n");
		exit(EXIT_FAILURE);
	}
	// If the first character is a '\0' it means that it is an empty element
	return ( (element->sourceName[0] == '\0') ? 0x1 : 0x0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

LineInfo *CreateLineInfo(uint32_t nRows, uint8_t hashPositionsFlag)
{
	uint32_t n;
	LineInfo *lineInfo = (LineInfo *)Calloc(1, sizeof(LineInfo));
	lineInfo->elements = (Element **)Calloc(nRows, sizeof(Element *));
	
	for(n = 0; n != nRows; ++n)
	{
			lineInfo->elements[n] = NULL;
	}
	
	// If hashPositionsFlag == 0x1, then allocate memory for the hash positions array
	// Else set to NULL
	lineInfo->hashPositions = ((hashPositionsFlag == 0x1) ? 
		(HashPosition *)Calloc(nRows, sizeof(HashPosition)) : NULL);
	
	lineInfo->hashPositionsFlag = hashPositionsFlag;	
	lineInfo->nRows = nRows;
	lineInfo->currentNRows = 0;
	
	return lineInfo;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeLineInfo(LineInfo *lineInfo)
{
	Free(lineInfo->elements, lineInfo->nRows*sizeof(Element *));

	// Only Free if there was any memory allocation before
	if(lineInfo->hashPositionsFlag == 0x1)
		Free(lineInfo->hashPositions, lineInfo->nRows*sizeof(HashPosition));

	Free(lineInfo, sizeof(LineInfo));
	lineInfo = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void IncreaseLineInfo(LineInfo *lineInfo, uint32_t row)
{
	uint32_t additionalSize, newSize = row+1;
	
	if(lineInfo == NULL)
	{
		fprintf(stderr, "Error (IncreaseLineInfo): NULL LineInfo pointer!\n");
		exit(EXIT_FAILURE);
	}
	
	if(newSize > lineInfo->nRows)
	{
		if((newSize+lineInfo->nRows) > UINT32_MAX)
		{
			fprintf(stderr, "Error (IncreaseLineInfo): overflow detected!\n");
			fprintf(stderr, "Error (IncreaseLineInfo): maximum size allowed in this context is %"PRIu32"!\n", UINT32_MAX);
			fprintf(stderr, "Error (IncreaseLineInfo): size computed is %"PRIu64".\n", (uint64_t)(newSize+lineInfo->nRows));
			exit(EXIT_FAILURE);
		}
		
		additionalSize = newSize - lineInfo->nRows;
		
		// Realloc procedures
		lineInfo->elements = (Element **)Realloc(lineInfo->elements, 
			newSize*sizeof(Element *), additionalSize*sizeof(Element *));
			
		if(lineInfo->hashPositionsFlag == 0x1)
		{
			lineInfo->hashPositions = (HashPosition *)Realloc(lineInfo->hashPositions, 
				newSize*sizeof(HashPosition), additionalSize*sizeof(HashPosition));
		}
		
		// Update the new number of allocated rows
		lineInfo->nRows = newSize;
	}
	// Do nothing here. The function does not require to allocate memory if the 
	// the current size is enought to store the line information
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void StoreLineInfo(LineInfo *lineInfo, Element *element, 
	HashPosition *hashPosition, uint32_t row, const char *sourceName, 
	uint32_t start, uint32_t seqSize, uint8_t strand, uint32_t sourceSize,
	uint8_t encodingFlag)
{
	// First verify if the number of rows allocated in the variable "lineInfo"
	// Allocate memory and update the number of allocated rows, if necessary
	IncreaseLineInfo(lineInfo, row);
	
	// Adress atribution
	lineInfo->elements[row] = element;
	
	// If the element is empty it means that it a new source name that is beeing processed
	if(IsElementEmpty(element))
	{
		// Copy the string source name
		Strcpy(lineInfo->elements[row]->sourceName, sourceName, lineInfo->elements[row]->srcNameMaxSize);
		
		// Save the information regarding to the source/chromossome size
		lineInfo->elements[row]->sourceSize = sourceSize;			
	}
	
	// Encoder
	if(encodingFlag == 0x1)
	{
		// If the element is not new (was processed before)
		lineInfo->elements[row]->prevStart = lineInfo->elements[row]->start;
		lineInfo->elements[row]->start = start;
	}
	// Decoder
	else
	{
		lineInfo->elements[row]->prevStart = start;
		lineInfo->elements[row]->start = start;
	}
	lineInfo->elements[row]->strand = strand;
			
	// Only store this information in the Encoder. The decoder does not need this
	if(lineInfo->hashPositionsFlag == 0x1)
	{
		// Store the hash key and the element id to be encoded latter
		lineInfo->hashPositions[row].hashKey = hashPosition->hashKey;
		lineInfo->hashPositions[row].elementId = hashPosition->elementId;
		
		// Save the sequence size field to be used latter
		lineInfo->elements[row]->currentSeqSize = seqSize;
	}
	
	// Update the current number of rows that are stored in this structure
	lineInfo->currentNRows = row + 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void InsertElement(LineInfo *lineInfo, Element *element, uint32_t row)
{
	// First verify if the number of rows allocated in the variable "lineInfo"
	// Allocate memory and update the number of allocated rows, if necessary
	IncreaseLineInfo(lineInfo, row);
	
	// Adress atribution
	lineInfo->elements[row] = element;
	
	// Update the current number of rows that are stored in this structure
	lineInfo->currentNRows = row + 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -