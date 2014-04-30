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
//	Description: functions for handling memory								 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "mem.h"
#include "defs.h"
#include <stdlib.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static uint64_t totalMemory = 0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void *Malloc(size_t size)
{
	void *pointer = malloc(size);

	if(pointer == NULL)
    {
		fprintf(stderr, "Error allocating %"PRIu64" bytes.\n", (uint64_t)size);
		exit(EXIT_FAILURE);
	}

	totalMemory += size;
	return pointer;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void *Calloc(size_t nmemb, size_t size)
{
	void *pointer = calloc(nmemb, size);

	if(pointer == NULL)
    {
		fprintf(stderr, "Error allocating %"PRIu64" bytes.\n", (uint64_t)size);
		exit(EXIT_FAILURE);
    }
	
	totalMemory += nmemb * size;
	return pointer;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void *Realloc(void *ptr, size_t size, size_t additionalSize)
{
	void *pointer = realloc(ptr, size);
	
	if(pointer == NULL) 
    {
		fprintf(stderr, "Error allocating %"PRIu64" bytes.\n", (uint64_t)size);
		exit(EXIT_FAILURE);
	}
	totalMemory += additionalSize;
	return pointer;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Free(void *ptr, size_t size)
{
	totalMemory -= size;
	free(ptr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t TotalMemory()
{
	return totalMemory;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
