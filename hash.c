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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "hash.h"
#include "mem.h"
#include "defs.h"
#include "common.h"
#include "element.h"

// http://www.azillionmonkeys.com/qed/hash.html
// http://stackoverflow.com/questions/11413860/best-string-hashing-function-for-short-filenames
// http://stackoverflow.com/questions/8317508/hash-function-for-a-string
// http://www.cse.yorku.ca/~oz/hash.html
// http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
// http://www.daniweb.com/software-development/cpp/threads/231987/string-hash-function

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Not a good hash function for the source names
/*
uint64_t HashFunctionV0(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 0x0;

	// Create the hash value
	for(i = 0; i < strSize; i++)
	{
		hashValue += str[i];
	}
	return hashValue;
}
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// url: http://stackoverflow.com/questions/8317508/hash-function-for-a-string
// Not a good hash function for the source names
/*
uint64_t HashFunctionV6(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 0;
	// Create the hash value
	for(i = 0; i != strSize; ++i)
	{
		hashValue += str[i] * (uint64_t)pow(31.0, (double)i);
	}
	return hashValue;
}
*/

// url: http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
// XOR hash
// Not a good hash function for the source names
/*
uint64_t HashFunctionV9(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 0;
	
	for(i = 0; i != strSize; ++i)
	{
		hashValue ^= str[i];
	}
	return hashValue;
}
*/
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// hashValue and FVNPrime too large for this context
/*
uint64_t HashFunctionV4(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 14695981039346656037;	// Using the 64 bits option
	uint64_t FVNPrime = 1099511628211;
		
	// Create the hash value
	for(i = 0; i != strSize; ++i)
	{
		hashValue = (hashValue * FVNPrime) ^ str[i];
	}
	return hashValue;
}
*/

uint64_t HashFunctionV0(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue=0x0;

	// Create the hash value
	for(i = 0; i != strSize; ++i)
	{
		//hashValue = (hashValue << 5) + str[i];
		hashValue = (hashValue << 5) | str[i];		// More fast
	}
	return hashValue;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// URL: http://stackoverflow.com/questions/98153/whats-the-best-hashing-algorithm-to-use-on-a-stl-string-when-using-hash-map
uint64_t HashFunctionV1(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue=0x0;

	// Create the hash value
	for(i = 0; i != strSize; ++i)
	{
		hashValue = (hashValue * 101) + str[i];
	}
	return hashValue;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// URL: http://programmers.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
// URL: http://stackoverflow.com/questions/2624192/good-hash-function-for-strings
// djb2 hash function
uint64_t HashFunctionV2(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 5381;
	
	// Create the hash value
	for(i = 0; i != strSize; ++i)
	{
		/* hash * 33 + c */
		hashValue = ( (hashValue << 5) + hashValue ) + str[i];
	}
	return hashValue;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// url: http://programmers.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed/145633#145633
// FNV-1a algorithm
//
// The FNV1 hash comes in variants that return 32, 64, 128, 256, 512 and 1024 
// bit hashes.
//
// The FNV-1a algorithm is:
//
// hash = FNV_offset_basis
// for each octetOfData to be hashed
//   hash = hash xor octetOfData
//   hash = hash * FNV_prime
// return hash
//
// Where the constants FNV_offset_basis and FNV_prime depend on the return hash size you want:
//
// Hash Size    Prime                       Offset
// ===========  =========================== =================================
// 32-bit       16777619                    2166136261
// 64-bit       1099511628211               14695981039346656037
// 128-bit      309485009821345068724781371 144066263297769815596495629667062367629
//

uint64_t HashFunctionV3(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 2166136261u;	// Using the 32 bits option
	uint64_t FVNPrime = 16777619;
		
	// Create the hash value
	for(i = 0; i != strSize; ++i)
	{
		hashValue = (hashValue * FVNPrime) ^ str[i];
	}
	return hashValue;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// url: http://www.cse.yorku.ca/~oz/hash.html
// sdbm
uint64_t HashFunctionV4(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 0;
	
	for(i = 0; i != strSize; ++i)
	{
		hashValue = str[i] + (hashValue << 6) + (hashValue << 16) - hashValue;
	}
	return hashValue;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// url: http://www.daniweb.com/software-development/cpp/threads/231987/string-hash-function
// SAX
// Shift-Add-XOR hash
uint64_t HashFunctionV5(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 0;
	
	for(i = 0; i != strSize; ++i)
	{
		hashValue ^= (hashValue << 5) + (hashValue >> 2) + str[i];
	}
	return hashValue;
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// url: http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
// Rotating hash
uint64_t HashFunctionV6(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 0;
	
	for(i = 0; i != strSize; ++i)
	{
		hashValue = (hashValue << 4) ^ (hashValue >> 28) ^ str[i];
	}
	return hashValue;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// url: http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
// Bernstein hash
uint64_t HashFunctionV7(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 0;
	
	for(i = 0; i != strSize; ++i)
	{
		hashValue = 33 * hashValue + str[i];
	}
	return hashValue;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// url: http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
// Modified Bernstein hash
uint64_t HashFunctionV8(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 0;
	
	for(i = 0; i != strSize; ++i)
	{
		hashValue = 33 * hashValue ^ str[i];
	}
	return hashValue;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// url: http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
// One-at-a-Time hash
uint64_t HashFunctionV9(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 0;
	
	for(i = 0; i != strSize; ++i)
	{
		hashValue += str[i];
		hashValue += (hashValue << 10);
		hashValue ^= (hashValue >> 6);
	}
	hashValue += (hashValue << 3);
	hashValue ^= (hashValue >> 11);
	hashValue += (hashValue << 15);
	
	return hashValue;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// url: http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
// ELF hash
uint64_t HashFunctionV10(const char *str)
{
	size_t i, strSize = Strlen(str);
	uint64_t hashValue = 0, g;

	for(i = 0; i != strSize; ++i)
	{
		hashValue = (hashValue << 4) + str[i];
		g = hashValue & 0xf0000000L;
		
		if(g != 0)
		{
			hashValue ^= g >> 24;
		}
		hashValue &= ~g;
	}
	return hashValue;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

HashTable *CreateHashTable(HashTableInfo *hashTableInfo)
{
	uint32_t size;
	HashTable *hashTable = (HashTable *)Calloc(1, sizeof(HashTable));
	
	// Get a prime number for the table size
	size = NextPrime(hashTableInfo->hashTableSize);
	
	// Create the pointers the "list" of entries
	hashTable->entries = (Entry **)Calloc(size, sizeof(Entry *));
	
	// To store the number of elements that each entry has
	hashTable->entriesSize = (uint8_t *)Calloc(size, sizeof(uint8_t));
	
	hashTable->hashTableSize = size;
	hashTable->maxNumberOfElements = hashTableInfo->maxNumberOfElements;
	hashTable->maxSrcNameSize = hashTableInfo->maxSrcNameSize;
	hashTable->hashFunction = hashTableInfo->hashFunction;
	hashTable->nUsedEntries = 0;
	hashTable->nElements = 0;
	
	return hashTable;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeHashTable(HashTable *hashTable)
{
	uint32_t r, c;
	
	for(r = 0; r != hashTable->hashTableSize; ++r)
	{
		for(c = 0; c != hashTable->entriesSize[r]; ++c)
		{
			FreeElement(hashTable->entries[r][c].element);
		}
		Free(hashTable->entries[r], hashTable->entriesSize[r]*sizeof(Entry));
	}
	
	Free(hashTable->entries, hashTable->hashTableSize*sizeof(Entry *));
	Free(hashTable->entriesSize, hashTable->hashTableSize*sizeof(uint8_t));
	Free(hashTable, sizeof(HashTable));
	hashTable = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetHashTable(HashTable *hashTable)
{
	uint32_t r, c;
	
	for(r = 0; r != hashTable->hashTableSize; ++r)
	{
		for(c = 0; c != hashTable->entriesSize[r]; ++c)
		{
			FreeElement(hashTable->entries[r][c].element);
		}
		if(hashTable->entriesSize[r] != 0)
		{
			Free(hashTable->entries[r], hashTable->entriesSize[r]*sizeof(Entry));
			//fprintf(1, "Row: %u | size: %u\n", r, hashTable->entriesSize[r]);
		}
		hashTable->entriesSize[r] = 0;
	}
	
	hashTable->nUsedEntries = 0;
	hashTable->nElements = 0;	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t GetHashValue(const char *str, HashTable *hashTable)
{
	uint8_t hashFunction = hashTable->hashFunction;
	size_t strSize = Strlen(str);
	
	if( (str == NULL) || (strSize == 0) )
	{
		fprintf(stderr, "Error (GetHashValue): can not obtain a hash key from an empty or NULL string.\n");
		exit(EXIT_FAILURE);
	}
	
	switch(hashFunction)
	{
		case 0: return HashFunctionV0(str);
		case 1: return HashFunctionV1(str);
		case 2: return HashFunctionV2(str);
		case 3: return HashFunctionV3(str);
		case 4: return HashFunctionV4(str);
		case 5: return HashFunctionV5(str);
		case 6: return HashFunctionV6(str);
		case 7: return HashFunctionV7(str);
		case 8: return HashFunctionV8(str);
		case 9: return HashFunctionV9(str);
		case 10: return HashFunctionV10(str);
		default: return HashFunctionV1(str);	// FIXME - Change the default Hash Function
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
uint32_t GetHashKey(const char *str, HashTable *hashTable)
{
	uint32_t hashTableSize = hashTable->hashTableSize;
	
	// Get the hash value
	uint64_t key = GetHashValue(str, hashTable);
	
	// The hash function
	key = (key % hashTableSize);
	if(key > UINT32_MAX)
	{
		fprintf(stderr,"Error(GetHashKey): overflow occurred.\n");
		fprintf(stderr,"Error(GetHashKey): maximum key allowed = %"PRIu32".\n", UINT32_MAX);
		fprintf(stderr,"Error(GetHashKey): key obtained %"PRIu64".\n", key);
		exit(EXIT_FAILURE);
	}
	return (uint32_t)(key);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Element *FindElementByString(const char *str, HashTable *hashTable, 
	HashPosition *hashPosition)
{
	// Get the hash key
	uint32_t n, k = GetHashKey(str, hashTable);

	// Loop all elements of the current entry
	for(n = 0; n != hashTable->entriesSize[k]; ++n)
	{
		if(Strcmp(str, hashTable->entries[k][n].element->sourceName) == 0)
		{
			hashPosition->hashKey = k;
			hashPosition->elementId = n;
			return hashTable->entries[k][n].element;
		}
	}
	
	// Because the encoder needs to send information about the element ID
	// The number of bits to encode and decode is constant so there is the 
	// reason for the limit
	if(hashTable->entriesSize[k] >= hashTable->maxNumberOfElements)
	{
		fprintf(stderr, "Error (FindElementByString): maximum number of elements for entry %"PRIu32" reached!\n", k);
		fprintf(stderr, "Error (FindElementByString): please increase the size of the hash table using the '-hts' parameter (option 1).\n");
		fprintf(stderr, "Error (FindElementByString): please increase the maximum number of entries allowed in each entry using the '-hme' parameter (option 2).\n");
		fprintf(stderr, "Error (FindElementByString): current number of elements in entry %"PRIu32" is %"PRIu8".\n", k, hashTable->entriesSize[k]);
		fprintf(stderr, "Error (FindElementByString): maximum number of elements allowed in this context is %"PRIu8".\n", hashTable->maxNumberOfElements);
		exit(EXIT_FAILURE);
	}
	
	// No elements in the current entry?
	if(hashTable->entriesSize[k] == 0)
		hashTable->nUsedEntries++;
		
	// If the function reaches this states it means that the string "str"
	// was not found. It is necessary to allocate memory for the new element
	hashTable->entries[k] = (Entry *)Realloc(hashTable->entries[k], 
		(hashTable->entriesSize[k] + 1) * sizeof(Entry) , sizeof(Entry));
	
	// Set an empty element to the new position 
	hashTable->entries[k][hashTable->entriesSize[k]].element = CreateEmptyElement(hashTable->maxSrcNameSize);
		
	// Increase the number of elements
	hashTable->nElements++;	
	
	// Store the position (hash key and element ID)
	hashPosition->hashKey = k;
	hashPosition->elementId = hashTable->entriesSize[k];
	
	// Increase the number of elements stored in the current entry
	hashTable->entriesSize[k]++;
	
	// Return the inserted element
	return hashTable->entries[k][hashTable->entriesSize[k]-1].element;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Element *GetElement(HashTable *hashTable, HashPosition *hashPosition)
{
	uint8_t elementId = hashPosition->elementId;
	uint32_t hashKey = hashPosition->hashKey;
	
	if(hashKey > hashTable->hashTableSize)
	{
		fprintf(stderr, "Error (GetElement): unable to access position %"PRIu32" of the hash table.\n", hashKey);
		fprintf(stderr, "Error (GetElement): hash table size is %"PRIu32".\n", hashTable->hashTableSize);
		exit(EXIT_FAILURE);
	}
	
	if(elementId > hashTable->entriesSize[hashKey])
	{
		fprintf(stderr, "Error (GetElement): element id %"PRIu8" out of range!\n", elementId);
		fprintf(stderr, "Error (GetElement): current number of element in entry %"PRIu32" = %"PRIu8".\n", hashKey, hashTable->entriesSize[hashKey]);
		fprintf(stderr, "Error (GetElement): element id should be <= %"PRIu8".\n", hashTable->entriesSize[hashKey]);
		exit(EXIT_FAILURE);
	}
	
	// The element was already decoded
	if(elementId < hashTable->entriesSize[hashKey])
	{
		return hashTable->entries[hashKey][elementId].element;
	}
	
	// The element was not decoded yet
	// Allocate memory for storing the new element
	hashTable->entries[hashKey] = (Entry *)Realloc(hashTable->entries[hashKey], 
		(hashTable->entriesSize[hashKey] + 1) * sizeof(Entry) , sizeof(Entry));

	// Set an empty element to the new position 
	hashTable->entries[hashKey][hashTable->entriesSize[hashKey]].element = CreateEmptyElement(hashTable->maxSrcNameSize);
		
	// Increase the number of elements
	hashTable->nElements++;	
	
	// Increase the number of elements stored in the current entry
	hashTable->entriesSize[hashKey]++;
	
	// Return the inserted element
	return hashTable->entries[hashKey][hashTable->entriesSize[hashKey]-1].element;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void HashingStats(HashTable *hashTable)
{
	uint8_t maxEntrySize = 0;
	uint32_t entry, emptyEntries=0;
	double deviation = 0, average = (double)hashTable->nElements / hashTable->hashTableSize;
	
		
	for(entry = 0; entry != hashTable->hashTableSize; ++entry)
	{
		deviation += fabs(average - hashTable->entriesSize[entry]);
		if(hashTable->entriesSize[entry] == 0) 
		{
			emptyEntries++;
			continue;
		}
		
		if(hashTable->entriesSize[entry] > maxEntrySize) maxEntrySize = hashTable->entriesSize[entry];
		
		
	}
	printf("------------------------------------\n");
	printf("| Hash status                      |\n");
	printf("------------------------------------\n");
	printf("Hash size .......... %"PRIu32"\n", hashTable->hashTableSize);
	printf("Used entries ....... %"PRIu32" (%.3lf %%) \n", hashTable->nUsedEntries, 
		100.0*((double)hashTable->nUsedEntries/hashTable->hashTableSize));
	printf("Ideal entry size ... %.3lf\n", average);
	printf("Deviation .......... %.3lf\n", deviation / hashTable->hashTableSize);
	printf("Max entry size ..... %"PRIu32"\n", maxEntrySize);
	printf("Used elements ...... %"PRIu64"\n",hashTable->nElements);
	printf("------------------------------------\n");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t IsPrimeV1(uint32_t number)
{
	uint32_t i;
	
	if(number <= 1) return 0x0;	// False
	for(i = 2; i != number; ++i)
	{
		if((number % i) == 0)
			return 0x0;		// False
	}
	return 0x1;				// True
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t IsPrimeV2(uint32_t number)
{
	uint32_t i, upperLimit = (uint32_t)sqrt(number * 1.0);
	
	if(number <= 1) return 0x0;	// False
	
	//for(i = 2; i <= upperLimit; i++)
	for(i = 2; i != (upperLimit+1); ++i)
	{
		if((number % i) == 0)
			return 0x0;		// False
	}
	return 0x1;				// True
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t IsPrimeV3(uint32_t number)
{
	uint32_t i, upperLimit = (uint32_t)sqrt(number * 1.0);
	
	if(number <= 1) return 0x0; 		// False
	if(number == 2) return 0x1; 		// True
	if((number % 2) == 0) return 0x0;	// False
		
	//for(i = 3; i <= upperLimit; i++)
	for(i = 3; i != (upperLimit+1); ++i)
	{
		if((number % i) == 0)
			return 0x0;		// False
	}
	return 0x1;				// True
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t NextPrime(uint32_t number)
{
	uint32_t n;
	// If is a prime number
	if(IsPrimeV3(number) == 0x1)
		return number;
	n = number+1;
	while(1)
	{
		if(IsPrimeV3(n) == 0x1)
			return n;
		if(n == UINT32_MAX)
		{
			fprintf(stderr, "Error (NextPrime): overflow occurred!\n");
			fprintf(stderr, "Error (NextPrime): there are no prime numbers between [%"PRIu32" - %"PRIu32"]!\n", number, UINT32_MAX);
			exit(EXIT_FAILURE);
		}
		n++;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t PreviousPrime(uint32_t number)
{
	uint32_t n;
	
	if(IsPrimeV3(number) == 0x1)
		return number;
	
	n = number-1;
	while(n > 1)
	{
		if(IsPrimeV3(n) == 0x1)
			return n;
		n--;
	}
	
	fprintf(stderr, "Error (PreviousPrime): there are no prime numbers after number %"PRIu32".\n", number);
	exit(EXIT_FAILURE);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
