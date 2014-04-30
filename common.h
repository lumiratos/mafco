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
//	Description: functions for handling with files, strings, etc.			 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct 
{
	uint32_t		maxMSABNRows;				// Maximum number of rows allowed
	uint32_t		maxMSABNCols;				// Maximum number of columns allowed
	uint32_t		maxHeaderLineSize;			// Maximum number of characters for header lines allowed
	uint32_t		maxLineSize;				// Buffer size used in fgets function
	uint32_t		maxAbsScoreValue;			// Maximum absolute score value
	uint32_t		maxSrcNameSize;				// Maximum number of character in specie.chr source
	uint32_t		maxSourceSize;				// Maximum source/chromossome size
	uint32_t		maxStartPosition;			// Maximum start position
	uint8_t			maxStartOffset;				// Maximum start offset used
	
	uint32_t		maxCountValue;				// Maximum count value of the 'i' lines
	
	//uint32_t		maxHashKeyValue;			// The same as the size of the hash table table
	//uint8_t		maxHashElementId;			// The maximum ID allowed in the elements of an entry of the hash table
	
} FieldsLimits;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	uint16_t 		storageBitsMSABRows;		// Storage bits for encoding the number of rows
	uint16_t 		storageBitsMSABCols;		// Storage bits for encoding the number of columns
	uint16_t 		storageBitsHeaderLine;		// Storage bits for encoding the header line size
	uint16_t		storateBitsAbsScoreValue;	// Storage bits for encoding absolute score value
	uint16_t		storageBitsSrcName;			// Storage bits for encoding the source (specie.chr) string
	uint16_t		storageBitsSourceSize;		// Storage bits for encoding the source/chromossome size
	uint16_t		storageBitsStartPosition;	// Storage bits for encoding the start position
	uint16_t		storageBitsStartOffset;		// Storage bits for encoding the start offset
	
	uint16_t		storageBitsCountValue;		// Storage bits foe encoding the count value
			
	uint16_t		storageBitsHashKeyValue;	// Storage bits for the hash key value
	uint16_t		storageBitsHashElementId;	// Storage bits for the hash element ID
	
} StorageBitsInfo;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FILE      	*Fopen						(const char *, const char *);
void		Fclose						(FILE *);
void 		Fseeko						(FILE *, off_t, int32_t);
uint64_t 	Ftello						(FILE *);
size_t		Fwrite						(const void *, size_t, size_t, FILE *);
size_t		Fread						(void *, size_t, size_t, FILE *);
uint8_t		FileExists					(const char *);

void		Remove						(const char *);

size_t 		Strlen						(const char *);
char 		*Strcpy						(char *, const char *, size_t);
char 		*Strcat						(char *, const char *, size_t);
int32_t 	Strcmp						(const char *, const char *);
int64_t		Strtol						(const char *, int32_t);	
uint64_t	Strtoul						(const char *, int32_t);	

int32_t		Atoi						(const char *);
uint32_t	Atoui						(const char *);
uint64_t 	Atoul						(const char *);
	
uint64_t 	GetNumberOfBytesInFile		(const char *);
void 		PrintHumanReadableBytes		(uint64_t);
uint16_t	GetNumberOfBits				(uint64_t);
uint8_t		GetNumberOfDigits			(uint64_t);

uint8_t		StatusCharacter2Symbol		(UChar);
UChar		Symbol2StatusCharacter		(uint8_t);		

void		ShiftBuffer					(uint8_t *buf, uint8_t bufSize, uint8_t newSymbol);

void 		PrintParameterIntegerOption	(const char *, const char *, 
										const char *, uint32_t);
void		PrintParameterCharOption	(const char *, const char *, 
										const char *, char);
void		PrintParameterStringOption	(const char *, const char *, 
										const char *, const char *);
void		PrintStringOption			(const char *, const char *, 
										const char *);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
