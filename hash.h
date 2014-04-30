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
//	Description: functions for the hash table structure.			 		 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "defs.h"
#include "element.h"
#include "common.h"
//#include <sys/types.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	uint32_t		hashTableSize;			// Hash table size
	uint8_t			hashFunction;			// Hash function to use
	uint8_t			maxNumberOfElements;	// Maximum number of elements 
											// in each entry
	uint8_t			maxSrcNameSize;			// Maximum characters allowed in the 
											// source name
} HashTableInfo;


typedef struct
{
	Element *element;
} Entry;

typedef struct
{
	uint32_t		hashTableSize; 			// Hash table size
	uint8_t			maxNumberOfElements;	// Maximum number of elements allowed in each entry
	uint32_t		nUsedEntries;			// Total number of used entries
	uint64_t		nElements;				// Total number of elements in the Hash Table
	uint8_t			maxSrcNameSize;			// Maximum characters allowed in the source
	uint8_t			hashFunction;			// Hash function to use
	
	uint8_t			*entriesSize;			// Number of elements in the current entry
	Entry			**entries;				// The heads of the hash table list
} HashTable;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
HashTable 	*CreateHashTable		(HashTableInfo *);
void 		FreeHashTable			(HashTable *);
void 		ResetHashTable			(HashTable *);

uint64_t	GetHashValue			(const char *, HashTable *);
uint32_t 	GetHashKey				(const char *, HashTable *);
Element		*FindElementByString	(const char *, HashTable *, HashPosition *); 
Element 	*GetElement				(HashTable *, HashPosition *);
void		HashingStats			(HashTable *);

uint8_t 	IsPrimeV1				(uint32_t);
uint8_t 	IsPrimeV2				(uint32_t);
uint8_t 	IsPrimeV3				(uint32_t);
uint32_t 	NextPrime				(uint32_t);
uint32_t 	PreviousPrime			(uint32_t);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif