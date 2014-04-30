// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - --
//      Copyright 2014 IEETA/DETI - University of Aveiro, Portugal.                     -
//      All Rights Reserved.                                                            -
//                                                                                      -
//      These programs are supplied free of charge for research purposes only,          -
//      and may not be sold or incorporated into any commercial product. There          -
//      is ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they            -
//      are fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you           -
//      do happen to find a bug, or have modifications to suggest, please report        -
//      the same to Luis M. O. Matos, luismatos@ua.pt. The copyright notice             -
//      above and this statement of conditions must remain an integral part of          -
//      each and every copy made of these files.                                        -
//                                                                                      -
//      Description: compressor for MAF files                                           -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - --



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef THREADS
#include <pthread.h>
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "defs.h"
#include "mem.h"
#include "common.h"
#include "models.h"
#include "msab.h"
#include "context.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	uint8_t 		threadNumber;					// Thread ID
	uint8_t 		nThreads;						// Total number of threads	

	uint8_t			numberOfHeaderLines;			// Number of header lines in the MAF file (lines that start with '##')
	uint32_t		maxHeaderLineSize;				// Maximum number of characters for header lines allowed
	uint32_t		maxSrcNameSize;					// Maximum number of character in specie.chr source
	
	uint32_t		nGOBs;							// Number of GOBs for the current thread
	uint8_t			lastGOB;						// Verifies if the current thread has (0x1) or has not (0x0)
													// the last MSAB
	uint8_t			headerGOB;						// Verifies if this GOB has header information to decode
	
	
	UChar			template;						// Template used for compressing the 's' lines
	char 			inputFile[FILENAME_MAX];		// Input MAF file
	char 			tmpOutDecFile[FILENAME_MAX];	// Temporary file to put the decoded files
	
	off_t			*startOffset;					// Start position where the current thread should start decoding
	//off_t			endOffset;						// End position where the current thread should terminate the
													// decoding process
	
	ModelsOrder		*modelsOrder;					// Hold the information regarding the models size to encode
													// the information of each MSAB
	StorageBitsInfo	*storageBitsInfo;				// Holds the storage bits information (number of bits to use
													// in the writeNBits function)
	HashTableInfo	*hashTableInfo;					// Information to create the hash table (size, hash function, etc.)
}
ThreadsData;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void 	DecodeMAFPart		(void *);
void 	DecodeSLinesData	(MSAB *, ACModels *, ACDecoder *, CModels *, 
							CTemplate *, StorageBitsInfo *);
void 	DecodeQLinesData	(MSAB *, ACModels *, ACDecoder *, CModels *, 
							ModelsOrder *);
void 	DecodeILinesData	(MSAB *, ACModels *, ACDecoder *, CModels *, 
							StorageBitsInfo *);
void 	DecodeELinesData	(MSAB *, ACModels *, ACDecoder *, CModels *, 
							StorageBitsInfo *);

void 	WriteMSABToFile		(MSAB *, FILE *);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
int main(int argc, char *argv[])
{
	char outFileName[FILENAME_MAX] = "decodedFile.maf";
	char tmpOutDir[FILENAME_MAX] = "", *buffer;
	UChar template;	
	uint8_t help=0x0, nThreads=DEFAULT_NUMBER_OF_THREADS; 
	uint8_t numberOfHeaderLines; //removeTmpFiles = 0x1;
	uint8_t hashFunction, maxNumberOfElements;
	
	uint8_t sLinesModelOrder;				// Model order to decode the 's' lines (DNA bases and gaps)
	uint8_t caseModelOrder;					// Model order to decode the case information (lower or upper symbols)
	uint8_t caseFlagModelOrder;				// Model order to decode the case flag information
	uint8_t xchrModelOrder;					// Model order to decode the extra symbols (N's n's)
	uint8_t xchrFlagModelOrder;				// Model order to decode the extra symbols flag
	uint8_t strandModelOrder;				// Model order to decode the strand informations	
	uint8_t startOffsetSignModelOrder;		// Model order to decode the start offset sign 
	
	uint8_t qLinesModelOrder;				// Model order to decode the 'q' lines (quality values)
	uint8_t qLineFlagModelOrder;			// Model order to decode the 'q' lines presence in the MSAB
	uint8_t qLineInFlagModelOrder;			// Model order to decode the 'q' lines position in the MSAB
	
	uint8_t iLineFlagModelOrder;			// Model order to decode the 'i' lines presence in the MSAB
	uint8_t statusModelOrder;				// Model order to decode the status symbol of 'i' and 'e' lines
	uint8_t irregularStatusModelOrder;		// Model order to decode irregular status flag of 'i' lines
	uint8_t irregularCountModelOrder;		// Model order to decode irregular count flag of 'i' lines
	
	uint8_t eLineFlagModelOrder;			// Model order to decode the 'e' lines presence in the MSAB
	uint8_t eLineIrregularStatusModelOrder;	// Model order to decode irregular status flag of 'e' lines
			
	uint16_t storageBitsMSABRows;			// Storage bits for decoding the number of rows
	uint16_t storageBitsMSABCols;			// Storage bits for decoding the number of columns
	uint16_t storageBitsHeaderLine;			// Storage bits for decoding the header line size
	uint16_t storageBitsSrcName;			// Storage bits for decoding the source (specie.chr) string
	uint16_t storateBitsAbsScoreValue;		// Storage bits for decoding the absolute score value	
	uint16_t storageBitsSourceSize;			// Storage bits for decoding the source size
	uint16_t storageBitsStartPosition;		// Storage bits for decoding the start position
	uint16_t storageBitsStartOffset;		// Storage bits for decoding the start offset
	uint16_t storageBitsCountValue;			// Storage bits for decoding the count value of the 'i' lines
	
	uint32_t n, i;	
	uint32_t maxHeaderLineSize, maxSrcNameSize, bufferSize=BUF_SIZE;
	//uint32_t hashTableSize, nParts, partsToDecode[2] = {0, UINT32_MAX};
	//uint32_t nPartsToDecode, nPartsPerThread=0, remainingParts=0, partId;
	uint32_t hashTableSize, nGOBs, GOBsToDecode[2] = {0, UINT32_MAX};
	uint32_t nGOBsToDecode, nGOBsPerThread=0, remainingGOBs=0, GOBId;
	uint64_t nTotalBytes, *sizeOfBinaryFiles, *sizesCumSum, encFileHeaderSize;
	
	//FILE *inFp, *outFp;
	FILE *inFp;
	pthread_t *threads = NULL;
	ThreadsData *threadsData = NULL;
	
	// NULL DEVICE MODE NOT ACTIVATED
	#ifndef NULL_DEV
		char tmpOutFileName[FILENAME_MAX]="", tmp[FILENAME_MAX]="";	
		uint8_t mergeMAF = 0x1, removeTmpFiles = 0x1; 
		FILE *outFp;
		size_t tmpSize;
		// Get process ID
		pid_t pid = getpid();
	#endif
		
	for(n = 1; n != argc; n++)
	{
		if(!(strcmp("-h", argv[n])))
		{ 
			help = 0x1; 
			break; 
		}
	}
	
	if(argc < 2 || help == 1)
	{
		fprintf(stderr, "Usage: %s [OPTIONS] ... [COMPRESSED MAF FILE]\n\n", argv[0]);
		PrintStringOption("-h", "", "(print this help)");
		PrintStringOption("-v", "", "(print version number)");
		
		PrintParameterStringOption("-o", "<outFileName>", "(output MAF file name)", outFileName);
		PrintStringOption("-O", "<temporary output dir>", "(temporary directory to store the files of each thread)");
		PrintParameterIntegerOption("-nt", "<nThreads>", "(number of threads)", nThreads);
		//PrintParameterStringOption("-p", "<Pi:Pj> or <Pi>", "(parts to decode from the encoded file)", "All parts");
		PrintParameterStringOption("-ng", "<Gi:Gj> or <Gi>", "(GOBs range to decode from the encoded file)", "All Blocks");
		
		PrintParameterIntegerOption("-bs", "<bufferSize>", "(buffer size used in the merge procedure)", bufferSize);
		PrintParameterStringOption("-rt", "<yes/no>", "(remove temporary files)", "yes");
		PrintParameterStringOption("-m", "<yes/no>", "(merge decoded files in a single MAF file)", "yes");
		fprintf(stderr, "\n");
		return EXIT_FAILURE;
	}

	
	// Print version and copyrigth notice
        for(n = 1; n != argc; ++n)
        {
                if(!(Strcmp("-v", argv[n])))
                {
                        printf(DEC_VERSION_NUMBER);
                        printf(COPYRIGHT_NOTICE);
                        return EXIT_SUCCESS;
                }
        }
	
	// Output file
	for(n = 1; n < argc-1; ++n)
	{
		if(!(strcmp("-o", argv[n])))
		{
			Strcpy(outFileName, argv[n+1], FILENAME_MAX);
			break;
		} 
	}
	
	// Output temporary directory
	for(n = 1; n < argc-1; ++n)
	{
		if(!(strcmp("-O", argv[n])))
		{
			//Strcpy(outTmpDir, argv[n+1], FILENAME_MAX);
			//break;	
			// Windows operating system	
			#if defined(OS_WINDOWS) || defined(WIN32) || defined(MSDOS)	
				if(argv[n+1][Strlen(argv[n+1])-1] == '\\')
				{
					// Copy string
					Strcpy(tmpOutDir, argv[n+1], FILENAME_MAX);
				}
				else
				{
					// Test the path size
					if(Strlen(argv[n+1]) >= FILENAME_MAX-1)
					{
						fprintf(stderr, "Error (%s): temporary path is too long", argv[0]);
						fprintf(stderr, "Error (%s): input temporary path is: '%s'.", argv[0], argv[n+1]);
						return EXIT_FAILURE;
					}
					// Add a '\' to the temporary path
					else
						sprintf(tmpOutDir, "%s\\", argv[n+1]);
				}
				// Other operating system Linux, Mac OS X 
			#else
				if(argv[n+1][Strlen(argv[n+1])-1] == '/')
				{
					// Copy string
					Strcpy(tmpOutDir, argv[n+1], FILENAME_MAX);
				}
				else
				{
					// Test the path size
					if(Strlen(argv[n+1]) >= FILENAME_MAX-1)
					{
						fprintf(stderr, "Error (%s): temporary path is too long", argv[0]);
						fprintf(stderr, "Error (%s): input temporary path is: '%s'.", argv[0], argv[n+1]);
						return EXIT_FAILURE;
					}
					// Add a '/' to the temporary path
					else
						sprintf(tmpOutDir, "%s/", argv[n+1]);
				}
			#endif
			break;
		} 
	}
	
	// Read the number of threads to be used
	for(n = 1; n != argc; ++n)
	{
		if(!(Strcmp("-nt", argv[n])))
		{
			nThreads = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_NUMBER_OF_THREADS)) ? 
				DEFAULT_NUMBER_OF_THREADS : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	//printf("%"PRIu32" - %"PRIu32"\n", partsToDecode[0], partsToDecode[1]);
	// Parts to decode
	for(n = 1; n != argc; ++n)
	{
		if(!(Strcmp("-ng", argv[n])))
		{
			if(sscanf(argv[n+1], "%"SCNu32":%"SCNu32"", &GOBsToDecode[0], &GOBsToDecode[1]) == 2)
			{
				GOBsToDecode[0] = ( (GOBsToDecode[0] <= 0) ? 1 : GOBsToDecode[0]);
				GOBsToDecode[0]--;
				GOBsToDecode[1]--;
				break;
			}
			
			if(sscanf(argv[n+1], "%"SCNu32"", &GOBsToDecode[0]) == 1)
			{
				GOBsToDecode[0] = ( (GOBsToDecode[0] <= 0) ? 1 : GOBsToDecode[0]);
				GOBsToDecode[1] = --GOBsToDecode[0];
				break;
			}
		}
	}
	//printf("%"PRIu32" - %"PRIu32"\n", partsToDecode[0], partsToDecode[1]);

	// Set the buffer size used in the merge procedure
	// Merge the decoded files into a single file
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-bs", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				bufferSize = Atoui(argv[n+1]);
			break; 
		}
	}
	
	// NULL DEVICE MODE NOT ACTIVATED
	#ifndef NULL_DEV
		// Remove temporary files flag
		for(n = 1; n < argc-1; ++n)
		{
			if(Strcmp("-rt", argv[n]) == 0)
			{
				if( (Strcmp(argv[n+1], "no") == 0) || (Strcmp(argv[n+1], "NO") == 0) || (Strcmp(argv[n+1], "No") == 0))
					removeTmpFiles = 0x0;	// False - will not remove temporary files
				if( (argv[n+1][0] == 'n') || (argv[n+1][0] == 'N') )
					removeTmpFiles = 0x0;	// False - will not remove temporary files
				break; 
			}
		}
		
		// Merge flag
		for(n = 1; n < argc-1; ++n)
		{
			if(Strcmp("-m", argv[n]) == 0)
			{
				if( (Strcmp(argv[n+1], "no") == 0) || (Strcmp(argv[n+1], "NO") == 0) || (Strcmp(argv[n+1], "No") == 0))
					mergeMAF = 0x0;	// False - will not merge the decoded files into a single MAF file
				if( (argv[n+1][0] == 'n') || (argv[n+1][0] == 'N') )
					mergeMAF = 0x0;	// False - will not merge the decoded files into a single MAF file
				break; 
			}
		}
	#endif
	
	
	
	
				
	// Get the number of bytes of the 
	nTotalBytes = GetNumberOfBytesInFile(argv[argc-1]);
	printf("Input encoded file '%s' has %"PRIu64" bytes.\n", argv[argc-1], nTotalBytes);
	
	// Open the file for reading the header information
	inFp = Fopen(argv[argc-1], "rb");
		
	// Read the parameters to start the decoding process
	//Fread(&nThreads, sizeof(uint8_t), 1, inFp);
	Fread(&nGOBs, sizeof(uint32_t), 1, inFp);
	Fread(&template, sizeof(uint8_t), 1, inFp);
	Fread(&numberOfHeaderLines, sizeof(uint8_t), 1, inFp);
	Fread(&maxHeaderLineSize, sizeof(uint32_t), 1, inFp);
	Fread(&maxSrcNameSize, sizeof(uint32_t), 1, inFp);
	
	Fread(&sLinesModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&caseModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&caseFlagModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&xchrModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&xchrFlagModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&strandModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&startOffsetSignModelOrder, sizeof(uint8_t), 1, inFp);
	
	Fread(&qLinesModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&qLineFlagModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&qLineInFlagModelOrder, sizeof(uint8_t), 1, inFp);
	
	Fread(&iLineFlagModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&statusModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&irregularStatusModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&irregularCountModelOrder, sizeof(uint8_t), 1, inFp);
	
	Fread(&eLineFlagModelOrder, sizeof(uint8_t), 1, inFp);
	Fread(&eLineIrregularStatusModelOrder, sizeof(uint8_t), 1, inFp);

	
	
	// Read the hash information 
	Fread(&hashTableSize, sizeof(uint32_t), 1, inFp);
	Fread(&hashFunction, sizeof(uint8_t), 1, inFp);
	Fread(&maxNumberOfElements, sizeof(uint8_t), 1, inFp);
	
	// Read the storage bits information
	Fread(&storageBitsMSABRows, sizeof(uint16_t), 1, inFp);
	Fread(&storageBitsMSABCols, sizeof(uint16_t), 1, inFp);
	Fread(&storateBitsAbsScoreValue, sizeof(uint16_t), 1, inFp);
	Fread(&storageBitsHeaderLine, sizeof(uint16_t), 1, inFp);
	Fread(&storageBitsSrcName, sizeof(uint16_t), 1, inFp);
	Fread(&storageBitsSourceSize, sizeof(uint16_t), 1, inFp);
	Fread(&storageBitsStartPosition, sizeof(uint16_t), 1, inFp);
	Fread(&storageBitsStartOffset, sizeof(uint16_t), 1, inFp);
	Fread(&storageBitsCountValue, sizeof(uint16_t), 1, inFp);
	
	
	// Compute the parts to decode
	if(GOBsToDecode[1] == UINT32_MAX) // Decode all parts
	{
		GOBsToDecode[0] = 0;
		GOBsToDecode[1] = nGOBs - 1;
	}
	else
	{
		if(GOBsToDecode[1] > nGOBs-1) 
			GOBsToDecode[1] = nGOBs-1;
		if(GOBsToDecode[0] > GOBsToDecode[1])
			GOBsToDecode[0] = GOBsToDecode[1];    
	}
	
	// Compute the number of parts to decode
	nGOBsToDecode = GOBsToDecode[1] - GOBsToDecode[0] + 1; 
	
	// First part ID
	GOBId = GOBsToDecode[0];
	
	// Allocate memory for storing the size of each binary file
	// produced by each thread in the encoder
	//sizeOfBinaryFiles = (uint64_t *)Calloc(nThreads, sizeof(uint64_t));
	sizeOfBinaryFiles = (uint64_t *)Calloc(nGOBs, sizeof(uint64_t));
	sizesCumSum = (uint64_t *)Calloc(nGOBs, sizeof(uint64_t));
	
	// Used in the merge process
	buffer = (char *)Calloc(bufferSize, sizeof(char));
		
	// Read the size of each encoded binary file
	// this values will be used latter in the Fseeko function
	//for(n = 0; n != nThreads; ++n)
	for(n = 0; n != nGOBs; ++n)
	{
		Fread(&sizeOfBinaryFiles[n], sizeof(uint64_t), 1, inFp);
		//if(n == 0) sizesCumSum[n] = sizeOfBinaryFiles[n];
		//else sizesCumSum[n] = sizesCumSum[n-1] + sizeOfBinaryFiles[n]
		//printf("sizeOfBinaryFiles[%"PRIu8"] = %"PRIu64" bytes.\n", n, sizeOfBinaryFiles[n]);
		printf("GOBs %"PRIu32" has %"PRIu64" bytes.\n", n, sizeOfBinaryFiles[n]);
	}
	
	printf("\n");
	
	// After the previous loop, the file pointer is pointed to the 
	// beggining of the first encoded file. We need to save the 
	// the file pointer position in order to be used latter in the fseeko function.
	encFileHeaderSize = Ftello(inFp);
	
	// Close the file input file
	Fclose(inFp);
	
	for(n = 0; n != nGOBs; n++)
	{
		if(n == 0) sizesCumSum[n] = encFileHeaderSize;
		else sizesCumSum[n] = sizesCumSum[n-1] + sizeOfBinaryFiles[n-1];
		printf("GOB %"PRIu32" start offset at pos %"PRIu64" bytes.\n", n, sizesCumSum[n]);
	}
	
	// Verify of the number of threads according to the number of parts to decode
	//if(nThreads > nParts)
	if(nThreads > nGOBsToDecode)	
	{
		printf("Number of threads was reduced from %"PRIu32" to %"PRIu32"\n", nThreads, nGOBsToDecode);
		printf("The optimal distribution is one thread for each one of the %"PRIu32" parts.\n", nGOBsToDecode);
		nThreads = nGOBsToDecode;
	}
	
	// This is an integer division division
	nGOBsPerThread = nGOBsToDecode/nThreads;
	// Compute the remaing part to ditribute all over the other threads
	remainingGOBs=nGOBsToDecode-(nGOBsPerThread*nThreads);
	
	//printf("nPartsPerThread = %"PRIu32" | remainingParts = %"PRIu32"\n", nPartsPerThread, remainingParts);
	
	// Allocate memory for all thread and for the ThreadData
	threads = (pthread_t *)Calloc(nThreads, sizeof(pthread_t));
	threadsData = (ThreadsData *)Calloc(nThreads, sizeof(ThreadsData));
	
	for(n = 0; n != nThreads; ++n)
	{
		threadsData[n].threadNumber = n;
		threadsData[n].nThreads = nThreads;
		
		threadsData[n].nGOBs = nGOBsPerThread + (n < remainingGOBs ? (1) : (0));
		//threadsData[n].totalNumberOfParts = nParts;
		
		threadsData[n].numberOfHeaderLines = numberOfHeaderLines;
		threadsData[n].template = template;
		threadsData[n].maxHeaderLineSize = maxHeaderLineSize;
		threadsData[n].maxSrcNameSize = maxSrcNameSize;
			
		// Inut file name
		Strcpy(threadsData[n].inputFile, argv[argc-1], FILENAME_MAX);
		
		// NULL DEVICE MODE ACTIVATED
		#ifdef NULL_DEV
			Strcpy(threadsData[n].tmpOutDecFile, "/dev/null", FILENAME_MAX);
		#else
			// Output file name for each thread
			//sprintf(tmpOutFileName, "%s%02"PRIu8".maf", DEFAULT_TMP_DEC_FILE, n);
			tmpSize=snprintf(tmpOutFileName, FILENAME_MAX, "%sPID_%010"PRId32"-T_%02"PRIu8".maf", DEFAULT_TMP_DEC_FILE, (int32_t)pid, n);			
			if(tmpSize >= FILENAME_MAX)
			{
				fprintf(stderr, "Error (main): error when trying to write formatted output to sized buffer.\n");
				fprintf(stderr, "Error (main): sprintf function tried to wrote %"PRIu64" characters + '\\0'.\n", (uint64_t)tmpSize);
				fprintf(stderr, "Error (main): buffer max size = %"PRIu32".\n", (uint32_t)FILENAME_MAX);
				return EXIT_FAILURE;
			}
			
			// Initialize temporary string with an emtpy string
			Strcpy(tmp, "", 1); 
			// Append path to empty string
			Strcat(tmp, tmpOutDir, FILENAME_MAX); 
			// Append file name
			Strcat(tmp, tmpOutFileName, FILENAME_MAX); 
			// Copy full path to be used in each thread
			Strcpy(threadsData[n].tmpOutDecFile, tmp, FILENAME_MAX);
		#endif
			
		
		// Allocate memory for the start offset
		threadsData[n].startOffset = (off_t *)Calloc(threadsData[n].nGOBs, sizeof(off_t));
		
		// Set the start offsets to use in the fseek function
		for(i = 0; i != threadsData[n].nGOBs; ++i)
		{
			// Compute the start offset to be used latter
			threadsData[n].startOffset[i] = sizesCumSum[GOBId++];
		}
		
		// Is it the last thread?
		if(n == nThreads-1)
		{
			// Will the current thread decode the last part
			if(threadsData[n].startOffset[threadsData[n].nGOBs-1] == sizesCumSum[nGOBs-1])
				threadsData[n].lastGOB = 0x1;
		}
		// For all the other threads the lastPart flag is always false (0x0)
		else
			threadsData[n].lastGOB = 0x0;
		
		// For the first thread, verify if the MAF header info is to decode
		// or not
		if(n == 0)
		{
			// The thread 0 will decode the header information?
			if(threadsData[n].startOffset[0] == sizesCumSum[0])
				threadsData[n].headerGOB = 0x1;
			// The MAF header will not be decoded
			else
				threadsData[n].headerGOB = 0x0;
		}
							
		// The end position will be the size of the binary file + the size
		// of the header information read before this loop
		// For the first tread: endOffset = sizeOfBinaryFile + sizeOfHeaderInfo
		// For the other threads: previousEndOffSet + sizeOfBinaryFile
		//threadsData[n].endOffset = ((n != 0) ? 
		//	(threadsData[n-1].endOffset + sizeOfBinaryFiles[n]) : (sizeOfBinaryFiles[n] + encFileHeaderSize) );
		
		threadsData[n].storageBitsInfo = (StorageBitsInfo *)Calloc(1, sizeof(StorageBitsInfo));
		threadsData[n].modelsOrder = (ModelsOrder *)Calloc(1, sizeof(ModelsOrder));
		threadsData[n].hashTableInfo = (HashTableInfo *)Calloc(1, sizeof(HashTableInfo));
		
		threadsData[n].storageBitsInfo->storageBitsMSABRows = storageBitsMSABRows;
		threadsData[n].storageBitsInfo->storageBitsMSABCols = storageBitsMSABCols;
		threadsData[n].storageBitsInfo->storateBitsAbsScoreValue = storateBitsAbsScoreValue;
		threadsData[n].storageBitsInfo->storageBitsHeaderLine = storageBitsHeaderLine;
		threadsData[n].storageBitsInfo->storageBitsSrcName = storageBitsSrcName;
		threadsData[n].storageBitsInfo->storageBitsSourceSize = storageBitsSourceSize;
		threadsData[n].storageBitsInfo->storageBitsStartPosition = storageBitsStartPosition;
		threadsData[n].storageBitsInfo->storageBitsStartOffset = storageBitsStartOffset;
		threadsData[n].storageBitsInfo->storageBitsCountValue = storageBitsCountValue;
		
		threadsData[n].storageBitsInfo->storageBitsHashKeyValue = GetNumberOfBits(hashTableSize);
		threadsData[n].storageBitsInfo->storageBitsHashElementId = GetNumberOfBits(maxNumberOfElements);
		
		threadsData[n].modelsOrder->sLinesModelOrder = sLinesModelOrder;
		threadsData[n].modelsOrder->caseModelOrder = caseModelOrder;
		threadsData[n].modelsOrder->caseFlagModelOrder = caseFlagModelOrder;
		threadsData[n].modelsOrder->xchrModelOrder = xchrModelOrder;
		threadsData[n].modelsOrder->xchrFlagModelOrder = xchrFlagModelOrder;
		threadsData[n].modelsOrder->strandModelOrder = strandModelOrder;
		threadsData[n].modelsOrder->startOffsetSignModelOrder = startOffsetSignModelOrder;
		
		threadsData[n].modelsOrder->qLinesModelOrder = qLinesModelOrder;
		threadsData[n].modelsOrder->qLineFlagModelOrder = qLineFlagModelOrder;
		threadsData[n].modelsOrder->qLineInFlagModelOrder = qLineInFlagModelOrder;
		
		threadsData[n].modelsOrder->iLineFlagModelOrder = iLineFlagModelOrder;
		threadsData[n].modelsOrder->statusModelOrder = statusModelOrder;
		threadsData[n].modelsOrder->irregularStatusModelOrder = irregularStatusModelOrder;
		threadsData[n].modelsOrder->irregularCountModelOrder = irregularCountModelOrder;
		
		threadsData[n].modelsOrder->eLineFlagModelOrder = eLineFlagModelOrder;
		threadsData[n].modelsOrder->eLineIrregularStatusModelOrder = eLineIrregularStatusModelOrder;
				
		threadsData[n].hashTableInfo->hashTableSize = hashTableSize;
		threadsData[n].hashTableInfo->hashFunction = hashFunction;
		threadsData[n].hashTableInfo->maxNumberOfElements = maxNumberOfElements;
		threadsData[n].hashTableInfo->maxSrcNameSize = maxSrcNameSize;
	}	
	
	printf("Decode %"PRIu32" GOBs out of %"PRIu32" using %"PRIu32" threads.\n\n", nGOBsToDecode, nGOBs, nThreads);
	
	// Now we have read all the information needed to lauch the threads to decompress the MAF file
	for(n=0; n != nThreads; ++n)
	{
		// Lauch threads
		pthread_create(&threads[n], NULL, (void *) &DecodeMAFPart, (void *)&threadsData[n]);
		//pthread_create(&threads[n], NULL, (void *) &DecodeMAFPartV2, (void *)&threadsData[n]);
		printf("Thread %02"PRIu8" created and launched.\n", n);
	}

	// Wait for all threads to finish
	for(n=0; n != nThreads; ++n)
	{
		pthread_join(threads[n], NULL);
		printf("Thread %02"PRIu8" ended.\n", n);
	}
	
	
	// NULL DEVICE MODE NOT ACTIVATED
	#ifndef NULL_DEV
		// Merge the outputed files into a single MAF file
		if(mergeMAF == 0x1)
		{
			// Open output MAF file for writting
			outFp = Fopen(outFileName, "w");

			// Merging the decoded files in a single MAF file
			for(n = 0; n != nThreads; ++n)
			{
				// Open the file generated by each thread
				inFp = Fopen(threadsData[n].tmpOutDecFile, "r");
		
				// Read bufferSize bytes from the input file
				while(fgets(buffer, bufferSize, inFp) != NULL)
				{
					fprintf(outFp, "%s", buffer);
				}
				// Close file generated by a previous ended thread
				Fclose(inFp);
			}
			// Close the output file
			Fclose(outFp);
		}
	
		// It is necessary to remove the temporary decoded files
		if(removeTmpFiles == 0x1)
		{
			// Remove the temporary decoded files
			for(n = 0; n != nThreads; ++n)
			{
				Remove(threadsData[n].tmpOutDecFile);
			}
		}
	#endif
	
		
	// Free memory prevously allocated
	Free(sizeOfBinaryFiles, nGOBs*sizeof(uint64_t));
	Free(sizesCumSum, nGOBs*sizeof(uint64_t));
	Free(buffer, bufferSize*sizeof(char));
	
	for(n = 0; n != nThreads; ++n)
	{
		Free(threadsData[n].startOffset, threadsData[n].nGOBs*sizeof(off_t));
		Free(threadsData[n].storageBitsInfo, sizeof(StorageBitsInfo));
		Free(threadsData[n].modelsOrder, sizeof(ModelsOrder));
		Free(threadsData[n].hashTableInfo, sizeof(HashTableInfo));
	}
	Free(threadsData, nThreads*sizeof(ThreadsData));
		
	if(nThreads == 1)
	{
		printf("Total memory in use: "); PrintHumanReadableBytes(TotalMemory());
		printf(" (%"PRIu64" bytes)\n", TotalMemory());
	}
	printf("\n");
		
	return EXIT_SUCCESS;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// This function will decode a portion of the input compressedFile 
// according to the posStartIndicator and cumBytesPerMasterBlock indicators
void DecodeMAFPart(void *ptr)
{
	ThreadsData *threadsData;
	// Type cast to a pointer of ThreadData type                        
	threadsData = (ThreadsData *)ptr;

	char *headerLine;
	UChar template;
	//uint8_t n8, threadNumber, numberOfHeaderLines;
	uint8_t n8, numberOfHeaderLines;
	uint16_t storageBitsHeaderLine, storageBitsMSABRows, storageBitsMSABCols;
	uint32_t maxHeaderLineSize, n32;
	uint32_t nRows=0, nCols=0, GOBId;
	uint64_t totalMSABs=0;
	off_t startOffset;//, endOffset;
	FILE *outFp = NULL;
	
	ACModels *acModels;
	ACDecoder *acDecoder;
	CModels *cModels;
	MSAB *msab;
	CTemplate *cTemplate;
	
	//threadNumber = threadsData->threadNumber;
	numberOfHeaderLines = threadsData->numberOfHeaderLines;
	template = threadsData->template;
	maxHeaderLineSize = threadsData->maxHeaderLineSize;
	//startOffset = threadsData->startOffset;
	startOffset = threadsData->startOffset[0];
	//printf("\nthread: %"PRIu32" | startOffset: %"PRId64"\n", threadNumber, startOffset);
	
	storageBitsHeaderLine = threadsData->storageBitsInfo->storageBitsHeaderLine;
	storageBitsMSABRows = threadsData->storageBitsInfo->storageBitsMSABRows;
	storageBitsMSABCols = threadsData->storageBitsInfo->storageBitsMSABCols;
	
	headerLine = (char *)Calloc(maxHeaderLineSize, sizeof(UChar));
	
	acModels = CreateACModels();
	acDecoder = CreateACDecoder(threadsData->inputFile, startOffset);
	cModels = CreateCModels(threadsData->modelsOrder);
	msab = CreateEmptyMSAB(threadsData->modelsOrder, threadsData->hashTableInfo);
	cTemplate = InitTemplate(template, threadsData->modelsOrder->sLinesModelOrder);
	
	
	// Open temporary file for put the decoded information
	outFp = Fopen(threadsData->tmpOutDecFile, "wb");
	
	//printf("Output file: '%s'\n", threadsData->tmpOutDecFile);
	//printf("Input file: '%s'\n", threadsData->inputFile);
	
	//printf("thread: %"PRIu32" | Number of parts: %"PRIu32"\n", threadNumber, threadsData->nParts);
	
	//for(partId=0; partId != threadsData->nParts; ++partId)
	for(GOBId=0; GOBId < threadsData->nGOBs; GOBId++)
	{
		totalMSABs = 0;
		
		// Get the respective start Offset
		startOffset = threadsData->startOffset[GOBId];
		//printf("thread: %"PRIu32" | Part: %"PRIu32"| startOffset: %"PRId64"\n", threadNumber, partId, startOffset);
		
		// Reset models and decoder
		if(GOBId != 0)
		{
			//printf("Reset!!!!!!!!!!!!\n");
			
			// Reset CModels
			ResetCModels(cModels);
			// Reset ACModels
			ResetACModels(acModels);
			
			// Reset some buffers
			ResetBuffers(msab);
			
			// Reset hash table
			//ResetHashTable(msab->hashTable);
			
			FreeHashTable(msab->hashTable);
			msab->hashTable = CreateHashTable(threadsData->hashTableInfo);
			
			// Resume the ACDecoder using a different start offset
			ResumeACDecoder(acDecoder, threadsData->inputFile, startOffset);
		}
		
	
		// First we need to check if we are in the first thread. If that is the case, the decoder
		// needs to decode first the MAF header lines (lines that start by '#')
		//if(threadNumber == 0 && partId == 0)			
		if( (GOBId == 0) && (threadsData->headerGOB == 0x1) )
		{
			//printf("Decoding MAF header.\n");
			// Decode each header line
			for(n8 = 0; n8 != numberOfHeaderLines; ++n8)
			{
				// Decode the corresponding size (number of characters)
				n32 = readNBits(storageBitsHeaderLine, acDecoder->globalDecoder, acModels->binaryUniformACModel) + 1;

				// Decode the header line string
				readString(headerLine, n32, acDecoder->globalDecoder, acModels->binaryUniformACModel);
			
				// Save the header line
				fprintf(outFp, "%s", headerLine);
				//printf("Header Line (%"PRIu8") decoded: %s", n8, headerLine);
			}		
		}
		
		//print_ac_decoder_info(acDecoder->globalDecoder);
		//print_ac_model_info(acModels->binaryUniformACModel);
		
		// Here we will decode each MSAB while the Ftello function gives a value inside [startOffset - endOffset]
		while( (nRows = readNBits(storageBitsMSABRows, acDecoder->globalDecoder, acModels->binaryUniformACModel)) != 0)
		{
			totalMSABs++;
			// Read the number of columns
			nCols = readNBits(storageBitsMSABCols, acDecoder->globalDecoder, acModels->binaryUniformACModel);
			
			// Allocate memory for the 's' lines
			ResizeSLinesData(msab->sLinesData, nRows, nCols);
			
			// Decode the 's' lines
			DecodeSLinesData(msab, acModels, acDecoder, cModels, cTemplate, 
				threadsData->storageBitsInfo);
			//printf("S lines Decoded...\n\n");
		
			// Decode the 'q' lines
			DecodeQLinesData(msab, acModels, acDecoder, cModels, threadsData->modelsOrder);
			//printf("Q lines Decoded...\n\n");
		
			// Decode the 'i' lines
			DecodeILinesData(msab, acModels, acDecoder, cModels, threadsData->storageBitsInfo);
			//printf("I lines Decoded...\n\n");
		
			// Decode the 'e' lines
			DecodeELinesData(msab, acModels, acDecoder, cModels, threadsData->storageBitsInfo);
			//printf("E lines Decoded...\n\n");

			// Write the decoded information to the output FILE
			WriteMSABToFile(msab, outFp);
		
			// Reset MSAB
			ResetMSABWith(msab);		
		}
		
		// Reset previous 'e' lines information after each slice/part
		msab->prevELinesInfo->currentNRows = 0;
		
		// Only decode the "##eof maf" flag if this the last part
		if( (threadsData->lastGOB == 0x1) && (GOBId == threadsData->nGOBs-1) )
		{
			// Verify if original file ends with a "##eof maf"
			if (readNBits(1, acDecoder->globalDecoder, acModels->binaryUniformACModel) == 0x1)
				fprintf(outFp, "##eof maf\n");
		}
		
		ACDecoderDone(acDecoder);
	}
	
	// Close output file
	Fclose(outFp);
	
	// Terminate decoding process
	//ACDecoderDone(acDecoder);
	
	
	// Free memory
	Free(headerLine, maxHeaderLineSize*sizeof(char));
	FreeACDecoder(acDecoder);
	FreeACModels(acModels);
	FreeCModels(cModels);
	FreeTemplate(cTemplate);
	FreeMSABWith(msab);
	
	pthread_exit(NULL);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void DecodeSLinesData(MSAB *msab, ACModels *acModels, ACDecoder *acDecoder, 
	CModels *cModels, CTemplate *cTemplate, StorageBitsInfo *storageBitsInfo)
{
	UChar c;
	uint8_t binarySymbol, s, strandSymbol;
	uint16_t storateBitsAbsScoreValue, startOffset;
	uint32_t row, col, score, start, sourceSize=0;
	Element *element;
	size_t sourceNameSize;
		
	//////////////////////////////////////////////////////// SCORE DECODING BEGIN
	storateBitsAbsScoreValue = storageBitsInfo->storateBitsAbsScoreValue;
	
	// First decode the score signal
	binarySymbol = readNBits(1, acDecoder->globalDecoder, acModels->binaryUniformACModel);

	// Decode the absolute score value
	score = readNBits(storateBitsAbsScoreValue, acDecoder->globalDecoder, acModels->binaryUniformACModel);
	
	// Save the score value with the respective signal (negative or positive)
	msab->score = ( (binarySymbol == 0x1) ? (double)(-1.0 * score): (double)score);
	//////////////////////////////////////////////////////// SCORE DECODING ENDS
	//printf("msab->score = %lf\n", msab->score);
		
	//////////////////////////////////////////////////////// CASE AND EXTRA CHAR INFO DECODING BEGIN
	ComputePModel2(cModels->caseFlagCModel, acModels->binaryACModel);
	msab->lowerCaseCount = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
	UpdateCModelCounter(cModels->caseFlagCModel, msab->lowerCaseCount);
	UpdateCModelIdx(cModels->caseFlagCModel, msab->lowerCaseCount);
	
	ComputePModel2(cModels->xchrFlagCModel, acModels->binaryACModel);
	msab->xchrCount = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
	UpdateCModelCounter(cModels->xchrFlagCModel, msab->xchrCount);
	UpdateCModelIdx(cModels->xchrFlagCModel, msab->xchrCount);
	//////////////////////////////////////////////////////// CASE AND EXTRA CHAR INFO DECODING BEGIN

	
	//////////////////////////////////////////////////////// HEADER S LINES DECODING BEGIN
	for(row = 0; row != msab->sLinesData->nRows; ++row)
	{

		//////////////////////////////////////////////////////// HASH KEY AND ELEMENT ID DECODING BEGIN
		// Read the hash key and element ID
		msab->hashPosition->hashKey = readNBits(storageBitsInfo->storageBitsHashKeyValue,
			acDecoder->globalDecoder, acModels->binaryUniformACModel);
		msab->hashPosition->elementId = readNBits(storageBitsInfo->storageBitsHashElementId,
			acDecoder->globalDecoder, acModels->binaryUniformACModel);
		//////////////////////////////////////////////////////// HASH KEY AND ELEMENT ID DECODING END
		
		//////////////////////////////////////////////////////// STRAND DECODING BEGIN
		ComputePModel2(cModels->strandCModel, acModels->binaryACModel);
		strandSymbol = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
		UpdateCModelCounter(cModels->strandCModel, strandSymbol);
		UpdateCModelIdx(cModels->strandCModel, strandSymbol);
		//////////////////////////////////////////////////////// STRAND DECODING BEGIN
		
			
		// After having the hash position of the element in question, the decoder must
		// get the element from the hash table
		element = GetElement(msab->hashTable, msab->hashPosition);

		if(IsElementEmpty(element) == 0x1)
		{
			//////////////////////////////////////////////////////// SOURCE NAME DECODING BEGIN
			// Decode the source name
			sourceNameSize = readNBits(storageBitsInfo->storageBitsSrcName, acDecoder->globalDecoder, 
				acModels->binaryUniformACModel);
			readString(msab->sourceName, sourceNameSize, acDecoder->globalDecoder, acModels->binaryUniformACModel);
			//////////////////////////////////////////////////////// SOURCE NAME DECODING END
			
			
			//////////////////////////////////////////////////////// START POSITION DECODING BEGIN
			start = readNBits(storageBitsInfo->storageBitsStartPosition, acDecoder->globalDecoder, 
				acModels->binaryUniformACModel);
			//////////////////////////////////////////////////////// START POSITION DECODING END
				
			
			//////////////////////////////////////////////////////// SOURCE SIZE DECODING BEGIN
			sourceSize = readNBits(storageBitsInfo->storageBitsSourceSize, acDecoder->globalDecoder,
				acModels->binaryUniformACModel);
			//////////////////////////////////////////////////////// SOURCE SIZE DECODING BEGIN

		}
		
		// If the element was already decoded before
		else
		{
			// Decode the start offset sign
			ComputePModel2(cModels->startOffsetSignCModel, acModels->binaryACModel);
			binarySymbol = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
			UpdateCModelCounter(cModels->startOffsetSignCModel, binarySymbol);
			UpdateCModelIdx(cModels->startOffsetSignCModel, binarySymbol);
			
			
			//////////////////////////////////////////////////////// RAW START POSITION DECODING BEGIN
			// The offset sign is negative? Decode the start position as raw
			if(binarySymbol == 0x1)
			{
				start = readNBits(storageBitsInfo->storageBitsStartPosition, acDecoder->globalDecoder, 
					acModels->binaryUniformACModel);
			}
			//////////////////////////////////////////////////////// RAW START POSITION DECODING END
			
			//////////////////////////////////////////////////////// OFFSET START POSITION DECODING BEGIN
			else
			{
				startOffset = readNBits(storageBitsInfo->storageBitsStartOffset, acDecoder->globalDecoder, 
					acModels->binaryUniformACModel);
				start = element->prevStart + element->seqSize + startOffset;
			}
			//////////////////////////////////////////////////////// OFFSET START POSITION DECODING END
		}
		
		// Store the line info
		StoreLineInfo(msab->linesInfo, element, NULL, row, msab->sourceName, start, 0, 
			strandSymbol, sourceSize, 0x0);
	}
	//////////////////////////////////////////////////////// HEADER S LINES DECODING END
	
	
	//////////////////////////////////////////////////////// S LINES DECODING BEGIN
	for(row = 0; row != msab->sLinesData->nRows; ++row)
	{	
		// Initialize the sequence size field
		msab->linesInfo->elements[row]->seqSize = 0;
		
		// Initialize the qualityInfo flag to 0x0
		msab->linesInfo->elements[row]->qualityInfo = 0x0;
		
		for(col = 0; col != msab->sLinesData->nCols; ++col)
		{
			GetPModelIdx(msab, row, col, cTemplate, cModels->sValuesCModel);
			ComputePModel(cModels->sValuesCModel, acModels->sValuesACModel);
			// Decode base or gap symbol
			s = acDecSymLowSizeVar(acDecoder->globalDecoder, acModels->sValuesACModel);
			c = Symbol2SequenceValueCharacter(s);
			UpdateCModelCounter(cModels->sValuesCModel, s);
			
			
			//////////////////////////////////////////////////////// CASE DECODING BEGIN
			// It is not a gap and we have lower case symbols?
			if(s != GAP_SYMBOL && msab->lowerCaseCount)
			{
				ComputePModel2(cModels->caseCModel, acModels->binaryACModel);
				binarySymbol = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
				UpdateCModelCounter(cModels->caseCModel, binarySymbol);
				UpdateCModelIdx(cModels->caseCModel, binarySymbol);
				
				// Is a lower case base?
				if(binarySymbol == 0x1)
					c = tolower(c); // Convert to lower case symbol
			}
			//////////////////////////////////////////////////////// CASE DECODING END 
			
			
			//////////////////////////////////////////////////////// EXTRA BASES DECODING BEGIN
			// It is a possible extra symbol (N or n) and there are extra symbols in the current
			// MSAB?
			if(s == XCHR_SYMBOL && msab->xchrCount)
			{
				ComputePModel2(cModels->xchrCModel, acModels->binaryACModel);
				binarySymbol = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
				UpdateCModelCounter(cModels->xchrCModel, binarySymbol);
				UpdateCModelIdx(cModels->xchrCModel, binarySymbol);
				
				// Is the decoded symbol an extra symbol (n or N)?
				if(binarySymbol == 0x1)
					c = (isupper(c) ? 'N' : 'n');
			}
			//////////////////////////////////////////////////////// EXTRA BASES DECODING END
			
			// Update sequence length value
			if(s != GAP_SYMBOL)
				msab->linesInfo->elements[row]->seqSize++;
			
			// Set the sequence value (base or gap)
			SetSequenceValueCharacter(msab->sLinesData, row, col, c);	
		}
	}
	//////////////////////////////////////////////////////// S LINES DECODING END
	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void DecodeQLinesData(MSAB *msab, ACModels *acModels, ACDecoder *acDecoder, 
	CModels *cModels, ModelsOrder *modelsOrder)
{
	uint8_t binarySymbol, qValue;
	uint32_t row, col, numberOfQLines=0, currentQLineRow = 0;

	
	//////////////////////////////////////////////////////// Q LINES DECODING BEGIN
	// First thing to do is find out if there is any 'q' lines to decode
	ComputePModel2(cModels->qLineFlagCModel, acModels->binaryACModel);
	binarySymbol = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
	UpdateCModelCounter(cModels->qLineFlagCModel, binarySymbol);
	UpdateCModelIdx(cModels->qLineFlagCModel, binarySymbol);
	
	//printf("q lines binary flag decoded %"PRIu8"\n", binarySymbol);
	
	// There are 'q' lines to decode
	if(binarySymbol == 0x1)
	{
		// Decode the position of each 'q' line first before decoding the quality values
		for(row = 1; row != msab->sLinesData->nRows; ++row)
		{
			ComputePModel2(cModels->qLineInFlagCModel, acModels->binaryACModel);
			binarySymbol = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
			UpdateCModelCounter(cModels->qLineInFlagCModel, binarySymbol);
			UpdateCModelIdx(cModels->qLineInFlagCModel, binarySymbol);
			
			// Update the counter with the number of 'q' lines
			numberOfQLines += binarySymbol;
			// Set the position flag
			msab->linesInfo->elements[row]->qualityInfo = binarySymbol;
		}
		
		// Allocate memory to save the decoded quality values
		ResizeQLinesData(msab->qLinesData, numberOfQLines, msab->sLinesData->nCols);

		// Loop all the row (from the 's' lines block)
		for(row = 1; row != msab->sLinesData->nRows; ++row)
		{
			// This row has a 'q' line associated?
			if(msab->linesInfo->elements[row]->qualityInfo == 0x1)
			{	
				// Loop all the columns
				for(col = 0; col != msab->sLinesData->nCols; ++col)
				{
					// Decode only if the previous 's' line does not have a gap symbol
					if(GetSequenceValueCharacter(msab->sLinesData, row, col) != '-')
					{
						// Decode the actually quality value
						GetPModelIdx3(msab->qLinesData->buffer, msab->qLinesData->bufSize, cModels->qValuesCModel);
						ComputePModel(cModels->qValuesCModel, acModels->qValuesACModel);
						qValue = acDecSymLowSizeVar(acDecoder->globalDecoder, acModels->qValuesACModel);
						UpdateCModelCounter(cModels->qValuesCModel, qValue);
						ShiftBuffer(msab->qLinesData->buffer, msab->qLinesData->bufSize, qValue);
								
						// Set the quality value
						msab->qLinesData->data[currentQLineRow][col] = qValue;
					}
					// Put a gap symbol in the 'q' line
					else
					{
						// Set an alignment gap
						msab->qLinesData->data[currentQLineRow][col] = Q_GAP_SYMBOL;
					}
				}
				// Update the current
				currentQLineRow++;
			}
		}
		
	}
	//////////////////////////////////////////////////////// Q LINES DECODING END
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void DecodeILinesData(MSAB *msab, ACModels *acModels, ACDecoder *acDecoder, 
	CModels *cModels, StorageBitsInfo *storageBitsInfo)
{
	uint8_t binaryFlag, leftStatus, rightStatus;
	uint32_t row, sRow, leftCount, rightCount;
	 
	//////////////////////////////////////////////////////// I LINES DECODING BEGIN
	// The first thing to do is to verify if there is actually any 'i' lines to encode
	ComputePModel2(cModels->iLineFlagCModel, acModels->binaryACModel);
	binaryFlag = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
	UpdateCModelCounter(cModels->iLineFlagCModel, binaryFlag);
	UpdateCModelIdx(cModels->iLineFlagCModel, binaryFlag);
	
	// Is there 'i' lines to decode?
	if(binaryFlag == 0x1)
	{
		// The number of 'i' lines to decode is associated with the number of 's'
		// lines of the current MSAB block. 
		for(row = 0; row != (msab->sLinesData->nRows-1); ++row)
		{
			// This will be necessary to verify if an 'i' line of the current source
			// was encoded before or not
			sRow = row + 1;
			
			// No 'i' line was encoded before of the current source
			// It is necessary to decode both (left and right) status and counts 
			if(msab->linesInfo->elements[sRow]->iStatus == 0x0)
			{
				
				//////////////////////////////////////////////////////// LEFT STATUS SYMBOL DECODING BEGIN
				GetPModelIdx3(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, cModels->statusCModel);
				ComputePModel(cModels->statusCModel, acModels->statusACModel);
				leftStatus = acDecSymLowSizeVar(acDecoder->globalDecoder, acModels->statusACModel);
				UpdateCModelCounter(cModels->statusCModel, leftStatus);
				ShiftBuffer(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, leftStatus);
				//////////////////////////////////////////////////////// LEFT STATUS SYMBOL DECODING END
				
				
				//////////////////////////////////////////////////////// LEFT COUNT DECODING BEGIN
				leftCount = readNBits(storageBitsInfo->storageBitsCountValue, acDecoder->globalDecoder,
					acModels->binaryUniformACModel);
				//////////////////////////////////////////////////////// LEFT COUNT DECODING END
			}
			
			// The 'i' line of the current source was encoded before,
			// find out if the previous right status symbol and count are the same as the current
			// left status symbol and count
			else
			{
				//////////////////////////////////////////////////////// IRREGULAR LEFT STATUS SYMBOL DECODING BEGIN
				// Is this an irregular status symbol?
				ComputePModel2(cModels->irregularStatusCModel, acModels->binaryACModel);
				binaryFlag = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
				UpdateCModelCounter(cModels->irregularStatusCModel, binaryFlag);
				UpdateCModelIdx(cModels->irregularStatusCModel, binaryFlag);
				
				// An irregular status symbol needs to be decoded
				if(binaryFlag == 0x1)
				{
					GetPModelIdx3(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, cModels->statusCModel);
					ComputePModel(cModels->statusCModel, acModels->statusACModel);
					leftStatus = acDecSymLowSizeVar(acDecoder->globalDecoder, acModels->statusACModel);
					UpdateCModelCounter(cModels->statusCModel, leftStatus);
					ShiftBuffer(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, leftStatus);
				}
				// The last right status symbol corresponds to the current left status symbol
				else
				{
					leftStatus = msab->linesInfo->elements[sRow]->lastRightStatus;
				}
				//////////////////////////////////////////////////////// IRREGULAR LEFT STATUS SYMBOL DECODING END
				
				
				//////////////////////////////////////////////////////// IRREGULAR LEFT COUNT DECODING BEGIN
				// Is this an irregular count value?
				ComputePModel2(cModels->irregularCountCModel, acModels->binaryACModel);
				binaryFlag = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
				UpdateCModelCounter(cModels->irregularCountCModel, binaryFlag);
				UpdateCModelIdx(cModels->irregularCountCModel, binaryFlag);
				
				// An irregular count value needs to be decoded
				if(binaryFlag == 0x1)
				{
					leftCount = readNBits(storageBitsInfo->storageBitsCountValue, acDecoder->globalDecoder,
						acModels->binaryUniformACModel);
					
				}
				// The last right count value corresponds to the current left count value
				else
				{
					leftCount = msab->linesInfo->elements[sRow]->lastRightCount;
				}
				//////////////////////////////////////////////////////// IRREGULAR LEFT COUNT DECODING END
			}
			
			// The right status symbol and count must be always decoded even if a previous 'i' line 
			// was encoded before of the same source
			//////////////////////////////////////////////////////// RIGHT STATUS SYMBOL DECODING BEGIN
			GetPModelIdx3(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, cModels->statusCModel);
			ComputePModel(cModels->statusCModel, acModels->statusACModel);
			rightStatus = acDecSymLowSizeVar(acDecoder->globalDecoder, acModels->statusACModel);
			UpdateCModelCounter(cModels->statusCModel, rightStatus);
			ShiftBuffer(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, rightStatus);
			//////////////////////////////////////////////////////// RIGHT STATUS SYMBOL DECODING END
			
			//////////////////////////////////////////////////////// LEFT COUNT DECODING BEGIN
			rightCount = readNBits(storageBitsInfo->storageBitsCountValue, acDecoder->globalDecoder,
				acModels->binaryUniformACModel);
			//////////////////////////////////////////////////////// LEFT COUNT DECODING END
			
			// Store the last right status and count encoded
			msab->linesInfo->elements[sRow]->lastRightStatus = rightStatus;
			msab->linesInfo->elements[sRow]->lastRightCount = rightCount;
			
			
			// Save the 'i' line information in order to latter be written in the output MAF file
			StoreInfoRowV2(msab->iLinesData, leftStatus, rightStatus, leftCount, rightCount);
			
			// Change the status flag for the 'i' line of the current source
			msab->linesInfo->elements[sRow]->iStatus = 0x1;
		}
	}
	//////////////////////////////////////////////////////// I LINES DECODING END
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void DecodeELinesData(MSAB *msab, ACModels *acModels, ACDecoder *acDecoder, 
	CModels *cModels, StorageBitsInfo *storageBitsInfo)
{
	uint8_t binaryFlag, strandSymbol, statusSymbol;
	uint32_t row, nRows, start, sourceSize, seqSize;
	size_t sourceNameSize;
	Element *element;
	
	//////////////////////////////////////////////////////// E LINES DECODING BEGIN
	// The first thing to do is to verify if there is actually any 'e' lines to encode
	ComputePModel2(cModels->eLineFlagCModel, acModels->binaryACModel);
	binaryFlag = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
	UpdateCModelCounter(cModels->eLineFlagCModel, binaryFlag);
	UpdateCModelIdx(cModels->eLineFlagCModel, binaryFlag);
	
	// This means that there are 'e' lines to decode in the current MSAB
	if(binaryFlag == 0x1)
	{
		
		// Get the number of 'e' lines that the current MSAB have
		nRows = readNBits(storageBitsInfo->storageBitsMSABRows, acDecoder->globalDecoder, 
			acModels->binaryUniformACModel);
					
		// Decode each 'e' line
		for(row = 0; row != nRows; ++row)
		{
			// First of all, decode the Hash Info
			//////////////////////////////////////////////////////// HASH KEY AND ELEMENT ID DECODING BEGIN
			// Read the hash key and element ID
			msab->hashPosition->hashKey = readNBits(storageBitsInfo->storageBitsHashKeyValue,
				acDecoder->globalDecoder, acModels->binaryUniformACModel);
			msab->hashPosition->elementId = readNBits(storageBitsInfo->storageBitsHashElementId,
				acDecoder->globalDecoder, acModels->binaryUniformACModel);			
			//////////////////////////////////////////////////////// HASH KEY AND ELEMENT ID DECODING END
			
		
			// After having the hash position of the element in question, the decoder must
			// get the element from the hash table
			element = GetElement(msab->hashTable, msab->hashPosition);
			//printf("Element possible source: '%s'\n", element->sourceName);
			
			// If the element was not encoded, it is necessary to encode all the information
			if(IsElementEmpty(element) == 0x1)
			{

				//////////////////////////////////////////////////////// SOURCE NAME DECODING BEGIN
				// Decode the source name
				sourceNameSize = readNBits(storageBitsInfo->storageBitsSrcName, acDecoder->globalDecoder, 
					acModels->binaryUniformACModel);
				readString(msab->sourceName, sourceNameSize, acDecoder->globalDecoder, acModels->binaryUniformACModel);
				//////////////////////////////////////////////////////// SOURCE NAME DECODING END
				

				//////////////////////////////////////////////////////// START POSITION DECODING BEGIN
				start = readNBits(storageBitsInfo->storageBitsStartPosition, acDecoder->globalDecoder, 
					acModels->binaryUniformACModel);
				//////////////////////////////////////////////////////// START POSITION DECODING END


				//////////////////////////////////////////////////////// SEQ SIZE POSITION DECODING BEGIN
				seqSize = readNBits(storageBitsInfo->storageBitsMSABCols, acDecoder->globalDecoder,
					acModels->binaryUniformACModel);
				//////////////////////////////////////////////////////// SEQ SIZE POSITION DECODING END


				//////////////////////////////////////////////////////// STRAND DECODING BEGIN
				ComputePModel2(cModels->strandCModel, acModels->binaryACModel);
				strandSymbol = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
				UpdateCModelCounter(cModels->strandCModel, strandSymbol);
				UpdateCModelIdx(cModels->strandCModel, strandSymbol);
				//////////////////////////////////////////////////////// STRAND DECODING BEGIN


				//////////////////////////////////////////////////////// SOURCE SIZE DECODING BEGIN
				sourceSize = readNBits(storageBitsInfo->storageBitsSourceSize, 
					acDecoder->globalDecoder, acModels->binaryUniformACModel);
				//////////////////////////////////////////////////////// SOURCE SIZE DECODING BEGIN

				//////////////////////////////////////////////////////// STATUS SYMBOL DECODING BEGIN
				GetPModelIdx3(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, 
					cModels->eLineStatusCModel);
				ComputePModel(cModels->eLineStatusCModel, acModels->statusACModel);
				statusSymbol = acDecSymLowSizeVar(acDecoder->globalDecoder, acModels->statusACModel);
				UpdateCModelCounter(cModels->eLineStatusCModel, statusSymbol);
				ShiftBuffer(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, statusSymbol);
				//////////////////////////////////////////////////////// STATUS SYMBOL DECODING END	
			
				// Store the element information
				StoreLineInfo(msab->eLinesInfo, element, NULL, row, msab->sourceName, start,
				0, strandSymbol, sourceSize, 0x0);
				
				msab->eLinesInfo->elements[row]->seqSize = seqSize;
				// Update the last status symbol
				msab->eLinesInfo->elements[row]->lastStatusSymbol = statusSymbol;
			}
			
			// The current source information was already encoded before
			// Information like source name, source size was already decoded before
			else
			{

				// This function will allocate memory in the array of pointers "msab->eLinesInfo"
				// if necessary and it will associate the element in the correct postition
				InsertElement(msab->eLinesInfo, element, row);
						
				strandSymbol = element->strand;
				sourceSize = element->sourceSize;
				// HERE
				start = element->start;
				seqSize = element->seqSize;
				
				// The previous MSAB has a 'e' line of the same source?
				if(element->closeELine == 0x1)
				{
					// HERE					
					//start = element->start;
					//seqSize = element->seqSize; 
					
					// If it is a regular status symbol, the status symbol is the same as the previous 
					// 'e' line
					statusSymbol = element->lastStatusSymbol;
										
					// Decode irregular flag for status symbol
					ComputePModel2(cModels->eLineIrregularStatusCModel, acModels->binaryACModel);
					binaryFlag = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
					UpdateCModelCounter(cModels->eLineIrregularStatusCModel, binaryFlag);
					UpdateCModelIdx(cModels->eLineIrregularStatusCModel, binaryFlag);

					// Irregular status symbol?
					if(binaryFlag == 0x1)
					{
						
						// Decode status symbol
						//////////////////////////////////////////////////////// STATUS SYMBOL DECODING BEGIN
						GetPModelIdx3(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, 
							cModels->eLineStatusCModel);
						ComputePModel(cModels->eLineStatusCModel, acModels->statusACModel);
						statusSymbol = acDecSymLowSizeVar(acDecoder->globalDecoder, acModels->statusACModel);
						UpdateCModelCounter(cModels->eLineStatusCModel, statusSymbol);
						ShiftBuffer(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, statusSymbol);
						//////////////////////////////////////////////////////// STATUS SYMBOL DECODING END	
						
						// Update the last status symbol
						element->lastStatusSymbol = statusSymbol;
					}
					
					
				}
				// This source of this 'e' line was already encoded in an 'i' line?
				else if(element->iStatus == 0x1)
				{
					
					start = element->start + element->seqSize;
					seqSize = element->lastRightCount;
						
					// If an 'i' line was decoded before, the status symbol (if it is a regular one)
					// is the same as the last right status symbol
					statusSymbol = element->lastRightStatus;
					
					
					// Decode irregular flag for status symbol
					ComputePModel2(cModels->eLineIrregularStatusCModel, acModels->binaryACModel);
					binaryFlag = acDecodeBinary(acDecoder->globalDecoder, acModels->binaryACModel);
					UpdateCModelCounter(cModels->eLineIrregularStatusCModel, binaryFlag);
					UpdateCModelIdx(cModels->eLineIrregularStatusCModel, binaryFlag);
					
					// Irregular status symbol?
					if(binaryFlag == 0x1)
					{
						// Decode status symbol
						//////////////////////////////////////////////////////// STATUS SYMBOL DECODING BEGIN
						GetPModelIdx3(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, 
							cModels->eLineStatusCModel);
						ComputePModel(cModels->eLineStatusCModel, acModels->statusACModel);
						statusSymbol = acDecSymLowSizeVar(acDecoder->globalDecoder, acModels->statusACModel);
						UpdateCModelCounter(cModels->eLineStatusCModel, statusSymbol);
						ShiftBuffer(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, statusSymbol);
						//////////////////////////////////////////////////////// STATUS SYMBOL DECODING END	
						
					}
					
					// Update start and prevStart position
					msab->eLinesInfo->elements[row]->start += msab->eLinesInfo->elements[row]->seqSize;
					msab->eLinesInfo->elements[row]->prevStart = msab->eLinesInfo->elements[row]->start;
					
					// Update the last status symbol
					element->lastStatusSymbol = statusSymbol;
				}
				// No 'i' lines were encoded for this source
				// Also, there are no 'e' line in the previous MSAB of this source
				else
				{
					// Decode the status symbol
					//////////////////////////////////////////////////////// STATUS SYMBOL DECODING BEGIN
					GetPModelIdx3(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, 
						cModels->eLineStatusCModel);
					ComputePModel(cModels->eLineStatusCModel, acModels->statusACModel);
					statusSymbol = acDecSymLowSizeVar(acDecoder->globalDecoder, acModels->statusACModel);
					UpdateCModelCounter(cModels->eLineStatusCModel, statusSymbol);
					ShiftBuffer(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, statusSymbol);
					//////////////////////////////////////////////////////// STATUS SYMBOL DECODING END	

					element->lastStatusSymbol = statusSymbol;
				}
					
			}
			// Store the 'e' line information
			StoreEmtpyRegionRowV2(msab->eLinesData, start, seqSize, strandSymbol, sourceSize, statusSymbol);				
			msab->eLinesInfo->elements[row]->seqSize = seqSize;
			
			
		}
	}
	//////////////////////////////////////////////////////// E LINES DECODING END
	else
	{
		// No e lines in the current MSAB so we need to set the current number of eLinesInfo
		// to zero!
		msab->eLinesInfo->currentNRows = 0;
	}
	
	// Reset close 'e' lines flag and update the prevELinesInfo field
	ResetELinesInfo(msab);	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
void WriteMSABToFile(MSAB *msab, FILE *outFp)
{
	uint8_t nPlaces[5] = {0, 0, 0, 0, 0}, totalPlaces = 5;
	uint32_t row, col, currentQLineRow = 0;
	
	GetNPlaces(msab, nPlaces);
	totalPlaces += nPlaces[4];
			
	// Print score information
	fprintf(outFp, "a score=%.6lf\n", msab->score);

	for(row = 0; row != msab->sLinesData->nRows; ++row)
	{
		// Write the source name string
		fprintf(outFp, "s %-*s ", nPlaces[0], msab->linesInfo->elements[row]->sourceName);
		
		// Write the start position field
		fprintf(outFp, "%*"PRIu32" ", nPlaces[1], msab->linesInfo->elements[row]->start);
		
		// Write the sequence size field
		fprintf(outFp, "%*"PRIu32" ", nPlaces[2], msab->linesInfo->elements[row]->seqSize);
		
		// Strand information
		fprintf(outFp, "%1c ", ((msab->linesInfo->elements[row]->strand == 0) ? '-' : '+'));
		
		// Write the source size
		fprintf(outFp, "%*"PRIu32" ", nPlaces[3], msab->linesInfo->elements[row]->sourceSize);
		
		// Write DNA bases and gaps
		for(col = 0; col != msab->sLinesData->nCols; ++col)
		{
			putc(GetSequenceValueCharacter(msab->sLinesData, row, col), outFp);
		}
		fprintf(outFp,"\n");
		
		// Is there any 'q' lines to write to the output MAF file
		if( (msab->qLinesData != NULL) && (msab->qLinesData->nRows != 0) )
		{
			// The current 's' line has some 'q' line associated?
			if(msab->linesInfo->elements[row]->qualityInfo == 0x1)
			{
				fprintf(outFp, "q %-*s ", totalPlaces, msab->linesInfo->elements[row]->sourceName);
				for(col = 0; col < msab->qLinesData->nCols; col++)
				{
					//printf("qValue = %"PRIu8"\n", GetQualityValue(msab->qLinesData, currentQLineRow, col));
					putc(Symbol2QualityValue(GetQualityValue(msab->qLinesData, currentQLineRow, col)), outFp);
				}
				fprintf(outFp,"\n");
				
				// Increase the current 'q' line row
				currentQLineRow++;
			}
			
		}
		
		// Is there any 'i' lines to write to the output MAF file
		if( (msab->iLinesData->nRows != 0) && (row != 0) )
		{
			fprintf(outFp, "i %-*s", nPlaces[0], msab->linesInfo->elements[row]->sourceName);
			fprintf(outFp, " %c %"PRIu32"", Symbol2StatusCharacter(msab->iLinesData->leftStatus[row-1]), msab->iLinesData->leftCount[row-1]);
			fprintf(outFp, " %c %"PRIu32"\n", Symbol2StatusCharacter(msab->iLinesData->rightStatus[row-1]), msab->iLinesData->rightCount[row-1]);
		}
	}
	
	
	// Write the 'e' lines info
	for(row = 0; row != msab->eLinesData->nRows; ++row)
	{
		
		// Write the source name string
		fprintf(outFp, "e %-*s ", nPlaces[0], msab->eLinesInfo->elements[row]->sourceName);
		
		// Write the start position field
		fprintf(outFp, "%*"PRIu32" ", nPlaces[1], msab->eLinesInfo->elements[row]->start);		
		
		// Write the sequence size field
		fprintf(outFp, "%*"PRIu32" ", nPlaces[2], msab->eLinesInfo->elements[row]->seqSize);
		
		// Strand information
		fprintf(outFp, "%1c ", ((msab->eLinesInfo->elements[row]->strand == 0) ? '-' : '+'));
		
		// Write the source size
		fprintf(outFp, "%*"PRIu32" ", nPlaces[3], msab->eLinesInfo->elements[row]->sourceSize);
		
		// Write the status symbol
		fprintf(outFp, "%1c\n", Symbol2StatusCharacter(msab->eLinesData->status[row]));
	}
	
	fprintf(outFp,"\n");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
