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

#ifndef ELEMENT_H_INCLUDED
#define ELEMENT_H_INCLUDED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct 
{
	char 		*sourceName;		// Source name specie.chromossome, 
									// specie.scaffold, etc.
	
	uint8_t		sStatus;			// Indicates if it is necessary to encode
									// sourceName, sourceSize, etc.
	
	uint8_t		srcNameMaxSize;		// Maximum characters allowed in the source
									// name
	uint32_t	start;				// Start position where the alignement 
									// begins
	uint32_t	prevStart;			// Previous start position in chromosome
	uint32_t	seqSize;			// Row base count of the current MSAB 
									// (without the gaps)
	uint32_t	currentSeqSize;		// Current base count
	
	uint8_t		strand;				// DNA strand. '-' if the alignment is to 
									// the reverse-complement source 
	uint32_t	sourceSize;			// Source/Chromosome length
	
	uint8_t		qualityInfo;		// Indicates if the current element has quality 
									// information ('q' lines)
	
	// Information about the 'i' lines
	
	uint8_t		iStatus;			// Indicates if an 'i' line was already
									// processed or not

	uint8_t		lastRightStatus;	// Stores the last right status symbol encoded
	
	uint32_t	lastRightCount;		// Stores the last right count encoded
	
	
	// Information about the 'e' lines
	
	uint8_t		closeELine;			// Flag that indicates the presence of an 'e'
									// line of the current source in the previous
									// MSAB
	uint8_t		lastStatusSymbol;	// Stores the last status symbol processed

} Element;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	uint32_t 	hashKey;				// The hash key correspond to the row 
										// coordinate of an element of the hash table
	uint8_t		elementId;				// The element ID correspond to the column 
										// coordinate of an element of the hash table
} HashPosition;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	uint32_t		nRows;				// Current number of allocated rows
	uint32_t		currentNRows;		// Current number of rows stored
	uint8_t			hashPositionsFlag;	// Indicates if the hashPositions array is necessary or not
	Element 		**elements;			// Pointers for the elements of the hash table
	HashPosition	*hashPositions;		// Hash key and element ID
} LineInfo;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Element 	*CreateEmptyElement	(uint8_t);
Element 	*CreateElement		(uint8_t, const char *, uint32_t, uint32_t, 
								uint8_t, uint32_t);
void 		FreeElement			(Element *);
void 		CopyElement			(Element *, Element *);
Element		*CloneElement		(Element *);
void		SetElement			(Element *, const char *, uint32_t, uint32_t, 
								uint8_t, uint32_t);
uint8_t		IsElementEmpty		(Element *);

LineInfo	*CreateLineInfo		(uint32_t, uint8_t);
void 		FreeLineInfo		(LineInfo *);
void 		IncreaseLineInfo	(LineInfo *, uint32_t);
void 		StoreLineInfo		(LineInfo *, Element *, HashPosition *, 
								uint32_t, const char *, uint32_t, uint32_t,
								uint8_t, uint32_t, uint8_t);
								
void 		InsertElement		(LineInfo *, Element *, uint32_t);

//void		StoreSLineInfo		(LineInfo *, Element *, HashPosition *, 
//								uint32_t, const char *, uint32_t, uint32_t, 
//								uint8_t, uint32_t, uint8_t);


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif