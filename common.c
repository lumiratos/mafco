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
//	Description: functions for handling with files							 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include "defs.h"
#include "mem.h"
#include "common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

FILE *Fopen(const char *path, const char *mode)
{
	FILE *file = fopen(path, mode);
	
	if(file == NULL)
	{
		fprintf(stderr, "Error (Fopen): unable to open file: '%s' (mode %s).\n", path, mode);
	    exit(EXIT_FAILURE);
	}
	return file;
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

void Fclose(FILE *stream)
{
	if(stream == NULL)
	{
		fprintf(stderr, "Error (Fclose): NULL file pointer.\n");
		exit(EXIT_FAILURE);
	}
	
	// Test if there is an error in the stream
	if(ferror(stream))
	{
		fprintf(stderr, "Error (Fclose): unable to close the stream due to an unknown error.\n");
		exit(EXIT_FAILURE);
	}
		
	if(fclose(stream) != 0)
	{
		fprintf(stderr, "Error (Fclose): unable to close the file stream.\n");
		exit(EXIT_FAILURE);	
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

void Fseeko(FILE *stream, off_t offset, int32_t origin)
{
	if(stream == NULL)
	{
		fprintf(stderr, "Error (Fseeko): NULL file pointer.\n");
		exit(EXIT_FAILURE);
	}
		
	if(fseeko(stream, offset, origin) != 0)
	{
		Fclose(stream);
		fprintf(stderr, "Error (Fseeko): unable to change file position.\n");
		fprintf(stderr, "Error (Fseeko): offset = %"PRId64".\n",offset);
		
		if(origin == SEEK_SET)
			fprintf(stderr, "Error (Fseeko): origin = %d (SEEK_SET).\n", origin);
		else if(origin == SEEK_CUR)
			fprintf(stderr, "Error (Fseeko): origin = %d (SEEK_CUR).\n", origin);
		else if(origin == SEEK_END)
			fprintf(stderr, "Error (Fseeko): origin = %d (SEEK_END).\n", origin);
		else
			fprintf(stderr, "Error (Fseeko): invalid origin value = %d.\n", origin);
		exit(EXIT_FAILURE);
	}			
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	
uint64_t Ftello(FILE *stream)
{
	if(stream == NULL)
	{
		fprintf(stderr, "Error (Ftello): NULL file pointer.\n");
		exit(EXIT_FAILURE);
	}
	
	if(ftello(stream) < 0)
	{
		Fclose(stream);
		fprintf(stderr, "Error (Ftello): unable get the current position of the file associated with stream.\n");
		exit(EXIT_FAILURE);	
	}
	
	return (uint64_t)ftello(stream);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

size_t Fwrite(const void *ptr, size_t size, size_t nmeb, FILE *stream)
{
	size_t elementsWritten;
		
	if(stream == NULL)
	{
		fprintf(stderr, "Error (Fwrite): NULL file pointer.\n");
		exit(EXIT_FAILURE);
	}
	
	if(ptr == NULL)
	{
		fprintf(stderr, "Error (Fwrite): NULL data pointer location input parameter (the first parameter)..\n");
		Fclose(stream);
		exit(EXIT_FAILURE);
	}
	
	if(size == 0)
	{
		fprintf(stderr, "Error (Fwrite): a non-zero size parameter is mandatory in this context.\n");
		Fclose(stream);
		exit(EXIT_FAILURE);
	}
	
	if(nmeb == 0)
	{
		fprintf(stderr, "Error (Fwrite): a non-zero number of elements parameter is mandatory in this context.\n");
		Fclose(stream);
		exit(EXIT_FAILURE);
	}
	
	elementsWritten = fwrite(ptr, size, nmeb, stream);
	// If the number of elements written is different from the input parameter 'nmeb'
	// it means that some error occurred
	if(elementsWritten != nmeb)
	{
		fprintf(stderr, "Error (Fwrite): unable to write to the output stream.\n");
		fprintf(stderr, "Error (Fwrite): hard disk run out of space (probably).\n");
		fprintf(stderr, "Error (Fwrite): number of items written                = %"PRIu64".\n", (uint64_t)elementsWritten);
		fprintf(stderr, "Error (Fwrite): number of items that should be written = %"PRIu64".\n", (uint64_t)nmeb);
		Fclose(stream);
		exit(EXIT_FAILURE);
	}	
	// No error occurred
	return elementsWritten;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

size_t Fread(void *ptr, size_t size, size_t nmeb, FILE *stream)
{
	size_t elementsRead;
	
	if(stream == NULL)
	{
		fprintf(stderr, "Error (Fread): NULL file pointer.\n");
		exit(EXIT_FAILURE);
	}
	
	if(ptr == NULL)
	{
		fprintf(stderr, "Error (Fread): NULL data pointer location input parameter (the first parameter).\n");
		Fclose(stream);
		exit(EXIT_FAILURE);
	}
	
	if(size == 0)
	{
		fprintf(stderr, "Error (Fread): a non-zero size parameter is mandatory in this context.\n");
		Fclose(stream);
		exit(EXIT_FAILURE);
	}
	
	if(nmeb == 0)
	{
		fprintf(stderr, "Error (Fread): a non-zero number of elements parameter is mandatory in this context.\n");
		Fclose(stream);
		exit(EXIT_FAILURE);
	}
	
	// The actual reading procedure
	elementsRead = fread(ptr, size, nmeb, stream);
	
	// Test if there is an error in the input stream
	if(ferror(stream))
	{
		fprintf(stderr, "Error (Fread): unable to read from the input file.\n");
		Fclose(stream);	
		exit(EXIT_FAILURE);
	}
	
	// If the number of elements read is diferent from 'nmeb' input parameter
	// and the fread did not reach the EOF it means that something went wrong
	if( (elementsRead != nmeb) && (!feof(stream)) )
	{
		fprintf(stderr, "Error (Fread): unable to read from the input stream.\n");
		fprintf(stderr, "Error (Fread): number of items read                = %"PRIu64".\n", (uint64_t)elementsRead);
		fprintf(stderr, "Error (Fread): number of items that should be read = %"PRIu64".\n", (uint64_t)nmeb);
		Fclose(stream);
		exit(EXIT_FAILURE);
	}
	
	// No error occurred
	return elementsRead;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

uint8_t FileExists(const char *path)
{
	FILE *file = fopen(path, "r");
	
	if(file == NULL)
	{
		fprintf(stderr, "Error (FileExists): unable to access file '%s' (read mode).\n", path);
		return 0x1;
	}
	// Close the stream
	Fclose(file);
	return 0x0;
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

void Remove(const char *path)
{
	if(remove(path) != 0)
	{
		fprintf(stderr, "Error (Remove): unable to remove file/directory '%s'.\n", path);
		exit(EXIT_FAILURE);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	
size_t Strlen(const char *s)
{
	if(s == NULL)
	{
		fprintf(stderr, "Error (Strlen): unable to get the string size of a NULL string.\n");
		exit(EXIT_FAILURE);
	}
	return strlen(s);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

char *Strcpy(char *dest, const char *src, size_t maxSizeAllowed)
{
	if(dest == NULL)
	{
		fprintf(stderr, "Error (Strcpy): NULL destination string.\n");
		exit(EXIT_FAILURE);
	}
	
	if(src == NULL)
	{
		fprintf(stderr, "Error (Strcpy): NULL source string.\n");
		exit(EXIT_FAILURE);
	}
	
	//if(Strlen(src) > Strlen(dest))
	if(Strlen(src) >= maxSizeAllowed)
	{
		fprintf(stderr, "Error (Strcpy): source string is too big to be copied to the destination string.\n");
		fprintf(stderr, "Error (Strcpy): source string has %"PRIu64" characters.\n", (uint64_t)Strlen(src));
		fprintf(stderr, "Error (Strcpy): destination string can only hold %"PRIu64" characters.\n", (uint64_t)maxSizeAllowed-1);
		fprintf(stderr, "Source string: '%s'\n", src);
		exit(EXIT_FAILURE);	
	}
		
	return strcpy(dest, src);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

char *Strcat(char *dest, const char *src, size_t maxSizeAllowed)
{
	if(dest == NULL)
	{
		fprintf(stderr, "Error (Strcat): NULL destination string.\n");
		exit(EXIT_FAILURE);
	}
	
	if(src == NULL)
	{
		fprintf(stderr, "Error (Strcat): NULL source string.\n");
		exit(EXIT_FAILURE);
	}
			
	if( (Strlen(dest) + Strlen(src)) >= maxSizeAllowed)
	{
		fprintf(stderr, "Error (Strcat): source string is too big to be appended to destination string.\n");
		fprintf(stderr, "Error (Strcat): source string has %"PRIu64" characters.\n", (uint64_t)Strlen(src));
		fprintf(stderr, "Error (Strcat): destination string has %"PRIu64" characters.\n", (uint64_t)Strlen(dest));
		fprintf(stderr, "Error (Strcat): destination string can only hold %"PRIu64" characters.\n", (uint64_t)maxSizeAllowed-1);
		fprintf(stderr, "Destination string: '%s'\n", dest);
		fprintf(stderr, "Source string:      '%s'\n", src);
		exit(EXIT_FAILURE);	
	}
	
	return strcat(dest, src);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	
int32_t	Strcmp(const char *s1, const char *s2)
{
	if(s1 == NULL)
	{
		fprintf(stderr, "Error (Strcmp): the first paramenter string is NULL.\n");
		exit(EXIT_FAILURE);
	}
	
	if(s2 == NULL)
	{
		fprintf(stderr, "Error (Strcmp): the second paramenter string is NULL.\n");
		exit(EXIT_FAILURE);
	}
	
	return (int32_t)strcmp(s1, s2);
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	
int64_t Strtol(const char* str, int32_t base)
{
	char *endPtr;
	long int lnum;
	
	errno = 0; // Global variable defined in <errno.h>
		
	if(str == NULL)
	{
		fprintf(stderr, "Error (Strtol): NULL input string.\n");
		exit(EXIT_FAILURE);
	}
	
	if(base < 0)
	{
		fprintf(stderr, "Error (Strtol): negative base specified.\n");
		fprintf(stderr, "Error (Strtol): input string '%s'.\n", str);
		exit(EXIT_FAILURE);
	}
			
	// Convert the string to a number
	lnum = strtol(str, &endPtr, base);
		
	// if no characters were converted these pointers are equal
	if (endPtr == str)     
	{
		fprintf(stderr, "Error (Strtol): cannot convert string '%s' to a number!\n", str);
		exit(EXIT_FAILURE);
	}
	
	// Overflow or underflow detected?
	if ((lnum == LONG_MAX || lnum == LONG_MIN) && ERANGE == errno)  
	{
		fprintf(stderr, "Error (Strtol): number is out of LONG INT range!\n");
		fprintf(stderr, "Error (Strtol): range allowed [ %ld - %ld ].\n", LONG_MIN, LONG_MAX);
		fprintf(stderr, "Error (Strtol): input string '%s'.\n", str);
		fprintf(stderr, "Use Strtoul function instead Strtol (in case of an overflow problem).\n");
		exit(EXIT_FAILURE);
	}
	
	return (int64_t)lnum;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

uint64_t Strtoul(const char *str, int32_t base)
{
	char *endPtr;
	unsigned long long int lnum;
	
	errno = 0; // Global variable defined in <errno.h>
		
	if(str == NULL)
	{
		fprintf(stderr, "Error (Strtoul): NULL input string.\n");
		exit(EXIT_FAILURE);
	}
	
	if(base < 0)
	{
		fprintf(stderr, "Error (Strtoul): negative base specified.\n");
		fprintf(stderr, "Error (Strtoul): input string '%s'.\n", str);
		exit(EXIT_FAILURE);
	}
	
	if(str[0] == '-')
	{
		fprintf(stderr, "Error (Strtoul): negative value are not allowed in this context.\n");
		fprintf(stderr, "Error (Strtoul): input string '%s'.\n", str);
		fprintf(stderr, "Please use Strtol() function to convert a string with a negative value.\n");
		exit(EXIT_FAILURE);
	}
			
	// Convert the string to a number
	lnum = strtoull(str, &endPtr, base);
	//lnum = strtoul(str, &endPtr, base);
		
	// If no characters were converted these pointers are equal
	if (endPtr == str)     
	{
		fprintf(stderr, "Error (Strtoul): cannot convert string '%s' to a number!\n", str);
		exit(EXIT_FAILURE);
	}
	
	// Overflow occurred?
	//if ( (lnum == ULLONG_MAX) && (ERANGE == errno) )  
	if ( (lnum == UINT64_MAX) && (ERANGE == errno) )  
	{
		fprintf(stderr, "Error (Strtoul): number is out of UNSIGNED LONG LONG INT range!\n");
		//fprintf(stderr, "Error (Strtoul): range allowed [ 0 - %llu ].\n", ULLONG_MAX);
		fprintf(stderr, "Error (Strtoul): range allowed [ 0 - %"PRIu64" ].\n", UINT64_MAX);
		fprintf(stderr, "Error (Strtoul): input string '%s'.\n", str);
		exit(EXIT_FAILURE);
	}	
	return (uint64_t)lnum;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

int32_t Atoi(const char* str)
{
	int64_t num;
		
	if(str == NULL)
	{
		fprintf(stderr, "Error (Atoi): NULL input string.\n");
		exit(EXIT_FAILURE);
	}
			
	num = Strtol(str, 10);
		
	if(num > INT32_MAX)
	{
		fprintf(stderr, "Error (Atoi): number is greater that INT32_MAX (%"PRId32").\n", INT32_MAX);
		fprintf(stderr, "Error (Atoi): input string to convert '%s'.\n", str);
		fprintf(stderr, "Error (Atoi): integer value obtained %"PRId64".\n", num);
		exit(EXIT_FAILURE);
	}
			
	if(num < INT32_MIN)
	{
		fprintf(stderr, "Error (Atoi): number is greater that INT32_MIN (%"PRId32").\n", INT32_MIN);
		fprintf(stderr, "Error (Atoi): input string to convert '%s'.\n", str);
		fprintf(stderr, "Error (Atoi): integer value obtained %"PRId64".\n", num);
		exit(EXIT_FAILURE);
	}
			
	return (int32_t)num;
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	
uint32_t Atoui(const char *str)
{
	uint64_t num;
	
	if(str == NULL)
	{
		fprintf(stderr, "Error (Atoui): NULL input string.\n");
		exit(EXIT_FAILURE);
	}
		
	//num = Strtol(str, 10);
	num = Strtoul(str, 10);
		
	// Overflow dected	
	if(num > UINT32_MAX)
	{
		fprintf(stderr, "Error (Atoui): number is greater that UINT32_MAX (%"PRIu32").\n", UINT32_MAX);
		fprintf(stderr, "Error (Atoui): input string to convert '%s'.\n", str);
		fprintf(stderr, "Error (Atoui): integer value obtained %"PRIu64".\n", num);
		fprintf(stderr, "Please use Atoul() function to convert large numbers.\n");
		exit(EXIT_FAILURE);
	}
			
	return (uint32_t)num;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	

uint64_t Atoul(const char *str)
{
	if(str == NULL)
	{
		fprintf(stderr, "Error (Atoul): NULL input string.\n");
		exit(EXIT_FAILURE);
	}
	
	return Strtoul(str, 10);	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
				
uint64_t GetNumberOfBytesInFile(const char *path)
{
	uint64_t size = 0;
	FILE *file = Fopen(path, "r");
	
	Fseeko(file, 0, SEEK_END);
	size = Ftello(file);
	Fclose(file);
	return size;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	
	
void PrintHumanReadableBytes(uint64_t bytes)
{
	uint32_t unit = 1024, expr;
	double size;
	char str[]={'K','M','G','T','E','Z','Y'};		
	if (bytes < unit) 
		printf("%"PRIu64" Bytes",bytes);
	else
	{
	    expr = log(bytes)/log(unit);
	    size = (double)bytes/pow((double)unit, (double)expr);
	    printf("%.1lf %cB", size, str[expr-1]);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
uint16_t GetNumberOfBits(uint64_t n)	
{
	uint64_t nBits;
	
	// If the input value is zero, return 1 bit
	if(n == 0) return 0x1;
	
	nBits = ceil(log((double)n)/log(2.0));
		
	if(nBits > UINT16_MAX)
	{
		fprintf(stderr, "Error (GetNumberOfBits): overflow detect.\n");
		fprintf(stderr, "Error (GetNumberOfBits): try to store the value %"PRIu64" in an unsigned 16 bits variable.\n", nBits);
		fprintf(stderr, "Error (GetNumberOfBits): maximum value allowed in an unsigned 16 bits variable is %"PRIu16".\n", UINT16_MAX);
		exit(EXIT_FAILURE);
	}
	else
		return (uint16_t)nBits;
}	
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t GetNumberOfDigits(uint64_t number)
{
	uint8_t digits = 0;
	if(number < 10) return 1;
	while(number)
	{	
		number /= 10;
		digits++;
	}
	return digits;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t StatusCharacter2Symbol(UChar c)
{
	switch(c)
	{
		case 'C':	return 0;
		case 'I':	return 1;
		case 'N':	return 2;
		case 'n':	return 3;
		case 'M':	return 4;		
		case 'T':	return 5;
		default: 
			fprintf(stderr,"Error (StatusCharacter2Symbol): Unexpected status character: '%c' (ASCII: %"PRIu8")\n", c, c);
			exit(EXIT_FAILURE);
	}
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
UChar Symbol2StatusCharacter(uint8_t s)
{
	switch(s)
	{
		case 0: return 'C';
		case 1: return 'I';
		case 2: return 'N';
		case 3: return 'n';
		case 4: return 'M';
		case 5: return 'T';
		default:  
			fprintf(stderr, "Error (Symbol2StatusCharacter): Unexpected status symbol: %"PRIu8"\n", s);
			exit(EXIT_FAILURE);
	}		
}
			
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ShiftBuffer(uint8_t *buf, uint8_t bufSize, uint8_t newSymbol)
{
	uint8_t n;
	for (n = 1; n != bufSize; ++n)
		buf[n-1] = buf[n];
	buf[bufSize - 1] = newSymbol;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintParameterIntegerOption(const char *arg, const char *variable, 
	const char *description, uint32_t defaultValue)
{
	fprintf(stderr, "\t%-4s %-32s %-75s [default: %10"PRIu32"]\n", arg, 
		variable, description, defaultValue);
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintParameterCharOption(const char *arg, const char *variable, 
	const char *description, char defaultValue)
{
			fprintf(stderr, "\t%-4s %-32s %-75s [default: %10c]\n", arg, 
				variable, description, defaultValue);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintParameterStringOption(const char *arg, const char *variable, 
	const char *description, const char *defaultValue)
{
			fprintf(stderr, "\t%-4s %-32s %-75s [default: %10s]\n", arg, 
				variable, description, defaultValue);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintStringOption(const char *arg, const char *variable, 
	const char *description)
{
			fprintf(stderr, "\t%-4s %-32s %-75s\n", arg, 
				variable, description);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
