// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - --
//	Copyright 2014 IEETA/DETI - University of Aveiro, Portugal.			-
//	All Rights Reserved.								-
//											-
//	These programs are supplied free of charge for research purposes only,   	-
//	and may not be sold or incorporated into any commercial product. There   	-
//	is ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they     	-
//	are fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you	 	- 
//	do happen to find a bug, or have modifications to suggest, please report	-
//	the same to Luis M. O. Matos, luismatos@ua.pt. The copyright notice      	-
//	above and this statement of conditions must remain an integral part of   	-
//	each and every copy made of these files.				 	-
//											-
//	Description: compressor for MAF files						-
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
#include "ac.h"
#include "msab.h"
#include "context.h"
#include "models.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	uint8_t 		threadNumber;					// Thread ID
	uint8_t 		nThreads;						// Total number of threads	
	uint64_t 		nTotalBytes;					// Number of bytes of the original file
	uint32_t		rowBlockSize;					// Slot size used in the memory allocation process
	UChar			template;						// Template used for compressing the 's' lines
	
	uint32_t		firstGOBId;						// The global ID of the first Group Of Blocks
	uint32_t		nGOBs;							// Number of Group of Blocks for the current thread
	uint32_t		totalNumberOfGOBs;				// Total Group of Blocks
	
	//uint32_t		firstPartId;					// The global Id of the first part for the current thread
	//uint32_t		nParts;							// Number of parts for the current thread 
	//uint32_t		totalNumberOfParts;				// Total number of parts
	
	
	char 			inputFile[FILENAME_MAX];		// Input MAF file
	char 			tmpOutEncFile[FILENAME_MAX];	// Temporary file to put the encoded files
	
	char			tmpMSABInfoFile[FILENAME_MAX];	// Log file used to write the MSAB sizes
	
	ModelsOrder		*modelsOrder;					// Hold the information regarding the models size to encode
													// the information of each MSAB
	FieldsLimits	*fieldsLimits;					// Holds the field limits (max value allowed)
	StorageBitsInfo	*storageBitsInfo;				// Holds the storage bits information (number of bits to use
													// in the writeNBits function)
	HashTableInfo	*hashTableInfo;					// Information to create the hash table (size, hash function, etc.)
}
ThreadsData;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	uint8_t		numberOfHeaderLines;		// Number of MAF header lines (lines that start with '#')
	
	uint64_t	*totalBlocks;				// Total number of MSAB
	uint64_t	*totalLowerCaseBlocks;		// Number of MSAB with lower case bases
	uint64_t	*totalXchrBlocks;			// Number of MSAB with extra characters
	
	uint64_t	*totalSLines;				// Number of 's' lines
	uint64_t	*totalQLines;				// Number of 'q' lines
	uint64_t	*totalILines;				// Number of 'i' lines
	uint64_t	*totalELines;				// Number of 'e' lines
	
	uint64_t	*totalBases;				// Total number of bases (with gaps)
	uint64_t	*totalGaps;					// Total number of gap symbols
	uint64_t	*totalLowerCaseSymbols;		// Total number of lower case symbols
	uint64_t	*totalXcharN;				// Total number of extra symbols (n's and N's)
	uint64_t	*totalXcharNC;				// Total number of extra symbols (n's, N's, c's and C's)
			
	uint64_t	*totalNumNegativeOff;		// Total number of negative start offsets
	uint64_t	*totalNumLargeOff;			// Total number of too large offsets
	
	uint64_t	*totalIrregularStatusI;		// Total number of irregular status in 'i' lines
	uint64_t	*totalIrregularCounts;		// Total number of irregular counts in 'i' lines
	
	uint64_t	*totalIrregularStatusE;		// Total number of irregular status in 'e' lines
	uint64_t	*totalIrregularStatusEI;	// Total number of irregular status in 'e' lines
	
	uint64_t 	*totalBits;					// Total bits of the encoded file
	
	uint64_t	*maxStartOffset;			// Maximum offset value computed
}
GlobalInfo;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// GLOBAL VARIABLES

GlobalInfo *globalInfo;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Only used for debugging purposes

#ifdef DEBUG_1
	typedef struct
	{
		ac_encoder	**aceHeader;
		ac_encoder	**aceScore;
		ac_encoder	**aceSNRows;
		ac_encoder	**aceSNCols;
		ac_encoder	**aceSourceName;
		ac_encoder	**aceSLines;
		ac_encoder	**aceCase;
		ac_encoder	**aceCaseFlag;
		ac_encoder	**aceXchr;
		ac_encoder	**aceXchrFlag;
		ac_encoder	**aceStartPosRaw;
		ac_encoder	**aceStartOffsetSign;
		ac_encoder	**aceStartOffset;
		ac_encoder	**aceStrand;
		ac_encoder	**aceSourceSize;
		
		ac_encoder	**aceQLines;
		ac_encoder	**aceQLineFlag;
		ac_encoder	**aceQLineInFlag;
		
		ac_encoder	**aceILineFlag;
		ac_encoder	**aceStatus;
		ac_encoder	**aceCounts;
		ac_encoder	**aceIrregularStatus;
		ac_encoder	**aceIrregularCounts;
		
		ac_encoder	**aceELineFlag;
		ac_encoder	**aceENRows;
		ac_encoder	**aceESourceName;
		ac_encoder	**aceEStartPosRaw;
		ac_encoder	**aceEStartOffsetSign;
		ac_encoder	**aceEStartOffset;
		ac_encoder	**aceESeqSize;
		ac_encoder	**aceEStrand;
		ac_encoder	**aceESourceSize;
		ac_encoder	**aceELineStatus;
		ac_encoder	**aceELineIrregularStatus;
				
		ac_encoder	**aceHashInfo;
	}
	AceDebuggers;

	typedef struct
	{
		uint64_t	totalBits;
		uint64_t	*headerBits;
		uint64_t	*scoreBits;
		uint64_t	*sNRowsBits;
		uint64_t	*sNColsBits;
		uint64_t	*sLinesBits;
		uint64_t	*sourceNameBits;
		uint64_t	*caseBits;
		uint64_t	*caseFlagBits;
		uint64_t	*xchrBits;
		uint64_t	*xchrFlagBits;
		uint64_t	*startPosRawBits;
		uint64_t	*startOffsetSignBits;
		uint64_t	*startOffsetBits;
		uint64_t	*strandBits;
		uint64_t	*sourceSizeBits;
		
		uint64_t	*qLinesBits;
		uint64_t	*qLineFlagBits;
		uint64_t	*qLineInFlagBits;
		
		uint64_t	*iLineFlagBits;
		uint64_t	*statusBits;
		uint64_t	*countBits;
		uint64_t	*irregularStatusBits;
		uint64_t	*irregularCountBits;
		
		uint64_t	*eLineFlagBits;
		uint64_t	*eNRowsBits;
		uint64_t	*eSourceNameBits;
		uint64_t	*eStartPosRawBits;
		uint64_t	*eStartOffsetSignBits;
		uint64_t	*eStartOffsetBits;
		uint64_t	*eSeqSizeBits;
		uint64_t	*eStrandBits;
		uint64_t	*eSourceSizeBits;		
		uint64_t	*eLineStatusBits;
		uint64_t	*eLineIrregularStatusBits;
				 
		uint64_t	*hashInfoBits;
	}
	DebugInfo;
		
	// GLOBAL VARIABLES FOR DEBUG MODE
	AceDebuggers 	*aceDebuggers;
	DebugInfo 		*debugInfo;
	
	// Function used to process the debug statistic information
	void 	CreateACEDebugger			(uint8_t);
	void 	FreeACEDebugger				(uint8_t);
	void 	ACEDebuggersDone			(uint8_t);
	void 	CreateDebugInfo				(uint8_t);
	void 	FreeDebugInfo				(uint8_t);
	void 	PrintDebugInfo				(uint8_t);
#endif // DEBUG_1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Local functions header definition
void 	ReadMAFPortion					(void *);
void 	ReadMAFHeaderLines				(FILE *, uint8_t, MSAB *, ACEncoder *, 
										ACModels *, FieldsLimits *, 
										StorageBitsInfo	*);
void	EndMAFReading					(FILE *, uint8_t, uint8_t, uint8_t, MSAB *, 
										StorageBitsInfo *, ACEncoder *, ACModels *, 
										CModels *, CTemplate *, FILE *, uint64_t);
void 	EndPartReading					(uint8_t, MSAB *, StorageBitsInfo *, 
										ACEncoder *, ACModels *, uint64_t);

void 	ReadSLine						(FILE *, uint8_t, MSAB *);
void 	ReadQLine						(FILE *, uint8_t, MSAB *);
void 	ReadILine						(FILE *, uint8_t, MSAB *);
void 	ReadELine						(FILE *, uint8_t, MSAB *, FieldsLimits *);

void	EncodeMSAB						(uint8_t, MSAB *, ACEncoder *, ACModels *, 
										CModels *, CTemplate *, FieldsLimits *, 
										StorageBitsInfo *);
void 	EncodeSLinesData				(uint8_t, MSAB *, ACEncoder *, ACModels *, 
										CModels *, CTemplate *, FieldsLimits *, 
										StorageBitsInfo *);
void 	EncodeQLinesData				(uint8_t, MSAB *, ACEncoder *, ACModels *, 
										CModels *);
void 	EncodeILinesData				(uint8_t, MSAB *, ACEncoder *, ACModels *, 
										CModels *, FieldsLimits *, 
										StorageBitsInfo *);
void	EncodeELinesData				(uint8_t, MSAB *, ACEncoder *, ACModels *, 
										CModels *, StorageBitsInfo *);
										
void	CreateGlobalInfo				(uint8_t);
void	FreeGlobalInfo					(uint8_t);
void 	ComputeGlobalInfo				(uint8_t);
void 	PrintGlobalInfo					(uint8_t);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main(int argc, char *argv[])
{
	char tmpOutDir[FILENAME_MAX]="";
	char outFileName[FILENAME_MAX]="mergedEncodedFile.dat";
	char outMSABInfoFileName[FILENAME_MAX]="MSAB-Sizes.info";
	UChar template = DEFAULT_TEMPLATE, *buffer;
	
	uint8_t help=0, hashFunction=HASH_FUNCTION; //,removeTmpFiles = 0x1;
	
	uint8_t maxNumberOfElements=HASH_MAX_ELEMENTS_PER_ENTRY; 
	uint8_t maxStartOffset = MAX_START_OFFSET;

	// Model order to encode the 's' lines (DNA bases and gaps) 
	uint8_t sLinesModelOrder=DEFAULT_S_LINES_MODEL_ORDER;			
	// Model order to encode the case information (lower or upper symbols)
	uint8_t caseModelOrder=DEFAULT_CASE_MODEL_ORDER;				
	// Model order to encode the case flag information (lower or upper symbols)
	uint8_t caseFlagModelOrder=DEFAULT_CASE_FLAG_MODEL_ORDER;		
	// Model order to encode the extra symbols (N's n's)
	uint8_t xchrModelOrder=DEFAULT_XCHAR_MODEL_ORDER;				
	// Model order to encode the extra symbols flag (N's n's)
	uint8_t xchrFlagModelOrder=DEFAULT_XCHAR_FLAG_MODEL_ORDER;		
	// Model order to encode the strand information
	uint8_t strandModelOrder=DEFAULT_STRAND_MODEL_ORDER;			
	// Model ordet to encode the start offset sign
	uint8_t startOffsetSignModelOrder = DEFAULT_START_OFFSET_SIGN_MODEL_ORDER;
	
	
	// Model order to encode the 'q' lines (quality values)
	uint8_t qLinesModelOrder=DEFAULT_Q_LINES_MODEL_ORDER;			
	// Model order to encode the presence of 'q' lines in the MSAB
	uint8_t qLineFlagModelOrder=DEFAULT_Q_LINE_FLAG_MODEL_ORDER;	
	// Model order to encode the position of each 'q' lines in the MSAB
	uint8_t qLineInFlagModelOrder=DEFAULT_Q_LINE_IN_FLAG_MODEL_ORDER;	
	
	// Model order to encode the 'i' lines presence
	uint8_t	iLineFlagModelOrder=DEFAULT_I_LINE_FLAG_MODEL_ORDER;	
	// Model order to encode the status symbol of 'i' and 'e' lines																
	uint8_t statusModelOrder=DEFAULT_STATUS_MODEL_ORDER;			
	// Model order to encode the irregular status symbol flag
	uint8_t	irregularStatusModelOrder = DEFAULT_IRREGULAR_STATUS_MODEL_ORDER;
	// Model order to encode the irregular counts flag
	uint8_t	irregularCountModelOrder = DEFAULT_IRREGULAR_COUNT_MODEL_ORDER;
	
	// Model order to encode the presence of 'e' lines 
	uint8_t eLineFlagModelOrder = DEFAULT_E_LINE_FLAG_MODEL_ORDER;
	// Model order to encode irregular status symbols
	uint8_t eLineIrregularStatusModelOrder = DEFAULT_E_LINE_IRREGULAR_STATUS_MODEL_ORDER;
			
	uint32_t n, nThreads=DEFAULT_NUMBER_OF_THREADS, hashTableSize=HASH_TABLE_SIZE; 
	uint32_t maxMSABNRows=MAX_MSAB_NROWS, maxMSABNCols=MAX_MSAB_NCOLS; 
	uint32_t maxHeaderLineSize = MAX_HEADER_LINE_SIZE;
	uint32_t rowBlockSize=DEFAULT_ROW_BLOCK_SIZE, maxLineSize = MAX_LINE_SIZE; 
	uint32_t maxSrcNameSize=MAX_SRC_NAME_SIZE, maxAbsScoreValue = MAX_ABSOLUTE_SCORE;
	uint32_t maxSourceSize = MAX_SRC_SIZE, maxStartPosition = MAX_START_POSITION;
	uint32_t maxCountValue = MAX_COUNT_VALUE;
	uint32_t bufferSize=BUF_SIZE;	
	uint32_t nGOBs=DEFAULT_NUMBER_OF_GOBS, nGOBsPerThread=0, remainingGOBs=0;

	#ifndef NULL_DEV
		uint32_t GOBId;
	#endif
		
	// DEBUG MODE
	#ifdef DEBUG_1
		uint32_t nRows=0, nCols=0;
		int32_t nRead;
	#endif
		
	uint64_t nTotalBytes; 
		
	pthread_t *threads;
	ThreadsData *threadsData;
	
	
	// NULL DEVICE MODE NOT ACTIVATED
	#ifndef NULL_DEV
		char tmp[FILENAME_MAX]="", tmpOutFileName[FILENAME_MAX]="";
		uint8_t removeTmpFiles = 0x1;
		uint32_t readBufferSize=0;
		FILE *outFp = NULL, *inFp = NULL;
		size_t tmpSize;
		// Get process ID
		pid_t pid = getpid();
	#endif
			
	for(n = 1; n != argc; ++n)
		if(!(Strcmp("-h", argv[n])))
			{ help = 1; break; }

	if(argc < 2 || help == 1)
	{
		
		fprintf(stderr, "\nUsage: %s [OPTIONS] ... [MAF FILE]\n\n", argv[0]);
		
		PrintStringOption("-h", "", "(print this help)");
		PrintStringOption("-v", "", "(print version number)");
		PrintStringOption("-o", "<outFileName>", "(encoded output file)");
		PrintStringOption("-O", "<temporary output dir>", "(this directory is used for the temporary files of each thread)");
		PrintStringOption("-inf", "<outFileName>", "(info file with the MSAB sizes)");
		PrintParameterIntegerOption("-nt", "<nThreads>", "(number of threads)", nThreads);
		PrintParameterCharOption("-t", "<template>", "(template type: 'A' - 'E')", template);
		PrintParameterIntegerOption("-ng", "<nGOBs>", "(number of group of MSABs)", nGOBs);
		
		fprintf(stderr, "\n");
		PrintParameterIntegerOption("-sm", "<sLinesModelOrder>", "(model order to encode the 's' lines bases)", sLinesModelOrder);
		PrintParameterIntegerOption("-cm", "<caseModelOrder>", "(model order to encode the case information: lower and upper)", caseModelOrder);
		PrintParameterIntegerOption("-cfm", "<caseFlagModelOrder>", "(model order to encode the case flag information)", caseFlagModelOrder);
		PrintParameterIntegerOption("-xm", "<xchrModelOrder>", "(model order to encode the extra symbols: N's n's)", xchrModelOrder);
		PrintParameterIntegerOption("-xfm", "<xchrFlagModelOrder>", "(model order to encode the extra symbol flag information)", xchrModelOrder);
		PrintParameterIntegerOption("-srm", "<strandModelOrder>", "(model order to encode the strand information)", strandModelOrder);
		PrintParameterIntegerOption("-som", "<startOffsetSignModelOrder>", "(model order to encode the start offset sign)", startOffsetSignModelOrder);
		
		PrintParameterIntegerOption("-qm", "<qLinesModelOrder>", "(model order to encode the 'q' lines)", qLinesModelOrder);
		PrintParameterIntegerOption("-qfm", "<qLineFlagModelOrder>", "(model order to encode the 'q' line flag)", qLineFlagModelOrder);
		PrintParameterIntegerOption("-qim", "<qLineInFlagModelOrder>", "(model order to encode the 'q' line inside flag)", qLineInFlagModelOrder);
		
		PrintParameterIntegerOption("-ifm", "<iLineFlagModelOrder>", "(model order to encode the 'i' line flag)", iLineFlagModelOrder);
		PrintParameterIntegerOption("-stm", "<statusModelOrder>", "(model order to encode the status symbols of 'i' and 'e' lines)", statusModelOrder);
		PrintParameterIntegerOption("-ism", "<irregularStatusModelOrder>", "(model order to encode the irregular status symbols flag of the 'i' lines)", irregularStatusModelOrder);
		PrintParameterIntegerOption("-icm", "<irregularCountModelOrder>", "(model order to encode the irregular counts values of 'i' lines)", irregularCountModelOrder);
		
		PrintParameterIntegerOption("-efm", "<eLineFlagModelOrder>", "(model order to encode the 'e' line flag)", eLineFlagModelOrder);
		PrintParameterIntegerOption("-esm", "<eLineIrregularStatusModelOrder>", "(model order to encode the irregular status symbols flag of the 'e' lines)", eLineIrregularStatusModelOrder);
				
		fprintf(stderr, "\n");
		PrintParameterIntegerOption("-mnr", "<maxNRows>", "(maximum number of rows allowed)", maxMSABNRows);
		PrintParameterIntegerOption("-mnc", "<maxMSABNCols>", "(maximum number of columns allowed)", maxMSABNCols);
		PrintParameterIntegerOption("-mhs", "<maxHeaderLineSize>", "(maximum number of characters allowed in header)", maxHeaderLineSize);
		PrintParameterIntegerOption("-mns", "<maxSrcNameSize>", "(maximum number of characters allowed in each source entry)", maxSrcNameSize);
		PrintParameterIntegerOption("-mas", "<maxAbsScoreValue>", "(maximum absolute score value allowed)", maxAbsScoreValue);
		PrintParameterIntegerOption("-mls", "<maxLineSize>", "(buffer size used in fgets function)", maxLineSize);
		PrintParameterIntegerOption("-mss", "<maxSourceSize>", "(maximum source/chromossome size allowed)", maxSourceSize);
		PrintParameterIntegerOption("-msp", "<maxStartPosition>", "(maximum start position allowed)", maxStartPosition);
		PrintParameterIntegerOption("-mso", "<maxStartOffset>", "(maximum start offset allowed)", maxStartOffset);
		PrintParameterIntegerOption("-mcv", "<maxCountValue>", "(maximum count value allowed)", maxCountValue);
		fprintf(stderr, "\n");
		PrintParameterIntegerOption("-hts", "<hashTableSize>", "(hash table size)", hashTableSize);
		PrintParameterIntegerOption("-hme", "<maxNumberOfElements>", "(hash maximum number of elements per entry)", maxNumberOfElements);
		PrintParameterIntegerOption("-hf", "<hashFunction>", "(hash function)", hashFunction);
		fprintf(stderr, "\n");
		PrintParameterIntegerOption("-rbs", "<rowBlockSize>", "(slot size used in the memory allocation)", rowBlockSize);
		PrintParameterIntegerOption("-bs", "<bufferSize>", "(buffer size used in the merge procedure)", bufferSize);
		PrintParameterStringOption("-rt", "<yes/no>", "(remove temporary files)", "yes");
		fprintf(stderr, "\n");
		
		return EXIT_FAILURE;
	}
	
	// Print version and copyrigth notice
	for(n = 1; n != argc; ++n)
	{
		if(!(Strcmp("-v", argv[n])))
		{
			printf(ENC_VERSION_NUMBER);
			printf(COPYRIGHT_NOTICE);	
			return EXIT_SUCCESS;
		}
	}
	
	
	// Output encoded file
	for(n = 1; n != argc; ++n)
	{
		if(!(Strcmp("-o", argv[n])))
		{
			//outFp = Fopen(argv[n+1], "wb");
			Strcpy(outFileName, argv[n+1], FILENAME_MAX);
			break;
		}
	}
	
	// Output MSAB Size Info file
	for(n = 1; n != argc; ++n)
	{
		if(!(Strcmp("-inf", argv[n])))
		{
			//outFp = Fopen(argv[n+1], "wb");
			Strcpy(outMSABInfoFileName, argv[n+1], FILENAME_MAX);
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
	
	// Read the template type to be used latter
	for(n = 1; n != argc; ++n)
	{
		if(!(Strcmp("-t", argv[n])))
		{
			template = (  ( (toupper(argv[n+1][0]) < 'A') || (toupper(argv[n+1][0]) > 'E') || (argv[n+1][0] < 0) ) ? 
				DEFAULT_TEMPLATE : (uint8_t)(argv[n+1][0]) );
			break;
		}
	}
	
	// Read the number of parts
	for(n = 1; n != argc; ++n)
	{
		if(!(Strcmp("-ng", argv[n])))
		{
			nGOBs = ( Atoui(argv[n+1]) > MAX_NUMBER_OF_GOBS ? MAX_NUMBER_OF_GOBS : Atoui(argv[n+1]) );
			break;
		}
	}
	
	/////////////////////////////////////////////////////// MODELS ORDER						
	// Get the Model order to encode the 's' lines
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-sm", argv[n]) == 0)
		{
			sLinesModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_S_LINES_MODEL_ORDER)) ? 
				DEFAULT_S_LINES_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the case information (lower and upper bases)
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-cfm", argv[n]) == 0)
		{
			caseModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_CASE_MODEL_ORDER)) ? 
				DEFAULT_CASE_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}

	// Get the Model order to encode the case flag information (lower and upper bases)
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-cm", argv[n]) == 0)
		{
			caseFlagModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_CASE_FLAG_MODEL_ORDER)) ? 
				DEFAULT_CASE_FLAG_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}

	// Get the Model order to encode extra symbols (N's and n's)
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-xm", argv[n]) == 0)
		{
			xchrModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_XCHAR_MODEL_ORDER)) ? 
				DEFAULT_XCHAR_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode extra symbol flag (N's and n's)
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-xfm", argv[n]) == 0)
		{
			xchrFlagModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_XCHAR_FLAG_MODEL_ORDER)) ? 
				DEFAULT_XCHAR_FLAG_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the strand information '-' and '+'
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-srm", argv[n]) == 0)
		{
			strandModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_STRAND_MODEL_ORDER)) ? 
				DEFAULT_STRAND_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the start offset sign
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-som", argv[n]) == 0)
		{
			startOffsetSignModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_START_OFFSET_SIGN_MODEL_ORDER)) ? 
				DEFAULT_START_OFFSET_SIGN_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the 'q' lines (quality values)
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-qm", argv[n]) == 0)
		{
			qLinesModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_Q_LINES_MODEL_ORDER)) ? 
				DEFAULT_Q_LINES_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the 'q' lines presence in each MSAB
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-qfm", argv[n]) == 0)
		{
			qLineFlagModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_Q_LINE_FLAG_MODEL_ORDER)) ? 
				DEFAULT_Q_LINE_FLAG_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}

	// Get the Model order to encode the 'q' lines presence in each MSAB
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-qim", argv[n]) == 0)
		{
			qLineInFlagModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_Q_LINE_IN_FLAG_MODEL_ORDER)) ? 
				DEFAULT_Q_LINE_IN_FLAG_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the 'i' lines presence in each MSAB
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-ifm", argv[n]) == 0)
		{
			iLineFlagModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_I_LINE_FLAG_MODEL_ORDER)) ? 
				DEFAULT_I_LINE_FLAG_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the status symbols of 'i' and 'e' lines
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-stm", argv[n]) == 0)
		{
			statusModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_STATUS_MODEL_ORDER)) ? 
				DEFAULT_STATUS_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the irregular status symbols flag, of 'i' lines
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-ism", argv[n]) == 0)
		{
			irregularStatusModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_IRREGULAR_STATUS_MODEL_ORDER)) ? 
				DEFAULT_IRREGULAR_STATUS_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the irregular count value flag, of 'i' lines
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-icm", argv[n]) == 0)
		{
			irregularCountModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_IRREGULAR_COUNT_MODEL_ORDER)) ? 
				DEFAULT_IRREGULAR_COUNT_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the 'e' lines presence in each MSAB
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-efm", argv[n]) == 0)
		{
			eLineFlagModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_E_LINE_FLAG_MODEL_ORDER)) ? 
				DEFAULT_E_LINE_FLAG_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	// Get the Model order to encode the irregular status symbols flag, of 'e' lines
	for(n = 1; n < argc-1; ++n)
	{
		if(Strcmp("-esm", argv[n]) == 0)
		{
			eLineIrregularStatusModelOrder = ( ((Atoui(argv[n+1]) == 0) || (Atoui(argv[n+1]) > MAX_E_LINE_IRREGULAR_STATUS_MODEL_ORDER)) ? 
				DEFAULT_E_LINE_IRREGULAR_STATUS_MODEL_ORDER : ((uint8_t)Atoui(argv[n+1])) );
			break; 
		}
	}
	
	/////////////////////////////////////////////////////// MODELS ORDER						
	
	// Set the maximum number of rows allowed in each MSAB
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-mnr", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				maxMSABNRows = Atoui(argv[n+1]);
			break; 
		}
	}
	
	// Set the maximum number of columns allowed in each MSAB
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-mnc", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				maxMSABNCols = Atoui(argv[n+1]);
			break;
		}
	}
	
	// Set the maximum absolute score value
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-mas", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				maxAbsScoreValue = Atoui(argv[n+1]);
			break;
		}
	}
	
	// Set the maximum header line size 
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-mhs", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				maxHeaderLineSize = Atoui(argv[n+1]);
			break; 
		}
	}

	// Set the buffer size used in the fgets function
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-mls", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				maxLineSize = Atoui(argv[n+1]);
			break; 
		}
	}
	
	// Set the buffer size used in the fgets function for soruce name
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-mns", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				maxSrcNameSize = Atoui(argv[n+1]);
			break; 
		}
	}
	
	// Set the maximum value allowed to the source/chromossome size
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-mss", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				maxSourceSize = Atoui(argv[n+1]);
			break; 
		}
	}
	
	// Set the maximum start position
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-msp", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				maxStartPosition = Atoui(argv[n+1]);
			break; 
		}
	}
	
	// Set the maximum start offset
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-mso", argv[n]) == 0)
		{
			if( (Atoui(argv[n+1]) != 0) && (Atoui(argv[n+1]) <= UINT8_MAX) )
				maxStartOffset = Atoui(argv[n+1]);
			break; 
		}
	}

	// Set the maximum count value allowed
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-mcv", argv[n]) == 0)
		{
			if( (Atoui(argv[n+1]) != 0) && (Atoui(argv[n+1]) <= UINT32_MAX) )
				maxCountValue = Atoui(argv[n+1]);
			break; 
		}
	}
			
	// Set the slot size used in the memory allocation in the MSAB
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-rbs", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				rowBlockSize = Atoui(argv[n+1]);
			break; 
		}
	}
	
	
	
	// Set the hash table size
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-hts", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) > 1)
				hashTableSize = Atoui(argv[n+1]);
			break; 
		}
	}
	
	// Set the hash maximum elements per entry
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-hme", argv[n]) == 0)
		{
			if( (Atoui(argv[n+1]) > 1) && (Atoui(argv[n+1]) < HASH_MAX_ELEMENTS_PER_ENTRY) )
				maxNumberOfElements = (uint8_t)Atoui(argv[n+1]);
			break; 
		}
	}
	
	// Set the hash function to use
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-hf", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) <= UINT8_MAX)
				hashFunction = (uint8_t)Atoui(argv[n+1]);
			break; 
		}
	}
	
	// Set the buffer size used in the merge procedure
	// Merge the encoded files into a single file
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-bs", argv[n]) == 0)
		{
			if(Atoui(argv[n+1]) != 0)
				bufferSize = Atoui(argv[n+1]);
			break; 
		}
	}
	
	// Default output temporary directory
	for(n = 1; n != argc-1; ++n)
	{
		if(Strcmp("-O", argv[n]) == 0)
		{
			// Windows operating system	
			//#ifdef OS_WINDOWS
			#if defined(OS_WINDOWS) || defined(WIN32) || defined(MSDOS)	
				if(argv[n+1][Strlen(argv[n+1])-1] == '\\')
				{
					// Copy string
					Strcpy(tmpOutDir, argv[n+1], FILENAME_MAX);
					//Strcat(tmpOutDir, argv[n+1], FILENAME_MAX);
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
					//Strcat(tmpOutDir, argv[n+1], FILENAME_MAX);
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
	
	// NULL DEVICE MODE NOT ACTIVATED
	#ifndef NULL_DEV
		// Remove temporary files flag
		for(n = 1; n != argc-1; ++n)
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
	#endif
		
	// If the number of threads ih higher than the number of parts
	// it is necessary to reduce the number of threads
	if(nThreads > nGOBs)
	{
		printf("Number of threads was reduced from %"PRIu32" to %"PRIu32"\n", nThreads, nGOBs);
		printf("The optimal distribution is one thread for each one of the %"PRIu32" Group Of Blocks (GOBs).\n", nGOBs);
		nThreads = nGOBs;
	}
	
	// This is an integer division division
	nGOBsPerThread = nGOBs/nThreads;
	// Compute the remaing part to ditribute
	remainingGOBs=nGOBs-(nGOBsPerThread*nThreads);
	
	// Allocate memory for threads global variable
	CreateGlobalInfo(nThreads);
	
	// Buffer to be used in the merge process
	buffer = (UChar *)Calloc(bufferSize, sizeof(UChar));
		
	// Threads memory allocation
	threads = (pthread_t *)Calloc(nThreads, sizeof(pthread_t));
	threadsData = (ThreadsData *)Calloc(nThreads, sizeof(ThreadsData));
	
	
	// DEBUG MODE
	#ifdef DEBUG_1
		CreateACEDebugger(nThreads);
		CreateDebugInfo(nThreads);
	#endif // DEBUG_1	
	
	// Get the number of bytes of the input file
	nTotalBytes = GetNumberOfBytesInFile(argv[argc-1]);
	printf("File '%s' has ", argv[argc-1]);
	PrintHumanReadableBytes(nTotalBytes);
	printf(" (%"PRIu64" bytes).\n", nTotalBytes);
	
	// Fill thread data that will be passed as a parameter when calling the 
	// thread function
	for(n=0; n != nThreads; ++n)
	{
		threadsData[n].threadNumber = n;
		threadsData[n].nThreads = nThreads;
		threadsData[n].nTotalBytes = nTotalBytes;
		threadsData[n].rowBlockSize = rowBlockSize;
		threadsData[n].template = template;
		Strcpy(threadsData[n].inputFile, argv[argc-1], FILENAME_MAX);
		
		// Set the number of GOBs
		threadsData[n].totalNumberOfGOBs = nGOBs;
		// Set the number of GOBs for this thread
		// The remaining parts will be added for the first threads
		threadsData[n].nGOBs = nGOBsPerThread + (n < remainingGOBs ? (1) : (0));
		
		// For the first theard we have:
		if(n == 0)
		{
			threadsData[n].firstGOBId = 0;
		}
		else
		{
			// Get the firt part ID of the previous thread and add
			// the number of parts of the previous thread
			threadsData[n].firstGOBId = threadsData[n-1].firstGOBId + threadsData[n-1].nGOBs;
		}
		
		// Output the compress stream to /dev/null
		#ifdef NULL_DEV
			Strcpy(threadsData[n].tmpOutEncFile, "/dev/null", FILENAME_MAX);
		#else
			// Output file name for each thread
			tmpSize=snprintf(tmpOutFileName, FILENAME_MAX, "%sPID_%05"PRId32"-THREAD_%02"PRIu8"", DEFAULT_TMP_ENC_FILE, (int32_t)pid, n);
			if(tmpSize >= FILENAME_MAX)
			{
				fprintf(stderr, "Error (main): error when trying to write formatted output to sized buffer.\n");
				fprintf(stderr, "Error (main): sprintf function tried to wrote %"PRIu64" characters + '\\0'.\n", (uint64_t)tmpSize);
				fprintf(stderr, "Error (main): buffer max size = %"PRIu32".\n", (uint32_t)FILENAME_MAX);
				return EXIT_FAILURE;
			}
			
			// Initialize temporary string with an emtpy string
			Strcpy(tmp, "", 1);
			// Append path
			Strcat(tmp, tmpOutDir, FILENAME_MAX);
			// Append file name
			Strcat(tmp, tmpOutFileName, FILENAME_MAX);
			// Copy full path to be used in each thread
			Strcpy(threadsData[n].tmpOutEncFile, tmp, FILENAME_MAX);
		
		#endif // NULL_DEV
		
		// DEBUG MODE
		#ifdef DEBUG_1
			// Temporary file for writing the MSAB sizes
			Strcpy(tmpOutFileName, "", 1);
			tmpSize=snprintf(tmpOutFileName, FILENAME_MAX, "%sPID_%05"PRId32"-THREAD_%02"PRIu8".info", DEFAULT_MSAB_INFO_FILE, (int32_t)pid, n);
			if(tmpSize >= FILENAME_MAX)
			{
				fprintf(stderr, "Error (main): error when trying to write formatted output to sized buffer.\n");
				fprintf(stderr, "Error (main): sprintf function tried to wrote %"PRIu64" characters + '\\0'.\n", (uint64_t)tmpSize);
				fprintf(stderr, "Error (main): buffer max size = %"PRIu32".\n", (uint32_t)FILENAME_MAX);
				return EXIT_FAILURE;
			}
			
			// Initialize temporary string with an emtpy string
			Strcpy(tmp, "", 1);
			// Append path
			Strcat(tmp, tmpOutDir, FILENAME_MAX);
			// Append file name
			Strcat(tmp, tmpOutFileName, FILENAME_MAX);
			// Copy full path to be used in each thread
			Strcpy(threadsData[n].tmpMSABInfoFile, tmp, FILENAME_MAX);
			
		#else
			// File not used
			Strcpy(threadsData[n].tmpMSABInfoFile, "", 1);
		#endif // DEBUG_1	
			
		threadsData[n].fieldsLimits = (FieldsLimits *)Calloc(1, sizeof(FieldsLimits));
		threadsData[n].storageBitsInfo = (StorageBitsInfo *)Calloc(1, sizeof(StorageBitsInfo));
		threadsData[n].modelsOrder = (ModelsOrder *)Calloc(1, sizeof(ModelsOrder));
		threadsData[n].hashTableInfo = (HashTableInfo *)Calloc(1, sizeof(HashTableInfo));
		
		threadsData[n].fieldsLimits->maxMSABNRows = maxMSABNRows;
		threadsData[n].fieldsLimits->maxMSABNCols = maxMSABNCols;
		threadsData[n].fieldsLimits->maxAbsScoreValue = maxAbsScoreValue;
		threadsData[n].fieldsLimits->maxHeaderLineSize = maxHeaderLineSize;
		threadsData[n].fieldsLimits->maxLineSize = maxLineSize;
		threadsData[n].fieldsLimits->maxSrcNameSize = maxSrcNameSize;
		threadsData[n].fieldsLimits->maxSourceSize = maxSourceSize;
		threadsData[n].fieldsLimits->maxStartPosition = maxStartPosition;
		threadsData[n].fieldsLimits->maxStartOffset = maxStartOffset;
		threadsData[n].fieldsLimits->maxCountValue = maxCountValue;
			
		threadsData[n].storageBitsInfo->storageBitsMSABRows = GetNumberOfBits(maxMSABNRows);
		threadsData[n].storageBitsInfo->storageBitsMSABCols = GetNumberOfBits(maxMSABNCols);
		threadsData[n].storageBitsInfo->storateBitsAbsScoreValue = GetNumberOfBits(maxAbsScoreValue);
		threadsData[n].storageBitsInfo->storageBitsHeaderLine = GetNumberOfBits(maxHeaderLineSize);
		threadsData[n].storageBitsInfo->storageBitsSrcName = GetNumberOfBits(maxSrcNameSize);
		threadsData[n].storageBitsInfo->storageBitsSourceSize = GetNumberOfBits(maxSourceSize);
		threadsData[n].storageBitsInfo->storageBitsStartPosition = GetNumberOfBits(maxStartPosition);
		threadsData[n].storageBitsInfo->storageBitsStartOffset = GetNumberOfBits(maxStartOffset);
		threadsData[n].storageBitsInfo->storageBitsCountValue = GetNumberOfBits(maxCountValue);
				
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

	// Create and launch each thread
	for(n = 0; n != nThreads; ++n)
	{
		//pthread_create (&threads[n], NULL, (void *) &readMAFPart, (void *)&data[n]);
		pthread_create (&threads[n], NULL, (void *) &ReadMAFPortion, (void *)&threadsData[n]);
		//pthread_create (&threads[n], NULL, (void *) &ReadMAFPortionV2, (void *)&threadsData[n]);
		printf("Thread %"PRIu8" started.\n", n);
	}
	
	
	// Waits for all threads to finish
	for(n = 0; n != nThreads; ++n)
	{
		pthread_join(threads[n], NULL);
		printf("Thread %"PRIu8" ended.\n", n);
	}	
		
	
	// Output the compress stream to /dev/null
	#ifdef NULL_DEV
		printf("The encoded stream was redirected to /dev/null.\n");
	#else
		// Open file for append the binary files of each thread
		outFp = Fopen(outFileName, "wb");
	
	
		// Save header information required in the decoder required to decode the encoded file
		//Fwrite(&nThreads, sizeof(uint8_t), 1, outFp);
		Fwrite(&nGOBs, sizeof(uint32_t), 1, outFp);
		Fwrite(&template, sizeof(uint8_t), 1, outFp);
		Fwrite(&(globalInfo->numberOfHeaderLines), sizeof(uint8_t), 1, outFp);
		Fwrite(&maxHeaderLineSize, sizeof(uint32_t), 1, outFp);
		Fwrite(&maxSrcNameSize, sizeof(uint32_t), 1, outFp);
			
		Fwrite(&sLinesModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&caseModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&caseFlagModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&xchrModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&xchrFlagModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&strandModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&startOffsetSignModelOrder, sizeof(uint8_t), 1, outFp);
	
		Fwrite(&qLinesModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&qLineFlagModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&qLineInFlagModelOrder, sizeof(uint8_t), 1, outFp);
	
		Fwrite(&iLineFlagModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&statusModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&irregularStatusModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&irregularCountModelOrder, sizeof(uint8_t), 1, outFp);
	
		Fwrite(&eLineFlagModelOrder, sizeof(uint8_t), 1, outFp);
		Fwrite(&eLineIrregularStatusModelOrder, sizeof(uint8_t), 1, outFp);
	
	
		// Save the hash information 
		Fwrite(&hashTableSize, sizeof(uint32_t), 1, outFp);
		Fwrite(&hashFunction, sizeof(uint8_t), 1, outFp);
		Fwrite(&maxNumberOfElements, sizeof(uint8_t), 1, outFp);
	
		// Save the storage bits information 
		Fwrite(&(threadsData[0].storageBitsInfo->storageBitsMSABRows), sizeof(uint16_t), 1, outFp);
		Fwrite(&(threadsData[0].storageBitsInfo->storageBitsMSABCols), sizeof(uint16_t), 1, outFp);
		Fwrite(&(threadsData[0].storageBitsInfo->storateBitsAbsScoreValue), sizeof(uint16_t), 1, outFp);
		Fwrite(&(threadsData[0].storageBitsInfo->storageBitsHeaderLine), sizeof(uint16_t), 1, outFp);
		Fwrite(&(threadsData[0].storageBitsInfo->storageBitsSrcName), sizeof(uint16_t), 1, outFp);
		Fwrite(&(threadsData[0].storageBitsInfo->storageBitsSourceSize), sizeof(uint16_t), 1, outFp);
		Fwrite(&(threadsData[0].storageBitsInfo->storageBitsStartPosition), sizeof(uint16_t), 1, outFp);
		Fwrite(&(threadsData[0].storageBitsInfo->storageBitsStartOffset), sizeof(uint16_t), 1, outFp);
		Fwrite(&(threadsData[0].storageBitsInfo->storageBitsCountValue), sizeof(uint16_t), 1, outFp);
	
		// Save the file size of each binary file that will be merged
		// not in the next loop but in the next to this one
		for(n = 0; n != nThreads; ++n)
		{
			for(GOBId = 0; GOBId != threadsData[n].nGOBs; ++GOBId)
			{
				tmpSize=snprintf(tmpOutFileName, FILENAME_MAX, "%s-PART_%04"PRIu32"_OUTOF_%04"PRIu32".dat", threadsData[n].tmpOutEncFile, GOBId+1, threadsData[n].nGOBs);	
				if(tmpSize >= FILENAME_MAX)
				{
					fprintf(stderr, "Error (main): error when trying to write formatted output to sized buffer.\n");
					fprintf(stderr, "Error (main): sprintf function tried to wrote %"PRIu64" characters + '\\0'.\n", (uint64_t)tmpSize);
					fprintf(stderr, "Error (main): buffer max size = %"PRIu32".\n", (uint32_t)FILENAME_MAX);
					return EXIT_FAILURE;
				}
			
			
				// Get the number of bytes of each binary file produced by 
				// each thread
				//nTotalBytes = GetNumberOfBytesInFile(threadsData[n].tmpOutEncFile);
				nTotalBytes = GetNumberOfBytesInFile(tmpOutFileName);
				// Write this information in the output file
				Fwrite(&nTotalBytes, sizeof(uint64_t), 1, outFp);
				//printf("File %s has %"PRIu64" bytes.\n", threadsData[n].tmpOutEncFile, nTotalBytes);
				printf("File %s has %"PRIu64" bytes.\n", tmpOutFileName, nTotalBytes);
			}
		}	
		
		printf("\n");
		
		// Merging encoded files
		for(n = 0; n != nThreads; ++n)
		{
			
			for(GOBId = 0; GOBId != threadsData[n].nGOBs; ++GOBId)
			{
				tmpSize=snprintf(tmpOutFileName, FILENAME_MAX, "%s-PART_%04"PRIu32"_OUTOF_%04"PRIu32".dat", threadsData[n].tmpOutEncFile, GOBId+1, threadsData[n].nGOBs);	
				if(tmpSize >= FILENAME_MAX)
				{
					fprintf(stderr, "Error (main): error when trying to write formatted output to sized buffer.\n");
					fprintf(stderr, "Error (main): sprintf function tried to wrote %"PRIu64" characters + '\\0'.\n", (uint64_t)tmpSize);
					fprintf(stderr, "Error (main): buffer max size = %"PRIu32".\n", (uint32_t)FILENAME_MAX);
					return EXIT_FAILURE;
				}
			
			
				// Open encoded file
				//inFp = Fopen(threadsData[n].tmpOutEncFile, "rb");
				inFp = Fopen(tmpOutFileName, "rb");
		
				// Read an block of size bufferSize to the variable "buffer"
				//while ((readBufferSize = fread(buffer, 1, bufferSize, inFp))) 
				while ((readBufferSize = Fread(buffer, 1, bufferSize, inFp))) 
				{
					// Write the binary information from the encoded thread file
					// to the output file
					//fwrite(buffer, 1, readBufferSize, outFp);
					Fwrite(buffer, 1, readBufferSize, outFp);
				}
		
				//printf("File %s appended!\n", threadsData[n].tmpOutEncFile);
				printf("File %s appended!\n", tmpOutFileName);
		
				// Close the input file
				Fclose(inFp);
		
				// Remove temporary files
				if(removeTmpFiles == 0x1)
				{
					// Remove temporary file created by the thread
					Remove(tmpOutFileName);
				}
			}
		}
		// Close the output file
		Fclose(outFp);
		
	#endif // NULL_DEV
	
	
	// Compute and print some statistics
	ComputeGlobalInfo(nThreads);
	PrintGlobalInfo(nThreads);
	
	// DEBUG MODE
	#ifdef DEBUG_1
		ACEDebuggersDone(nThreads);
		PrintDebugInfo(nThreads);
		
		// File that will contain the MSAB sizes
		outFp = Fopen(outMSABInfoFileName,"w");

		// Join the MSAB sizes info read into a single file
		for(n = 0; n != nThreads; ++n)
		{
			// Open the text file
			inFp = Fopen(threadsData[n].tmpMSABInfoFile, "r");
			while( (nRead=fscanf(inFp, "%"SCNu32"%"SCNu32"\n", &nRows, &nCols)) != EOF )
			{
				if(nRead != 2)
				{
					fprintf(stderr,"Error (main): unformatted line read!\n");
					return EXIT_FAILURE;
				}
				else
				{
					fprintf(outFp, "%10"PRIu32"%10"PRIu32"\n", nRows, nCols);
				}
			}
			Fclose(inFp);
			
			// Remove tmp file
			Remove(threadsData[n].tmpMSABInfoFile);
		}
		Fclose(outFp);
		
	#endif // DEBUG_1
	
	// Free buffer
	Free(buffer, bufferSize * sizeof(UChar));
	
	
	printf("\n");
	printf("Total bits of the encoded file: %19"PRIu64" bits\n", globalInfo->totalBits[nThreads]);
	
	// NULL DEVICE MODE NOT ACTIVATED
	#ifndef NULL_DEV
		// Get the number of bytes of the merged output file
		nTotalBytes = GetNumberOfBytesInFile(outFileName);
		printf("File '%s' has ", outFileName);
		PrintHumanReadableBytes(nTotalBytes);
		printf(" (%"PRIu64" bytes | %"PRIu64" bits).\n", nTotalBytes, nTotalBytes*8);
	#endif
		
	// Free memory
	Free(threads, nThreads*sizeof(pthread_t));
	for(n=0; n != nThreads; ++n)
	{
		Free(threadsData[n].fieldsLimits, sizeof(FieldsLimits));
		Free(threadsData[n].storageBitsInfo, sizeof(StorageBitsInfo));
		Free(threadsData[n].modelsOrder, sizeof(ModelsOrder));
		Free(threadsData[n].hashTableInfo, sizeof(HashTableInfo));
	}
	Free(threadsData, nThreads*sizeof(ThreadsData));
	FreeGlobalInfo(nThreads);
	
	// DEBUG MODE
	#ifdef DEBUG_1
		FreeACEDebugger(nThreads);
		FreeDebugInfo(nThreads);
	#endif // DEBUG_1
	
	
	if(nThreads == 1)
	{
		printf("Total memory in use: "); PrintHumanReadableBytes(TotalMemory());
		printf(" (%"PRIu64" bytes)\n", TotalMemory());
	}
	printf("\n");
			
	return EXIT_SUCCESS;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ReadMAFPortion(void *ptr)
{
	ThreadsData *threadsData;
	// Type cast to a pointer of ThreadData type            
	threadsData = (ThreadsData *)ptr;  
	
	char outFileName[FILENAME_MAX]="";
	char inputFile[FILENAME_MAX]; 
	//char *line; 
	UChar template;
	
	int8_t c, threadNumber, nThreads, skip = 0x0;
	
	uint32_t maxMSABNRows, maxMSABNCols; 
	uint32_t rowBlockSize, maxLineSize; 
	uint32_t GOBId;
	
	uint64_t nTotalBytes, masterBlockSize, nextMasterBlockOffset;
	uint64_t totalBlocks=0;
	
	off_t offset=0;
	size_t tmpSize=0;
	
	FILE *inFp, *outMSABInfoFile = NULL;
	ACModels *acModels;
	ACEncoder *acEncoder=NULL;
	CModels *cModels;
	MSAB *msab;
	CTemplate *cTemplate;
	
	threadNumber = threadsData->threadNumber;
	nThreads = threadsData->nThreads;
	nTotalBytes = threadsData->nTotalBytes;
	rowBlockSize = threadsData->rowBlockSize;
	template = threadsData->template;
	
	
	Strcpy(inputFile, threadsData->inputFile, FILENAME_MAX);
	
	maxMSABNRows = threadsData->fieldsLimits->maxMSABNRows;
	maxMSABNCols = threadsData->fieldsLimits->maxMSABNCols;
	maxLineSize = threadsData->fieldsLimits->maxLineSize;
	
	msab = CreateMSAB(rowBlockSize, threadsData->modelsOrder, threadsData->hashTableInfo, maxLineSize);
	cTemplate = InitTemplate(template, threadsData->modelsOrder->sLinesModelOrder);
	
	// Show template used to encode the 's' lines
	if(nThreads == 1)
		ShowTemplate(cTemplate);
	
	//line = (char *)Calloc(maxLineSize, sizeof(char));		

	
	// DEBUG MODE
	#ifdef DEBUG_1
		outMSABInfoFile = Fopen(threadsData->tmpMSABInfoFile,"w");
	#endif //DEBUG MODE
	
	
	// Open input MAF file for reading mode
	inFp = Fopen(inputFile, "r");
	
	// The follwoing structures need to be created for the scratch for each part
	acModels = CreateACModels();
	//acEncoder = CreateACEncoder(threadsData->tmpOutEncFile);
	//acEncoder = CreateACEncoder(outFileName);
	cModels = CreateCModels(threadsData->modelsOrder);
	
	// Loop all parts associated to the current thread
	for(GOBId = 0; GOBId < threadsData->nGOBs; GOBId++)
	{
		//printf("--------------------------------------------------\n");
		//printf("Thread %"PRIu32" | Part %"PRIu32"\n", threadNumber, partId);
		skip = 0x0;
		totalBlocks = 0;
		
		// Encoded file name
		tmpSize=snprintf(outFileName, FILENAME_MAX, "%s-PART_%04"PRIu32"_OUTOF_%04"PRIu32".dat", threadsData->tmpOutEncFile, GOBId+1, threadsData->nGOBs);
		if(tmpSize >= FILENAME_MAX)
		{
			fprintf(stderr, "Error (ReadMAFPortion)[Thread %02"PRIu8"]: error when trying to write formatted output to sized buffer.\n", threadNumber);
			fprintf(stderr, "Error (ReadMAFPortion)[Thread %02"PRIu8"]: sprintf function tried to wrote %"PRIu64" characters + '\\0'.\n", threadNumber, (uint64_t)tmpSize);
			fprintf(stderr, "Error (ReadMAFPortion)[Thread %02"PRIu8"]: buffer max size = %"PRIu32".\n", threadNumber, (uint32_t)FILENAME_MAX);
			pthread_exit(NULL);
		}	
		
		// If the first part was already encoded, it is necessary to reset the model counters
		// for the other parts
		if(GOBId != 0)
		{
			//printf("Reset models!!!!!\n");
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
			
			// Resume the ACEncoder using a different output file
			ResumeACEncoder(acEncoder, outFileName);
		}
		else
		{
			// For the first part it is necessary to allocate memory
			// And initialize the ACEncoder
			acEncoder = CreateACEncoder(outFileName);
		}
		
		
		// Compute the offsets to use in the fseeko function
		masterBlockSize = nTotalBytes/threadsData->totalNumberOfGOBs;
		offset = masterBlockSize * (threadsData->firstGOBId + GOBId);
		
		nextMasterBlockOffset = offset+masterBlockSize;
	
		// Change the file pointer to offset position
		Fseeko(inFp, offset, SEEK_SET);
	
		// If we are dealing with the first master block then we need to read the header info
		// The header lines are the first lines of the MAF file that starts with '#'
		if(threadNumber==0 && GOBId == 0) 
		{
			ReadMAFHeaderLines(inFp, threadNumber, msab, acEncoder, acModels, threadsData->fieldsLimits, threadsData->storageBitsInfo);
		}
		
		// If not, the fseeko operation could set the file pointer inside a MSAB so we need to 
		// search for the beggining of the next MSAB
		else
		{
			// Read lines until find an "a score=<score value>" line
			do
			{
				if(!fgets(msab->line, maxLineSize, inFp))
				{
					fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: unexpected end-of-file\n", threadNumber);
					pthread_exit(NULL);
				}
			} 
			// It is necessary two condition here in order to make sure that we are dealing with a
			// "a score" line. After the previous fseeko operation, the file pointer could be
			// point at a position of an 'a' base so we need this two condition in the loop.
			while(msab->line[0] != 'a' || msab->line[1] != ' ');
		
			// The variable "line" contains the score information that we need to extract
			if(sscanf(msab->line,"a score=%lf", &msab->score) != 1)
			{
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: failed to get alignment score line!\n", threadNumber);
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Line read           : '%s'", threadNumber, msab->line);
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Line format expected: a score=###...#.000000\n", threadNumber);
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Current file position: byte %"PRIu64"\n", threadNumber, Ftello(inFp));
				pthread_exit(NULL);
			}	
		
			// An extra control in order to avoid the current thread to reach out the 
			// MSAB of the next thread
			if(Ftello(inFp) >= nextMasterBlockOffset)	
			{
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: will process information of Thread %02"PRIu8"!\n", threadNumber, threadNumber+1);
				fprintf(stderr, "The motive for this error is the higher number of thread and/or the small size of the input file.\n");
				fprintf(stderr, "Please reduce the number of threads using the '-nt' parameter.\n");
				pthread_exit(NULL);
			}
		
		}
	
		// Free memory allocated for buffer of fgets
		// Free(line, maxLineSize*sizeof(char));
	
		// After this the program is in the begining of a MSAB. The score information was already read 
		// before so the encoder just need to start reading the data of the current MSAB (s, e, q, and i lines)
		//while((c = fgetc(inFp)) != EOF)
		while( (skip == 0x0) && ((c = fgetc(inFp)) != EOF) )
		{
			//skip = 0x0;
			
			switch(c)
			{
				////////////////////////// S LINES READING START
				// We are in the presence of a 's' line, so we need to read the line
				case 's':
					//printf("Read s line...\n");
					ReadSLine(inFp, threadNumber, msab);	
					break;
				////////////////////////// S LINES READING END
				
				////////////////////////// Q LINES READING START
				case 'q':
					//printf("Read q line...\n");
					ReadQLine(inFp, threadNumber, msab);
					break;
				////////////////////////// Q LINES READING END
				
				////////////////////////// I LINES READING START
				case 'i':
					//printf("Read i line...\n");
					ReadILine(inFp, threadNumber, msab);
					break;
				////////////////////////// I LINES READING END
				
				////////////////////////// E LINES READING START
				case 'e':
					//printf("Read e line...\n");
					ReadELine(inFp, threadNumber, msab, threadsData->fieldsLimits);
					break;
				////////////////////////// E LINES READING END
				
				// The encoder already read all the lines of the current MSAB
				case 'a':
				case '#':			
				case '\n':
					if(msab->sLinesData->nRows > 0 && msab->sLinesData->nCols > 0)
					{
						// Verify the number of rows of the current MSAB is bigger than maxMSABNRows 
						if(msab->sLinesData->nRows > maxMSABNRows)
						{
							fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: The block %"PRIu64" is to big!", threadNumber, totalBlocks);
							fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Block has two many rows (%"PRIu32" rows).\n", threadNumber, msab->sLinesData->nRows);
							fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Max number of rows allowed is: %"PRIu32".\n", threadNumber, maxMSABNRows);
							fprintf(stderr, "Rerun the encoder using an higher '-nr' paramater value ( > %"PRIu32").\n", msab->sLinesData->nRows);
							pthread_exit(NULL);
						}
					
						// Verify the number of columns of the current MSAB
						if(msab->sLinesData->nCols > maxMSABNCols)
						{
							fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: The block %"PRIu64" is to big!\n", threadNumber, totalBlocks);
							fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Block has too many columns (%"PRIu32" columns).\n", threadNumber, msab->sLinesData->nCols);
							fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Max number of columns allowed is: %"PRIu32".\n", threadNumber, maxMSABNCols);
							fprintf(stderr, "Rerun the encoder using an higher '-nc' paramater value ( > %"PRIu32").\n", msab->sLinesData->nCols);						
							pthread_exit(NULL);
						}
					
						// DEBUG MODE
						#ifdef DEBUG_1
							fprintf(outMSABInfoFile, "%10"PRIu32" %10"PRIu32"\n", msab->sLinesData->nRows, msab->sLinesData->nCols);
						#endif
						
						// Compression of the MSAB 
						EncodeMSAB(threadNumber, msab, acEncoder, acModels, cModels, cTemplate, threadsData->fieldsLimits, threadsData->storageBitsInfo);
					
						// Save some statistics
						totalBlocks++;
						globalInfo->totalBlocks[threadNumber]++;
						globalInfo->totalBases[threadNumber] += (msab->sLinesData->nRows * msab->sLinesData->nCols);
					
						ResetMSAB(msab);
					
						// If we reached the upper limit byte (next master block offset), the encoder ends the reading procedure
						if(Ftello(inFp) >= nextMasterBlockOffset)
						{
							// If this is the last part, the compression is terminated
							if(GOBId == threadsData->nGOBs-1)
							{
								EndMAFReading(inFp, threadNumber, 0x0, ((threadNumber==nThreads-1) ? 0x1 : 0x0), msab, threadsData->storageBitsInfo, acEncoder, acModels, cModels, cTemplate, outMSABInfoFile, totalBlocks);
							}
							else
							{
								// There are still part to read but we need to finish the reading
								// process for this part
								EndPartReading(threadNumber, msab, threadsData->storageBitsInfo, acEncoder, acModels, totalBlocks);
								// Skips the remaing code of the loop to read the next part
								//c = EOF;
								skip = 0x1;
								//continue;
								break;
							}
						}
						
						// Read the next score information
						if(c == 'a')
						{
							if(fscanf(inFp, " score=%lf\n", &msab->score) != 1)				
							{
								fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: failed to get alignment score line!\n", threadNumber);							
								fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: line format expected: a score=###...#.000000\n", threadNumber);
								fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: current file position: byte %"PRIu64"\n", threadNumber, Ftello(inFp));
								pthread_exit(NULL);
							}
						}

						// It the next character is a '#', it means that the encoder reached the end of the MAF file
						// This cenario hapens when a MAF files ends with:
						//	##eof maf
						// It is necessary to sinalize this information to the decoder
						if(c == '#')
						{
							//printf("Reading time: %.3f seconds\n", readingTime);
							//printf("Compression time: %.3f seconds\n", compressingTime);
							//printf("Search species name time: %.3f seconds\n", msaImg->speciesNameSearchTime);
							
							// If a '#' character is found it means that the algorithm is in the end reading the final line
							EndMAFReading(inFp, threadNumber, 0x1, 0x1, msab, threadsData->storageBitsInfo, acEncoder, acModels, cModels, cTemplate, outMSABInfoFile, totalBlocks);	
							
						}
					}
					else
					{
						// Read the next score information
						if(fscanf(inFp, " score=%lf\n", &msab->score) != 1)				
						{
							fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: failed to get alignment score line!\n", threadNumber);
							fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: line format expected: a score=###...#.000000\n", threadNumber);
							fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: current file position: byte %"PRIu64"\n", threadNumber, Ftello(inFp));
							pthread_exit(NULL);
						}
					}
					break;
				
				// In case of an unexpected character
				default:
					fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: unexpected character '%c' (ascii: %"PRId8").\n", threadNumber, c, c);
					fprintf(stderr, "At Byte: %"PRIu64"\n", Ftello(inFp));
					pthread_exit(NULL);
			}	
		}
		
		//printf("Thread %"PRIu8" skip value %"PRIu8"\n", threadNumber, skip);
		
		if(skip == 0x0)
		{	
			// Deal with the last MSAB read here
			// Verify the number of rows of the current MSAB is bigger than maxMSABNRows 
			if(msab->sLinesData->nRows > maxMSABNRows)
			{
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: The block %"PRIu64" is to big!", threadNumber, totalBlocks);
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Block has two many rows (%"PRIu32" rows).\n", threadNumber, msab->sLinesData->nRows);
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Max number of rows allowed is: %"PRIu32".\n", threadNumber, maxMSABNRows);
				fprintf(stderr, "Rerun the encoder using an higher '-nr' paramater value ( > %"PRIu32").\n", msab->sLinesData->nRows);
				pthread_exit(NULL);
			}
	
			// Verify the number of columns of the current MSAB
			if(msab->sLinesData->nCols > maxMSABNCols)
			{
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: The block %"PRIu64" is to big!\n", threadNumber, totalBlocks);
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Block has too many columns (%"PRIu32" columns).\n", threadNumber, msab->sLinesData->nCols);
				fprintf(stderr, "Error (readMAFPortion)[Thread %02"PRIu8"]: Max number of columns allowed is: %"PRIu32".\n", threadNumber, maxMSABNCols);
				fprintf(stderr, "Rerun the encoder using an higher '-nc' paramater value ( > %"PRIu32").\n", msab->sLinesData->nCols);						
				pthread_exit(NULL);
			}
	
			// DEBUG MODE
			#ifdef DEBUG_1
				fprintf(outMSABInfoFile, "%10"PRIu32" %10"PRIu32"\n", msab->sLinesData->nRows, msab->sLinesData->nCols);
			#endif
	
			// Compression of the MSAB 
			EncodeMSAB(threadNumber, msab, acEncoder, acModels, cModels, cTemplate, threadsData->fieldsLimits, threadsData->storageBitsInfo);
	
			// Save some statistics
			totalBlocks++;
			globalInfo->totalBlocks[threadNumber]++;
			globalInfo->totalBases[threadNumber] += (msab->sLinesData->nRows * msab->sLinesData->nCols);
		
			// If this is the last part, the compression is terminated
			if(GOBId == threadsData->nGOBs-1)
			{
				// End reading process
				EndMAFReading(inFp, threadNumber, 0x0, 0x1, msab, threadsData->storageBitsInfo, acEncoder, acModels, cModels, cTemplate, outMSABInfoFile, totalBlocks);
			}
			// There are still parts to process but there are thing that need be done
			// after reading a part
			else
			{
				EndPartReading(threadNumber, msab, threadsData->storageBitsInfo, acEncoder, acModels, totalBlocks);
			}
		}
	}
	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ReadMAFHeaderLines(FILE *inFp, uint8_t threadNumber, MSAB *msab, 
	ACEncoder *acEncoder, ACModels *acModels, FieldsLimits *fieldsLimits,
	StorageBitsInfo	*storageBitsInfo)
{
	char *line;
	uint32_t maxLineSize = fieldsLimits->maxMSABNCols;
	uint32_t maxHeaderLineSize = fieldsLimits->maxHeaderLineSize;
	uint32_t storageBitsHeaderLine = storageBitsInfo->storageBitsHeaderLine;
	
	// Allocate memory
	line = (char *)Calloc(maxLineSize, sizeof(char));		
	
	// Read the initial lines until the first "a" line
	do
	{
		// Read line
		if(!fgets(line, maxLineSize, inFp))
		{
			fprintf(stderr, "Error (readMAFHeaderLines)[Thread %02"PRIu8"]: unexpected end-of-file\n", threadNumber);
			pthread_exit(NULL);
		}
	
		// Is still a MAF header line?
		// If the first character of the read line is not an 'a' it means that there is more lines
		// starting by '#'
		if(line[0] != 'a')
		{			
			// Verify if the size of the line if higher than HEADER_LINE_MAX_SIZE
			if (Strlen(line) > maxHeaderLineSize)
			{
				fprintf(stderr, "Error (readMAFHeaderLines)[Thread %02"PRIu8"]: MAF header line is too big!\n", threadNumber);
				fprintf(stderr, "Line read (%"PRIu64" characters): '%s'\n", (uint64_t)Strlen(line), line);
				fprintf(stderr, "Maximum MAF header line size allowed is %"PRIu32" characters.\n", maxHeaderLineSize);
				fprintf(stderr, "Rerun the encoder using an higher '-mhs' paramater value ( > %"PRIu64").\n", (uint64_t)Strlen(line));
				pthread_exit(NULL);
			}

			// Save the information about the number of MAF header lines
			globalInfo->numberOfHeaderLines++;
			
			// Write the size of the line
			writeNBits(Strlen(line)-1, storageBitsHeaderLine, acEncoder->globalEncoder, acModels->binaryUniformACModel);
			//WriteNBits(Strlen(line)-1, STORAGE_HEADER_LINE_MAX_SIZE, acEncoders->globalEncoder, acModels->binaryModel);

			// Encode the header line
			writeString(line, Strlen(line), acEncoder->globalEncoder, acModels->binaryUniformACModel);

		}
	}
	// If we read an 'a', we need to stop the read process of '#'
	while(line[0] != 'a');	

	// The variable "line" contains the score information that we need to extract
	if(sscanf(line,"a score=%lf", &msab->score) != 1)
	{
		fprintf(stderr, "Error (readMAFHeaderLines)[Thread %02"PRIu8"]: failed to get alignment score line!\n", threadNumber);
		fprintf(stderr, "Error (readMAFHeaderLines)[Thread %02"PRIu8"]: line read           : '%s'", threadNumber, line);
		fprintf(stderr, "Error (readMAFHeaderLines)[Thread %02"PRIu8"]: line format expected: a score=###...#.000000\n", threadNumber);
		fprintf(stderr, "Error (readMAFHeaderLines)[Thread %02"PRIu8"]: current file position: byte %"PRIu64"\n", threadNumber, Ftello(inFp));
		pthread_exit(NULL);
	}

	// Free memory allocated
	Free(line, maxLineSize*sizeof(char));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EndMAFReading(FILE *inFp, uint8_t threadNumber, uint8_t eofMAF, 
	uint8_t lastPart, MSAB *msab, StorageBitsInfo *storageBitsInfo, 
	ACEncoder *acEncoder, ACModels *acModels, CModels *cModels, 
	CTemplate *cTemplate, FILE *outMSABInfoFile, uint64_t numberOfMSAB)
{
	uint32_t storageBitsMSABRows = storageBitsInfo->storageBitsMSABRows;

	// Close the input file
	Fclose(inFp);
	
	// DEBUG MODE
	#ifdef DEBUG_1
		Fclose(outMSABInfoFile);
	#endif // DEBUG MODE
	
	// Sinalize the end of file with nRows = 0
	writeNBits(0x0, storageBitsMSABRows, acEncoder->globalEncoder, acModels->binaryUniformACModel);
	// DEBUG MODE
	#ifdef DEBUG_1
		writeNBits(0x0, storageBitsMSABRows, aceDebuggers->aceSNRows[threadNumber], acModels->binaryUniformACModel);
	#endif // DEBUG_1	

	// Only send the "##eof maf" flag if we are dealing with the last MSAB
	if(lastPart == 0x1)
	{
		// The file ends with a "##eof maf"
		writeNBits(eofMAF, 1, acEncoder->globalEncoder, acModels->binaryUniformACModel);
		// DEBUG MODE
		#ifdef DEBUG_1
			writeNBits(eofMAF, 1, aceDebuggers->aceHeader[threadNumber], acModels->binaryUniformACModel);
		#endif // DEBUG_1	
	}
	
	// Close the encoder
	ACEncoderDone(acEncoder);
	
	// Save the number of bits used
	globalInfo->totalBits[threadNumber] += ac_encoder_bits(acEncoder->globalEncoder);
	
	// Free allocated memory
	FreeACEncoder(acEncoder);
	FreeACModels(acModels);
	FreeCModels(cModels);
	FreeTemplate(cTemplate);
	
	
	HashingStats(msab->hashTable);
	FreeMSAB(msab);

	pthread_exit(NULL);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EndPartReading(uint8_t threadNumber, MSAB *msab,
	StorageBitsInfo *storageBitsInfo, ACEncoder *acEncoder, ACModels *acModels,
	uint64_t numberOfMSAB)
{
	uint32_t storageBitsMSABRows = storageBitsInfo->storageBitsMSABRows;
	
	// Sinalize the end of file with nRows = 0
	writeNBits(0x0, storageBitsMSABRows, acEncoder->globalEncoder, acModels->binaryUniformACModel);
	
	// DEBUG MODE
	#ifdef DEBUG_1
		writeNBits(0x0, storageBitsMSABRows, aceDebuggers->aceSNRows[threadNumber], acModels->binaryUniformACModel);
	#endif // DEBUG_1	
	
	// Close the encoder
	ACEncoderDone(acEncoder);
	
	// Save the number of bits used in this part
	globalInfo->totalBits[threadNumber] += ac_encoder_bits(acEncoder->globalEncoder);
		
	ResetMSAB(msab);
	
	// Reset previous 'e' lines status by setting the field <currentNRows> to zero!
	msab->prevELinesInfo->currentNRows = 0;
	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ReadSLine(FILE *inpFp, uint8_t threadNumber, MSAB *msab)
{
	//char src[MAX_SPECIE_NAME], strand='+', base;
	//char *src, strand='+', base;
	char strand='+', base;
	//uint64_t srcSize=0, start = 0, size = 0, sequenceRowSize=0;
	uint32_t sourceSize=0, start = 0, seqSize = 0, sequenceRowSize=0;
	uint32_t currentRow = 0;
	UChar *sequenceRow = NULL;
	uint32_t rowBlockSize = msab->rowBlockSize;
	Element *element;
	
	
	// Read the 's' lines header info of the MSAB
	if(fscanf(inpFp, "%s %"SCNu32" %"SCNu32" %c %"SCNu32"\n", msab->sourceName, &start, &seqSize, &strand, &sourceSize) != 5)
	{
		fprintf(stderr, "Error (readSLine)[Thread %02"PRIu8"]: failed to get the 's' line.\n", threadNumber);
		pthread_exit(NULL);
	}
		
	// Read each base one by one
	while((base = fgetc(inpFp)) != '\n')
	{
		// Count the number of n or N
		if(base == 'n' || base == 'N') msab->xchrCount++;

		// Count if there is any lower case 
		if(islower(base)) msab->lowerCaseCount++; 

		// Store each base and gap
		StoreSequenceValue(&sequenceRow, sequenceRowSize, rowBlockSize, base);

		// Increment the size of the current 's' line
		sequenceRowSize++;
	}
	
	// Associate the 's' line read to the MSAB
	StoreSequenceRow(msab->sLinesData, sequenceRow, sequenceRowSize);

	// The current row of the sLinesData
	currentRow = msab->sLinesData->nRows-1;
	
	// Save the pointer location of the element read
	// This will also set the values of the hash position (hashKey and elementID)
	// These two values must be send to the decoder latter
	element = FindElementByString(msab->sourceName, msab->hashTable, msab->hashPosition);
	
	// Store sourceName, start, seqSize, strand, sourceSize, etc.	
	StoreLineInfo(msab->linesInfo, element, msab->hashPosition, currentRow, 
		msab->sourceName, start, seqSize, ((strand == '-') ? 0x0 : 0x1), sourceSize, 0x1);
	
	// Set the presence of 'q' line to false just in case
	msab->linesInfo->elements[currentRow]->qualityInfo = 0x0;
						
	// Update the number of 's' lines read
	globalInfo->totalSLines[threadNumber]++;
	globalInfo->totalGaps[threadNumber] += (msab->sLinesData->nCols-seqSize);
	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ReadQLine(FILE *inpFp, uint8_t threadNumber, MSAB *msab)
{
	//UChar *src, qualityValue;
	UChar qualityValue;
	uint8_t *qualityValuesRow=NULL;
	uint32_t rowBlockSize = msab->rowBlockSize;
	uint64_t qualityValuesRowSize=0;
	
	// Read the species.chromosome info
	if(fscanf(inpFp, "%s", msab->sourceName) != 1)
	{
		fprintf(stderr, "Error (readQLine)[Thread %02"PRIu8"]: failed to get the 'q' line\n", threadNumber);
		pthread_exit(NULL);
	}
	
	// Read each quality value until reaching an '\n'
	while((qualityValue = fgetc(inpFp)) != '\n')
	{
		// Only stores the symbol if we are not in the presence of a space character
		// This corresponds to the spaces that are after the species.chr string
		if(qualityValue != ' ')
		{
			StoreQualityValue(&qualityValuesRow, qualityValuesRowSize, rowBlockSize, qualityValue);
			qualityValuesRowSize++;	
		}
	}
	// Save the 'q' line to be compressed later
	//StoreQualityRow(msab->qLinesData, qualityValuesRow, qualityValuesRowSize);
	StoreQualityRow(msab->qLinesData, qualityValuesRow, qualityValuesRowSize);
	
	// Update the number of 'q' lines read
	globalInfo->totalQLines[threadNumber]++;
		
	// Set the score status to 1 in order to inform the decoder that we have 
	// a quality line for the current source that correspond to the previous
	// 's' line alignment
	msab->linesInfo->elements[msab->sLinesData->nRows-1]->qualityInfo = 0x1;
	//msaImg->qScoreStatus[msaImg->nRows-1] = 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ReadILine(FILE *inpFp, uint8_t threadNumber, MSAB *msab)
{
	//UChar *src, leftStatus, rightStatus;
	UChar leftStatus, rightStatus;
	uint32_t leftCount, rightCount;
	
	// Read the 'i' line
	// i <specie.chr> <leftStatus> <leftCount> <rightStatus> <rightCount>
	if(fscanf(inpFp, "%s %1c %"SCNu32" %1c %"SCNu32"\n", msab->sourceName, &leftStatus, &leftCount, &rightStatus, &rightCount) != 5)
	{
		fprintf(stderr, "Error (readILine)[Thread %02"PRIu8"]: failed to get the 'i' line\n", threadNumber);
		pthread_exit(NULL);
	}
	
	StoreInfoRow(msab->iLinesData, leftStatus, rightStatus, leftCount, rightCount);
	
	// Update the number of 'i' lines read
	globalInfo->totalILines[threadNumber]++;
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ReadELine(FILE *inpFp, uint8_t threadNumber, MSAB *msab, 
	FieldsLimits *fieldsLimits)
{
	UChar strand='+', status;
	uint32_t currentRow = 0;
	uint32_t sourceSize=0, start = 0, seqSize = 0;
	Element *element;
	
	// e <specie.chr> <start> <size> <strand> <srcSize>
	if(fscanf(inpFp, "%s %"SCNu32" %"SCNu32" %c %"SCNu32" %c\n", msab->sourceName, 
		&start, &seqSize, &strand, &sourceSize, &status) != 6)
	{
		fprintf(stderr, "Error (readELine)[Thread %02"PRIu8"]: failed to get the 'e' line\n", threadNumber);
		pthread_exit(NULL);
	}
	
	// Verify if the sequence size value is higher then the maximum allowed
	// The number of columns limit can be used as a reference because they are related
	if(seqSize > fieldsLimits->maxMSABNCols)
	{
		fprintf(stderr, "Error (ReadELine)[Thread %02"PRIu8"]: sequence size of the following 'e' line is to big\n", threadNumber);
		fprintf(stderr, "Error (ReadELine)[Thread %02"PRIu8"]: e %s %"PRIu32" %"PRIu32" %c %"PRIu32" %c", 
			threadNumber, msab->sourceName, start, seqSize, strand, sourceSize, status);
		fprintf(stderr, "Error (ReadELine)[Thread %02"PRIu8"]: maximum sequence size value allowed in this context = %"PRIu32"\n", 
			threadNumber, fieldsLimits->maxMSABNCols);	
		fprintf(stderr, "Rerun the encoder using an higher '-nc' paramater value ( > %"PRIu32").\n", seqSize);						
		pthread_exit(NULL);
	}
	
	StoreEmtpyRegionRow(msab->eLinesData, start, seqSize, strand, sourceSize, status);
	
	// The current row of the sLinesData
	currentRow = msab->eLinesData->nRows-1;
	
	// Save the pointer location of the element read
	// This will also set the values of the hash position (hashKey and elementID)
	// These two values must be send to the decoder latter
	element = FindElementByString(msab->sourceName, msab->hashTable, msab->hashPosition);
	
	// Store sourceName, start, seqSize, strand, sourceSize, etc.	
	StoreLineInfo(msab->eLinesInfo, element, msab->hashPosition, currentRow, 
		msab->sourceName, start, seqSize, ((strand == '-') ? 0x0 : 0x1), sourceSize, 0x1);
	
	// Update sequence size
	element->seqSize = seqSize;
	
	// Update the number of 'e' lines read
	globalInfo->totalELines[threadNumber]++;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EncodeMSAB(uint8_t threadNumber, MSAB *msab, ACEncoder *acEncoder, 
	ACModels *acModels, CModels *cModels, CTemplate *cTemplate, 
	FieldsLimits *fieldsLimits, StorageBitsInfo *storageBitsInfo)
{
	
	//printf("Encoding MSAB %lf\n", msab->score);
	
	// Encode the 's' lines information
	//EncodeSLinesData(threadNumber, msab, acEncoder, acModels, cModels, 
	EncodeSLinesData(threadNumber, msab, acEncoder, acModels, cModels, 
		cTemplate, fieldsLimits, storageBitsInfo);
	
	// Encode the 'q' lines information
	EncodeQLinesData(threadNumber, msab, acEncoder, acModels, cModels);
	
	// Encode the 'i' lines information
	EncodeILinesData(threadNumber, msab, acEncoder, acModels, cModels,
		fieldsLimits, storageBitsInfo);
		
	// Encode the 'e' lines information
	EncodeELinesData(threadNumber, msab, acEncoder, acModels, cModels,
		storageBitsInfo);
			
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EncodeSLinesData(uint8_t threadNumber, MSAB *msab, ACEncoder *acEncoder, 
	ACModels *acModels, CModels *cModels, CTemplate *cTemplate, 
	FieldsLimits *fieldsLimits, StorageBitsInfo *storageBitsInfo)
{
	UChar c;
	uint8_t s, binaryFlag, caseSymbol, xchrSymbol, strandSymbol;
	//int16_t startOffset;
	uint32_t score, row, col;
	int64_t startOffset;
	size_t sourceNameSize;
	
	// Encode the number of 's' lines and their size (number of columns)
	writeNBits(msab->sLinesData->nRows, storageBitsInfo->storageBitsMSABRows, acEncoder->globalEncoder, acModels->binaryUniformACModel);
	writeNBits(msab->sLinesData->nCols, storageBitsInfo->storageBitsMSABCols, acEncoder->globalEncoder, acModels->binaryUniformACModel);
	
	// DEBUG MODE
	#ifdef DEBUG_1
		writeNBits(msab->sLinesData->nRows, storageBitsInfo->storageBitsMSABRows, aceDebuggers->aceSNRows[threadNumber], acModels->binaryUniformACModel);
		writeNBits(msab->sLinesData->nCols, storageBitsInfo->storageBitsMSABCols, aceDebuggers->aceSNCols[threadNumber], acModels->binaryUniformACModel);
	#endif // DEBUG_1	
	
	//////////////////////////////////////////////////////// SCORE ENCODING BEGIN
	// Encode the score sign information
	//if(msab->score < 0.0) binaryFlag = 0x1;	// Negative score
	//else binaryFlag = 0x0;					// Positive score
	binaryFlag = ( (msab->score < 0.0) ? 0x1 : 0x0);
	writeNBits(binaryFlag, 1, acEncoder->globalEncoder, acModels->binaryUniformACModel);
	
	// DEBUG MODE
	#ifdef DEBUG_1	
		writeNBits(binaryFlag, 1, aceDebuggers->aceScore[threadNumber], acModels->binaryUniformACModel);
	#endif // DEBUG_1	
		
	score = ((msab->score >= 0.0) ? (uint64_t)msab->score : (uint64_t)(-1.0*msab->score));
	if(score > fieldsLimits->maxAbsScoreValue)
	{
		fprintf(stderr, "Error (CompressSLinesData)[Thread %02"PRIu8"]: absolute score value %"PRIu32" is to high!\n", threadNumber, score);
		fprintf(stderr, "Error (CompressSLinesData)[Thread %02"PRIu8"]: maximum absolute score value allowed: %"PRIu32".\n", threadNumber, fieldsLimits->maxAbsScoreValue); 
		fprintf(stderr, "Rerun the encoder using an higher '-mas' paramater value ( > %"PRIu32").\n", score);
		pthread_exit(NULL);
	}
	
	writeNBits(score, storageBitsInfo->storateBitsAbsScoreValue, acEncoder->globalEncoder, acModels->binaryUniformACModel);
	// DEBUG MODE
	#ifdef DEBUG_1	
		writeNBits(score, storageBitsInfo->storateBitsAbsScoreValue, aceDebuggers->aceScore[threadNumber], acModels->binaryUniformACModel);
	#endif // DEBUG_1	
	//////////////////////////////////////////////////////// SCORE ENCODING END
	
	
	//////////////////////////////////////////////////////// CASE INFO ENCODING BEGIN
	// Encode information regarding the presence of lower case letters
	binaryFlag = ((msab->lowerCaseCount > 0) ? 0x1 : 0x0);
	globalInfo->totalLowerCaseBlocks[threadNumber] += binaryFlag;
	ComputePModel2(cModels->caseFlagCModel, acModels->binaryACModel);
	acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
	// DEBUG MODE
	#ifdef DEBUG_1
		acEncodeBinary(aceDebuggers->aceCaseFlag[threadNumber], acModels->binaryACModel, binaryFlag);
	#endif // DEBUG_1
	UpdateCModelCounter(cModels->caseFlagCModel, binaryFlag);
	UpdateCModelIdx(cModels->caseFlagCModel, binaryFlag);
	//////////////////////////////////////////////////////// CASE INFO ENCODING END
	
	//////////////////////////////////////////////////////// EXTRA CHAR INFO ENCODING BEGIN
	// Encode information regarding the presence of extra characters (n and N)
	binaryFlag = ((msab->xchrCount > 0) ? 0x1 : 0x0);
	globalInfo->totalXchrBlocks[threadNumber] += binaryFlag;
	ComputePModel2(cModels->xchrFlagCModel, acModels->binaryACModel);
	acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
	// DEBUG MODE
	#ifdef DEBUG_1
		acEncodeBinary(aceDebuggers->aceXchrFlag[threadNumber], acModels->binaryACModel, binaryFlag);
	#endif // DEBUG_1
	UpdateCModelCounter(cModels->xchrFlagCModel, binaryFlag);
	UpdateCModelIdx(cModels->xchrFlagCModel, binaryFlag);
	//////////////////////////////////////////////////////// EXTRA CHAR INFO ENCODING END
	
	//if(msab->score == 2238629.000000)
	//{
	/*
	if(msab->score ==420268.000000)
	{
		printf("Score = %lf | Encoding MSAB with %"PRIu32" x %"PRIu32".\n", msab->score, msab->sLinesData->nRows, msab->sLinesData->nCols);
		//print_ac_model_info(acModels->binaryUniformACModel);
		//print_ac_encoder_info(acEncoder->globalEncoder);
	}
	*/	
	//printf("Encoding MSAB with %"PRIu32" x %"PRIu32".\n", msab->sLinesData->nRows, msab->sLinesData->nCols);
	//}
	
	//////////////////////////////////////////////////////// HEADER S LINES ENCODING BEGIN
	for(row = 0; row != msab->sLinesData->nRows; ++row)
	{
		/*
		if( (msab->score ==420268.000000) && (row < 4) )
		{
			print_ac_model_info(acModels->binaryUniformACModel);
			print_ac_encoder_info(acEncoder->globalEncoder);
		}
		*/
		
		//////////////////////////////////////////////////////// HASH KEY AND ELEMENT ID ENCODING BEGIN
		writeNBits(msab->linesInfo->hashPositions[row].hashKey, 
			storageBitsInfo->storageBitsHashKeyValue, acEncoder->globalEncoder, acModels->binaryUniformACModel);
		writeNBits(msab->linesInfo->hashPositions[row].elementId, 
				storageBitsInfo->storageBitsHashElementId, acEncoder->globalEncoder, acModels->binaryUniformACModel);
		
		//if(msab->score == 2238629.000000 && row < 4)
		/*
		if( (msab->score ==420268.000000) && (row < 4) )
		{
			printf("Hash Key: %"PRIu32"| Element Id: %"PRIu8"\n", 
				msab->linesInfo->hashPositions[row].hashKey, msab->linesInfo->hashPositions[row].elementId);
		}
		*/		
		// DEBUG MODE
		#ifdef DEBUG_1
			writeNBits(msab->linesInfo->hashPositions[row].hashKey, 
				storageBitsInfo->storageBitsHashKeyValue, aceDebuggers->aceHashInfo[threadNumber], 
				acModels->binaryUniformACModel);
			writeNBits(msab->linesInfo->hashPositions[row].elementId, 
				storageBitsInfo->storageBitsHashElementId, aceDebuggers->aceHashInfo[threadNumber], 
				acModels->binaryUniformACModel);	
		#endif // DEBUG_1
		//////////////////////////////////////////////////////// HASH KEY AND ELEMENT ID ENCODING END
		
		//////////////////////////////////////////////////////// STRAND ENCODING BEGIN
		strandSymbol = msab->linesInfo->elements[row]->strand;
		ComputePModel2(cModels->strandCModel, acModels->binaryACModel);
		acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, strandSymbol);

		// DEBUG MODE
		#ifdef DEBUG_1
			acEncodeBinary(aceDebuggers->aceStrand[threadNumber], acModels->binaryACModel, strandSymbol);
		#endif // DEBUG_1
		UpdateCModelCounter(cModels->strandCModel, strandSymbol);
		UpdateCModelIdx(cModels->strandCModel, strandSymbol);
		//////////////////////////////////////////////////////// STRAND ENCODING END						
		
		// If the element was not encoded, it is necessary to encode all the information
		if(msab->linesInfo->elements[row]->sStatus == 0x0)
		{
			//////////////////////////////////////////////////////// SOURCE NAME ENCODING BEGIN
			// Encode the size of the string
			sourceNameSize = Strlen(msab->linesInfo->elements[row]->sourceName);
			writeNBits(sourceNameSize, 
				storageBitsInfo->storageBitsSrcName, acEncoder->globalEncoder, 
				acModels->binaryUniformACModel);
			writeString(msab->linesInfo->elements[row]->sourceName, 
				sourceNameSize, acEncoder->globalEncoder, acModels->binaryUniformACModel);
			
			/*
			//if(msab->score == 2238629.000000 && row < 4)
			if( (msab->score == 420268.000000) && (row < 4) )
			{
				printf("New source '%s' was encoded as raw with %zu characters.\n\n", msab->linesInfo->elements[row]->sourceName, sourceNameSize);
			}
			*/	
			// DEBUG MODE
			#ifdef DEBUG_1
				writeNBits(sourceNameSize, 
					storageBitsInfo->storageBitsSrcName, aceDebuggers->aceSourceName[threadNumber], 
					acModels->binaryUniformACModel);
				writeString(msab->linesInfo->elements[row]->sourceName, 
					sourceNameSize, aceDebuggers->aceSourceName[threadNumber], acModels->binaryUniformACModel);
			#endif // DEBUG_1				
			//////////////////////////////////////////////////////// SOURCE NAME ENCODING END	
			
			
			//////////////////////////////////////////////////////// START POSITION ENCODING BEGIN
			writeNBits(msab->linesInfo->elements[row]->start, 
				storageBitsInfo->storageBitsStartPosition,	acEncoder->globalEncoder, 
				acModels->binaryUniformACModel);
				
			// DEBUG MODE
			#ifdef DEBUG_1
				writeNBits(msab->linesInfo->elements[row]->start, 
					storageBitsInfo->storageBitsStartPosition,	aceDebuggers->aceStartPosRaw[threadNumber], 
					acModels->binaryUniformACModel);
			#endif // DEBUG_1
			//////////////////////////////////////////////////////// START POSITION ENCODING END
			
			//////////////////////////////////////////////////////// SOURCE SIZE ENCODING BEGIN
			writeNBits(msab->linesInfo->elements[row]->sourceSize, 
				storageBitsInfo->storageBitsSourceSize, acEncoder->globalEncoder, 
				acModels->binaryUniformACModel);
				
			// DEBUG MODE
			#ifdef DEBUG_1
				writeNBits(msab->linesInfo->elements[row]->sourceSize, 
					storageBitsInfo->storageBitsSourceSize, aceDebuggers->aceSourceSize[threadNumber], 
					acModels->binaryUniformACModel);
			#endif // DEBUG_1				
			//////////////////////////////////////////////////////// SOURCE SIZE ENCODING END	
			
			// Change the element status flag in order to avoid encoding extra information
			// next time this source appears
			msab->linesInfo->elements[row]->sStatus = 0x1;
		}
		
		// If the source name was already processed before, the start position can be encoded in two ways
		// 	- Using an offset where offset = start - prevStart - seqSize
		//	- Using the absolute value in case of a negative offset or to big offset
		else
		{
			/*
			if( (msab->score ==420268.000000) && (row < 4) )
			{
				printf("Source name was already encoded...\n");
			}
			*/
			//////////////////////////////////////////////////////// START POSITION ENCODING BEGIN
			// The offset value is not always a positive value. There are two situations where it is negative.
			// First situation: prevStart + prevSequenceSize > currentStart
			// This happens because there is a fration of bases in the current MSAB that were already
			// included in the previous MSAB of the current source.
			//
			// Second situation: prevStart > currentStart
			// The previous start position is higher thant the current start position. Usually this happens because
			// there was a first situation case before this block
			//
			// Third situation: prevStart = currentStart
			// There are two MSAB associated with this source that can be alligned in two diferent positions of
			// the reference source
			//
			// One example on the multi28way: head -n 213 chrY.maf | tail -n 12
			
			// This offset value can be negative or positive
			startOffset = (int64_t)msab->linesInfo->elements[row]->start - msab->linesInfo->elements[row]->prevStart -
				msab->linesInfo->elements[row]->seqSize;
				
			// The start offset is negative
			if(startOffset < 0) globalInfo->totalNumNegativeOff[threadNumber]++;
			
			// The start offset is positive
			else 
			{
				if(startOffset > (int64_t)fieldsLimits->maxStartOffset)
					globalInfo->totalNumLargeOff[threadNumber]++;
				
				if(startOffset > (int64_t)globalInfo->maxStartOffset[threadNumber])
					globalInfo->maxStartOffset[threadNumber] = startOffset;
			}
			
			
			// If the start offset is negative or is too big, set the startOffset to -1
			if( (startOffset < 0) || (labs(startOffset) > fieldsLimits->maxStartOffset) )
				startOffset = -1;

							
			//////////////////////////////////////////////////////// START OFFSET SIGN ENCODING BEGIN
			// Compute the binary start offset sign
			binaryFlag = ( ((startOffset < 0) || (startOffset > fieldsLimits->maxStartOffset)) ? 0x1 : 0x0);
			// Encode the start offset sign flag
			ComputePModel2(cModels->startOffsetSignCModel, acModels->binaryACModel);
			acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
			// DEBUG MODE
			#ifdef DEBUG_1
				acEncodeBinary(aceDebuggers->aceStartOffsetSign[threadNumber], acModels->binaryACModel, binaryFlag);
			#endif // DEBUG_1
			UpdateCModelCounter(cModels->startOffsetSignCModel, binaryFlag);
			UpdateCModelIdx(cModels->startOffsetSignCModel, binaryFlag);
			//////////////////////////////////////////////////////// START OFFSET SIGN ENCODING END
			
			//////////////////////////////////////////////////////// RAW START POSITION ENCODING BEGIN
			if(binaryFlag == 0x1)
			{
				writeNBits(msab->linesInfo->elements[row]->start, storageBitsInfo->storageBitsStartPosition,
					acEncoder->globalEncoder, acModels->binaryUniformACModel);
				
				// DEBUG MODE
				#ifdef DEBUG_1
					writeNBits(msab->linesInfo->elements[row]->start, storageBitsInfo->storageBitsStartPosition,
					aceDebuggers->aceStartPosRaw[threadNumber], acModels->binaryUniformACModel);
				#endif // DEBUG_1
			}
			//////////////////////////////////////////////////////// RAW START POSITION ENCODING END
			
			//////////////////////////////////////////////////////// OFFSET START POSITION ENCODING BEGIN
			else
			{
				writeNBits(startOffset, storageBitsInfo->storageBitsStartOffset, acEncoder->globalEncoder, 
					acModels->binaryUniformACModel);
				// DEBUG MODE
				#ifdef DEBUG_1
					writeNBits(startOffset, storageBitsInfo->storageBitsStartOffset, 
						aceDebuggers->aceStartOffset[threadNumber], acModels->binaryUniformACModel);
				#endif // DEBUG_1
			}
			//////////////////////////////////////////////////////// OFFSET START POSITION ENCODING END
		}
		
	}
	//////////////////////////////////////////////////////// HEADER S LINES ENCODING END
	
	
	//////////////////////////////////////////////////////// S LINES ENCODING BEGIN
	for(row = 0; row != msab->sLinesData->nRows; ++row)
	{
		for(col = 0; col != msab->sLinesData->nCols; ++col)
		{
			
			// Get the base character (ASCCI code)
			c = GetSequenceValueCharacter(msab->sLinesData, row, col);
			
			// Convert the previous character to a symbol (integer value)
			s = SequenceValueCharacter2Symbol(c);
			
			GetPModelIdx(msab, row, col, cTemplate, cModels->sValuesCModel);
			ComputePModel(cModels->sValuesCModel, acModels->sValuesACModel);
			ac_encode_symbol(acEncoder->globalEncoder, acModels->sValuesACModel, s);
			// DEBUG MODE
			#ifdef DEBUG_1
				ac_encode_symbol(aceDebuggers->aceSLines[threadNumber], acModels->sValuesACModel,s);
			#endif // DEBUG_1	
			UpdateCModelCounter(cModels->sValuesCModel, s);	
			
			
			//////////////////////////////////////////////////////// CASE ENCODING BEGIN 
			// On one hand, the program checks if there is any lower case symbol in the 
			// current MSAB. On the other hand, we only need to send information when
			// the current symbol is not an alignment gap '-'
			if(s != GAP_SYMBOL && msab->lowerCaseCount)
			{
				// Case information
				caseSymbol = (islower(c) ? 0x1 : 0x0);
				ComputePModel2(cModels->caseCModel, acModels->binaryACModel);
				acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, caseSymbol);
				// DEBUG MODE
				#ifdef DEBUG_1
					acEncodeBinary(aceDebuggers->aceCase[threadNumber], acModels->binaryACModel, caseSymbol);
				#endif // DEBUG_1
				UpdateCModelCounter(cModels->caseCModel, caseSymbol);
				UpdateCModelIdx(cModels->caseCModel, caseSymbol);
				
				globalInfo->totalLowerCaseSymbols[threadNumber] += caseSymbol;
			}
			//////////////////////////////////////////////////////// CASE ENCODING END 
			
			
			//////////////////////////////////////////////////////// EXTRA BASES ENCODING BEGIN
			// The current symbol is an extra char (N or n)?
			if(s == XCHR_SYMBOL && msab->xchrCount > 0)
			{
				xchrSymbol = ((c == 'N' || c == 'n') ?  0x1 : 0x0);
				ComputePModel2(cModels->xchrCModel, acModels->binaryACModel);
				acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, xchrSymbol);
				// DEBUG MODE
				#ifdef DEBUG_1
					acEncodeBinary(aceDebuggers->aceXchr[threadNumber], acModels->binaryACModel, xchrSymbol);
				#endif // DEBUG_1
				UpdateCModelCounter(cModels->xchrCModel, xchrSymbol);
				UpdateCModelIdx(cModels->xchrCModel, xchrSymbol);
				
				globalInfo->totalXcharNC[threadNumber]++;				// Extra char in this case is {N, n, C, c}
				globalInfo->totalXcharN[threadNumber] += xchrSymbol;	// Count the N's and n's				
			}
			//////////////////////////////////////////////////////// EXTRA BASES ENCODING END
			
		}
		
		// Update the sequence size field 
		msab->linesInfo->elements[row]->seqSize = msab->linesInfo->elements[row]->currentSeqSize;
		
	}	
	//////////////////////////////////////////////////////// S LINES ENCODING ENDS	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EncodeQLinesData(uint8_t threadNumber, MSAB *msab, ACEncoder *acEncoder, 
	ACModels *acModels, CModels *cModels)
{
	uint8_t binaryFlag, qValue;
	uint32_t row, col;
	
	//////////////////////////////////////////////////////// Q LINES ENCODING BEGIN
	// The first thing to do is to verify if there is actually any 'q' lines to encode
	if(msab->qLinesData->nRows == 0)
	{
		// It is necessary to send to the decoder this information saying that
		// there are no 'q' lines in the current MSAB
		binaryFlag = 0x0;
		ComputePModel2(cModels->qLineFlagCModel, acModels->binaryACModel);
		acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
		// DEBUG MODE
		#ifdef DEBUG_1
			acEncodeBinary(aceDebuggers->aceQLineFlag[threadNumber], 
							acModels->binaryACModel, binaryFlag);
		#endif // DEBUG_1
		UpdateCModelCounter(cModels->qLineFlagCModel, binaryFlag);
		UpdateCModelIdx(cModels->qLineFlagCModel, binaryFlag);
	}
	else
	{
		// Inform the decoder that there is 'q' lines to decode
		binaryFlag = 0x1;
		ComputePModel2(cModels->qLineFlagCModel, acModels->binaryACModel);
		acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
		// DEBUG MODE
		#ifdef DEBUG_1
			acEncodeBinary(aceDebuggers->aceQLineFlag[threadNumber], 
							acModels->binaryACModel, binaryFlag);
		#endif // DEBUG_1
		UpdateCModelCounter(cModels->qLineFlagCModel, binaryFlag);
		UpdateCModelIdx(cModels->qLineFlagCModel, binaryFlag);
		
		// This loop will send the precise position of the 'q' lines inside the MSAB
		// The loop can start in 1 instead of 0 because the first 's' line never has
		// a 'q' line of the same source
		for(row = 1; row != msab->sLinesData->nRows; ++row)
		{
			// Extract the flag that sinalizes if there is a 'q' line or not
			binaryFlag = msab->linesInfo->elements[row]->qualityInfo;
			// Encode the positions of each 'q' line
			ComputePModel2(cModels->qLineInFlagCModel, acModels->binaryACModel);
			acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
			// DEBUG MODE
			#ifdef DEBUG_1
				acEncodeBinary(aceDebuggers->aceQLineInFlag[threadNumber], 
								acModels->binaryACModel, binaryFlag);
			#endif // DEBUG_1
			UpdateCModelCounter(cModels->qLineInFlagCModel, binaryFlag);
			UpdateCModelIdx(cModels->qLineInFlagCModel, binaryFlag);
			
		}

		// After the previous encoding loop, the decoder will have access to the positions 
		// of each q line. Now the encoder only needs to encode the quality values
		for(row = 0; row != msab->qLinesData->nRows; ++row)
		{
			for(col = 0; col != msab->qLinesData->nCols; ++col)
			{
				// Get the quality value
				qValue = GetQualityValue(msab->qLinesData, row, col);
				// Only encode the quality value if it is not a gap symbol (-)
				if(qValue != Q_GAP_SYMBOL)
				{
					GetPModelIdx3(msab->qLinesData->buffer, msab->qLinesData->bufSize, 
									cModels->qValuesCModel);
					ComputePModel(cModels->qValuesCModel, acModels->qValuesACModel);
					ac_encode_symbol(acEncoder->globalEncoder, acModels->qValuesACModel, 
										qValue);
					// DEBUG MODE
					#ifdef DEBUG_1
						ac_encode_symbol(aceDebuggers->aceQLines[threadNumber], 
											acModels->qValuesACModel, qValue);
					#endif // DEBUG_1	
					UpdateCModelCounter(cModels->qValuesCModel, qValue);	
					ShiftBuffer(msab->qLinesData->buffer, msab->qLinesData->bufSize, 
								qValue);
				}
			}
		}
	}
	//////////////////////////////////////////////////////// Q LINES ENCODING END
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void EncodeILinesData(uint8_t threadNumber, MSAB *msab, ACEncoder *acEncoder, 
	ACModels *acModels, CModels *cModels, FieldsLimits *fieldsLimits, 
	StorageBitsInfo *storageBitsInfo)
{
	uint8_t binaryFlag, statusSymbol;
	uint32_t row, sRow, count;
		
	//////////////////////////////////////////////////////// I LINES ENCODING BEGIN
	// The first thing to do is to verify if there is actually any 'i' lines to encode
	if(msab->iLinesData->nRows == 0)
	{
		binaryFlag = 0x0;
		ComputePModel2(cModels->iLineFlagCModel, acModels->binaryACModel);
		acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
		// DEBUG MODE
		#ifdef DEBUG_1
			acEncodeBinary(aceDebuggers->aceILineFlag[threadNumber], acModels->binaryACModel, binaryFlag);
		#endif // DEBUG_1
		UpdateCModelCounter(cModels->iLineFlagCModel, binaryFlag);
		UpdateCModelIdx(cModels->iLineFlagCModel, binaryFlag);
	}
	else
	{
		// If there are 'i' lines to encode, there should be (numberOfSLines - 1) 
		// 'i' lines. The reference source (usually the human) does not have an
		// 'i' line associated
		if(msab->iLinesData->nRows != (msab->sLinesData->nRows-1) )
		{
			fprintf(stderr, "Error (EncodeILinesData): unexpected number of 'i' lines detected!\n");
			fprintf(stderr, "Error (EncodeILinesData): current number of 's' lines = %"PRIu32"\n", msab->sLinesData->nRows);
			fprintf(stderr, "Error (EncodeILinesData): current number of 'i' lines = %"PRIu32"\n", msab->iLinesData->nRows);
			fprintf(stderr, "Error (EncodeILinesData): expected number of 'i' lines = %"PRIu32"\n", msab->sLinesData->nRows-1);
			pthread_exit(NULL);
		}
		
		binaryFlag = 0x1;
		ComputePModel2(cModels->iLineFlagCModel, acModels->binaryACModel);
		acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
		// DEBUG MODE
		#ifdef DEBUG_1
			acEncodeBinary(aceDebuggers->aceILineFlag[threadNumber], acModels->binaryACModel, binaryFlag);
		#endif // DEBUG_1
		UpdateCModelCounter(cModels->iLineFlagCModel, binaryFlag);
		UpdateCModelIdx(cModels->iLineFlagCModel, binaryFlag);
		
			
		// Loop all 'i' lines
		for(row = 0; row != msab->iLinesData->nRows; ++row)
		{
			// Compute the 's' row
			sRow = row + 1;
			
			// No 'i' line was encoded before of the current source
			// It is necessary to encode both (left and right)
			if(msab->linesInfo->elements[sRow]->iStatus == 0x0)
			{
				//////////////////////////////////////////////////////// LEFT STATUS SYMBOL ENCODING BEGIN
				statusSymbol = msab->iLinesData->leftStatus[row];
				GetPModelIdx3(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, cModels->statusCModel);
				ComputePModel(cModels->statusCModel, acModels->statusACModel);
				ac_encode_symbol(acEncoder->globalEncoder, acModels->statusACModel, statusSymbol);
				// DEBUG MODE
				#ifdef DEBUG_1
					ac_encode_symbol(aceDebuggers->aceStatus[threadNumber], acModels->statusACModel, statusSymbol);
				#endif // DEBUG_1	
				UpdateCModelCounter(cModels->statusCModel, statusSymbol);
				ShiftBuffer(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, statusSymbol);
				//////////////////////////////////////////////////////// LEFT STATUS SYMBOL ENCODING END
				
				
				//////////////////////////////////////////////////////// LEFT COUNT ENCODING BEGIN
				count = msab->iLinesData->leftCount[row];
				if(count > fieldsLimits->maxCountValue)
				{
					fprintf(stderr, "Error (EncodeILinesData): left count value is too big!\n");
					fprintf(stderr, "Error (EncodeILinesData): left count value read %"PRIu32"\n", count);
					fprintf(stderr, "Error (EncodeILinesData): maximum count value allowed in this context %"PRIu32"\n", fieldsLimits->maxCountValue);
					fprintf(stderr, "Rerun the encoder using an higher '-mcv' paramater value ( > %"PRIu32").\n", count);
					pthread_exit(NULL);
				}
				
				// Encode the count value itself
				writeNBits(count, storageBitsInfo->storageBitsCountValue, acEncoder->globalEncoder, acModels->binaryUniformACModel);
				// DEBUG MODE
				#ifdef DEBUG_1
					writeNBits(count, storageBitsInfo->storageBitsCountValue, aceDebuggers->aceCounts[threadNumber], 
					acModels->binaryUniformACModel);
				#endif // DEBUG_1	
				//////////////////////////////////////////////////////// LEFT COUNT ENCODING END
				
				// Change the 'i' status flag
				//msab->linesInfo->elements[sRow]->iStatus = 0x1;
				
				// Store the last right status and count encoded
				msab->linesInfo->elements[sRow]->lastRightStatus = msab->iLinesData->rightStatus[row];
				msab->linesInfo->elements[sRow]->lastRightCount = msab->iLinesData->rightCount[row];
			}
			// The 'i' line of the current source was encoded before,
			// find out if the previous right status symbol and count are the same as the current
			// left status symbol and count
			else
			{
				//////////////////////////////////////////////////////// IRREGULAR LEFT STATUS SYMBOL ENCODING BEGIN
				statusSymbol = msab->iLinesData->leftStatus[row];
				if(statusSymbol != msab->linesInfo->elements[sRow]->lastRightStatus)
				{
					// Encode a binary stream that sinalizes that there is an irregularity in the status symbol
					binaryFlag = 0x1;
					globalInfo->totalIrregularStatusI[threadNumber]++;
					
					ComputePModel2(cModels->irregularStatusCModel, acModels->binaryACModel);
					acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
					// DEBUG MODE
					#ifdef DEBUG_1
						acEncodeBinary(aceDebuggers->aceIrregularStatus[threadNumber], acModels->binaryACModel, binaryFlag);
					#endif // DEBUG_1
					UpdateCModelCounter(cModels->irregularStatusCModel, binaryFlag);
					UpdateCModelIdx(cModels->irregularStatusCModel, binaryFlag);
					
					
					// Encode the left status symbol
					GetPModelIdx3(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, cModels->statusCModel);
					ComputePModel(cModels->statusCModel, acModels->statusACModel);
					ac_encode_symbol(acEncoder->globalEncoder, acModels->statusACModel, statusSymbol);
					// DEBUG MODE
					#ifdef DEBUG_1
						ac_encode_symbol(aceDebuggers->aceStatus[threadNumber], acModels->statusACModel, statusSymbol);
					#endif // DEBUG_1	
					UpdateCModelCounter(cModels->statusCModel, statusSymbol);			
					ShiftBuffer(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, statusSymbol);
				}
				else
				{
					// Encode a binary flag that sinalizes that the previous right status symbol
					// is equal to the current left count
					binaryFlag = 0x0;
					ComputePModel2(cModels->irregularStatusCModel, acModels->binaryACModel);
					acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
					// DEBUG MODE
					#ifdef DEBUG_1
						acEncodeBinary(aceDebuggers->aceIrregularStatus[threadNumber], acModels->binaryACModel, binaryFlag);
					#endif // DEBUG_1
					UpdateCModelCounter(cModels->irregularStatusCModel, binaryFlag);
					UpdateCModelIdx(cModels->irregularStatusCModel, binaryFlag);
				}
				//////////////////////////////////////////////////////// IRREGULAR LEFT STATUS SYMBOL ENCODING END	
				
				
				//////////////////////////////////////////////////////// IRREGULAR LEFT COUNT ENCODING BEGIN
				count = msab->iLinesData->leftCount[row];
				if(count > fieldsLimits->maxCountValue)
				{
					fprintf(stderr, "Error (EncodeILinesData): left count value is too big!\n");
					fprintf(stderr, "Error (EncodeILinesData): left count value read %"PRIu32"\n", count);
					fprintf(stderr, "Error (EncodeILinesData): maximum count value allowed in this context %"PRIu32"\n", fieldsLimits->maxCountValue);
					fprintf(stderr, "Rerun the encoder using an higher '-mcv' paramater value ( > %"PRIu32").\n", count);
					pthread_exit(NULL);
				}
				if(count != msab->linesInfo->elements[sRow]->lastRightCount)
				{
					// Encode a binary stream that sinalizes that there is an irregularity in the count value
					binaryFlag = 0x1;
					globalInfo->totalIrregularCounts[threadNumber]++;
					
					ComputePModel2(cModels->irregularCountCModel, acModels->binaryACModel);
					acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
					// DEBUG MODE
					#ifdef DEBUG_1
						acEncodeBinary(aceDebuggers->aceIrregularCounts[threadNumber], acModels->binaryACModel, binaryFlag);
					#endif // DEBUG_1
					UpdateCModelCounter(cModels->irregularCountCModel, binaryFlag);
					UpdateCModelIdx(cModels->irregularCountCModel, binaryFlag);
					
					
					// Encode the left count value itself
					writeNBits(count, storageBitsInfo->storageBitsCountValue, acEncoder->globalEncoder, acModels->binaryUniformACModel);
					// DEBUG MODE
					#ifdef DEBUG_1
						writeNBits(count, storageBitsInfo->storageBitsCountValue, aceDebuggers->aceCounts[threadNumber],
							acModels->binaryUniformACModel);
					#endif // DEBUG_1	
						
				}	
				else
				{
					// Encode a binary flag that sinalizes that the previous right count is equal
					// to the current left count
					binaryFlag = 0x0;
					ComputePModel2(cModels->irregularCountCModel, acModels->binaryACModel);
					acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
					// DEBUG MODE
					#ifdef DEBUG_1
						acEncodeBinary(aceDebuggers->aceIrregularCounts[threadNumber], acModels->binaryACModel, binaryFlag);
					#endif // DEBUG_1
					UpdateCModelCounter(cModels->irregularCountCModel, binaryFlag);
					UpdateCModelIdx(cModels->irregularCountCModel, binaryFlag);
					
				}
				//////////////////////////////////////////////////////// IRREGULAR LEFT COUNT ENCODING END
			}
			
			// The right status symbol and count must be always encoded even if a previous 'i' line 
			// was encoded before of the same source
			
			//////////////////////////////////////////////////////// RIGHT STATUS SYMBOL ENCODING BEGIN
			statusSymbol = msab->iLinesData->rightStatus[row];
			GetPModelIdx3(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, cModels->statusCModel);
			ComputePModel(cModels->statusCModel, acModels->statusACModel);
			ac_encode_symbol(acEncoder->globalEncoder, acModels->statusACModel, statusSymbol);
			// DEBUG MODE
			#ifdef DEBUG_1
				ac_encode_symbol(aceDebuggers->aceStatus[threadNumber], acModels->statusACModel, statusSymbol);
			#endif // DEBUG_1	
			UpdateCModelCounter(cModels->statusCModel, statusSymbol);
			ShiftBuffer(msab->iLinesData->statusBuffer, msab->iLinesData->bufSize, statusSymbol);	
			//////////////////////////////////////////////////////// RIGHT STATUS SYMBOL ENCODING END
			
			
			//////////////////////////////////////////////////////// RIGHT COUNT ENCODING BEGIN
			count = msab->iLinesData->rightCount[row];
			if(count > fieldsLimits->maxCountValue)
			{
				fprintf(stderr, "Error (EncodeILinesData): right count value is too big!\n");
				fprintf(stderr, "Error (EncodeILinesData): right count value read %"PRIu32"\n", count);
				fprintf(stderr, "Error (EncodeILinesData): maximum count value allowed in this context %"PRIu32"\n", fieldsLimits->maxCountValue);
				fprintf(stderr, "Rerun the encoder using an higher '-mcv' paramater value ( > %"PRIu32").\n", count);
				pthread_exit(NULL);
			}
			
			// Encode the count value itself
			writeNBits(count, storageBitsInfo->storageBitsCountValue, acEncoder->globalEncoder, acModels->binaryUniformACModel);
			// DEBUG MODE
			#ifdef DEBUG_1
				writeNBits(count, storageBitsInfo->storageBitsCountValue, aceDebuggers->aceCounts[threadNumber],
					acModels->binaryUniformACModel);
			#endif // DEBUG_1	
			//////////////////////////////////////////////////////// RIGHT COUNT ENCODING END
			
			// Change the status for the current 'i' line		
			msab->linesInfo->elements[sRow]->iStatus = 0x1;
			// Update the last right status symbol
			msab->linesInfo->elements[sRow]->lastRightStatus = statusSymbol;
			msab->linesInfo->elements[sRow]->lastRightCount = count;
		}
		
	}
	//////////////////////////////////////////////////////// I LINES ENCODING END
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


void EncodeELinesData(uint8_t threadNumber, MSAB *msab, ACEncoder *acEncoder, 
	ACModels *acModels, CModels *cModels, StorageBitsInfo *storageBitsInfo)
{
	uint8_t binaryFlag, statusSymbol, strandSymbol;
	uint32_t row;
	size_t sourceNameSize;
	
	//////////////////////////////////////////////////////// E LINES ENCODING BEGIN
	// The first thing to do is to verify if there is actually any 'e' lines to encode
	if(msab->eLinesData->nRows == 0)
	{
		// Sinalize to the decoder that there are not any 'e' line in the current MSAB
		binaryFlag = 0x0;
		ComputePModel2(cModels->eLineFlagCModel, acModels->binaryACModel);
		acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
		// DEBUG MODE
		#ifdef DEBUG_1
			acEncodeBinary(aceDebuggers->aceELineFlag[threadNumber], acModels->binaryACModel, binaryFlag);
		#endif // DEBUG_1
		UpdateCModelCounter(cModels->eLineFlagCModel, binaryFlag);
		UpdateCModelIdx(cModels->eLineFlagCModel, binaryFlag);
		
		// No e lines in the current MSAB so we need to set the current number of eLinesInfo
		// to zero!
		msab->eLinesInfo->currentNRows = 0;
	}
	// There are 'e' lines to encode
	else
	{
		// Send a 0x1 flag that indicates that there is 'e' lines in the current MSAB
		binaryFlag = 0x1;
		ComputePModel2(cModels->eLineFlagCModel, acModels->binaryACModel);
		acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
		
		// DEBUG MODE
		#ifdef DEBUG_1
			acEncodeBinary(aceDebuggers->aceELineFlag[threadNumber], 
				acModels->binaryACModel, binaryFlag);
		#endif // DEBUG_1
		UpdateCModelCounter(cModels->eLineFlagCModel, binaryFlag);
		UpdateCModelIdx(cModels->eLineFlagCModel, binaryFlag);
		
			
		// The first thing to do is to encode the number of 'e' lines that are 
		// in the current MSAB
		// Encode the number of 'e' lines
		writeNBits(msab->eLinesData->nRows, storageBitsInfo->storageBitsMSABRows, 
			acEncoder->globalEncoder, acModels->binaryUniformACModel);
		
		// DEBUG MODE
		#ifdef DEBUG_1
			writeNBits(msab->eLinesData->nRows, storageBitsInfo->storageBitsMSABRows, 
				aceDebuggers->aceENRows[threadNumber], acModels->binaryUniformACModel);			
		#endif // DEBUG_1
				
		// Loop all 'e' lines	
		for(row = 0; row != msab->eLinesData->nRows; ++row)
		{
			// First encode the hash information
			//////////////////////////////////////////////////////// HASH KEY AND ELEMENT ID ENCODING BEGIN
			writeNBits(msab->eLinesInfo->hashPositions[row].hashKey, 
				storageBitsInfo->storageBitsHashKeyValue, acEncoder->globalEncoder, acModels->binaryUniformACModel);
			writeNBits(msab->eLinesInfo->hashPositions[row].elementId, 
					storageBitsInfo->storageBitsHashElementId, acEncoder->globalEncoder, acModels->binaryUniformACModel);
					
			// DEBUG MODE
			#ifdef DEBUG_1
				writeNBits(msab->eLinesInfo->hashPositions[row].hashKey, 
					storageBitsInfo->storageBitsHashKeyValue, aceDebuggers->aceHashInfo[threadNumber], 
					acModels->binaryUniformACModel);
				writeNBits(msab->eLinesInfo->hashPositions[row].elementId, 
					storageBitsInfo->storageBitsHashElementId, aceDebuggers->aceHashInfo[threadNumber], 
					acModels->binaryUniformACModel);	
			#endif // DEBUG_1
			//////////////////////////////////////////////////////// HASH KEY AND ELEMENT ID ENCODING END
			
			
			// Get the status symbol
			statusSymbol = msab->eLinesData->status[row];	 
			
			// If the element was not encoded, it is necessary to encode all the information
			// source name, start position, etc.
			if(msab->eLinesInfo->elements[row]->sStatus == 0x0)
			{
				//////////////////////////////////////////////////////// SOURCE NAME ENCODING BEGIN
				// Encode the size of the string
				sourceNameSize = Strlen(msab->eLinesInfo->elements[row]->sourceName);
				writeNBits(sourceNameSize, 
					storageBitsInfo->storageBitsSrcName, acEncoder->globalEncoder, 
					acModels->binaryUniformACModel);
				writeString(msab->eLinesInfo->elements[row]->sourceName, 
					sourceNameSize, acEncoder->globalEncoder, acModels->binaryUniformACModel);
				
				// DEBUG MODE
				#ifdef DEBUG_1
					writeNBits(sourceNameSize, 
						storageBitsInfo->storageBitsSrcName, aceDebuggers->aceESourceName[threadNumber], 
						acModels->binaryUniformACModel);
					writeString(msab->eLinesInfo->elements[row]->sourceName, 
						sourceNameSize, aceDebuggers->aceESourceName[threadNumber], acModels->binaryUniformACModel);
				#endif // DEBUG_1				
				//////////////////////////////////////////////////////// SOURCE NAME ENCODING END	
			

				//////////////////////////////////////////////////////// START POSITION ENCODING BEGIN
				writeNBits(msab->eLinesInfo->elements[row]->start, 
					storageBitsInfo->storageBitsStartPosition,	acEncoder->globalEncoder, 
					acModels->binaryUniformACModel);
				
				// DEBUG MODE
				#ifdef DEBUG_1
					writeNBits(msab->eLinesInfo->elements[row]->start, 
						storageBitsInfo->storageBitsStartPosition,	aceDebuggers->aceEStartPosRaw[threadNumber], 
						acModels->binaryUniformACModel);
				#endif // DEBUG_1
				//////////////////////////////////////////////////////// START POSITION ENCODING END


				//////////////////////////////////////////////////////// SEQ SIZE POSITION ENCODING BEGIN
				writeNBits(msab->eLinesInfo->elements[row]->seqSize, storageBitsInfo->storageBitsMSABCols,
					acEncoder->globalEncoder, acModels->binaryUniformACModel);
				// DEBUG MODE
				#ifdef DEBUG_1
					writeNBits(msab->eLinesInfo->elements[row]->seqSize, storageBitsInfo->storageBitsMSABCols,
						aceDebuggers->aceESeqSize[threadNumber], acModels->binaryUniformACModel);
				#endif // DEBUG_1
				//////////////////////////////////////////////////////// SEQ SIZE POSITION ENCODING END

				//////////////////////////////////////////////////////// STRAND ENCODING BEGIN
				strandSymbol = msab->eLinesInfo->elements[row]->strand;
				ComputePModel2(cModels->strandCModel, acModels->binaryACModel);
				acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, strandSymbol);

				// DEBUG MODE
				#ifdef DEBUG_1
					acEncodeBinary(aceDebuggers->aceEStrand[threadNumber], acModels->binaryACModel, strandSymbol);
				#endif // DEBUG_1
				UpdateCModelCounter(cModels->strandCModel, strandSymbol);
				UpdateCModelIdx(cModels->strandCModel, strandSymbol);
				//////////////////////////////////////////////////////// STRAND ENCODING END


				//////////////////////////////////////////////////////// SOURCE SIZE ENCODING BEGIN
				writeNBits(msab->eLinesInfo->elements[row]->sourceSize, 
					storageBitsInfo->storageBitsSourceSize, acEncoder->globalEncoder, 
					acModels->binaryUniformACModel);
				
				// DEBUG MODE
				#ifdef DEBUG_1
					writeNBits(msab->eLinesInfo->elements[row]->sourceSize, 
						storageBitsInfo->storageBitsSourceSize, aceDebuggers->aceESourceSize[threadNumber], 
						acModels->binaryUniformACModel);
				#endif // DEBUG_1				
				//////////////////////////////////////////////////////// SOURCE SIZE ENCODING END	


				//////////////////////////////////////////////////////// STATUS SYMBOL ENCODING BEGIN
				//statusSymbol = msab->eLinesData->status[row];
				GetPModelIdx3(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, cModels->eLineStatusCModel);
				ComputePModel(cModels->eLineStatusCModel, acModels->statusACModel);
				ac_encode_symbol(acEncoder->globalEncoder, acModels->statusACModel, statusSymbol);
				// DEBUG MODE
				#ifdef DEBUG_1
					ac_encode_symbol(aceDebuggers->aceELineStatus[threadNumber], 
						acModels->statusACModel, statusSymbol);
				#endif // DEBUG_1
				UpdateCModelCounter(cModels->eLineStatusCModel, statusSymbol);
				ShiftBuffer(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, statusSymbol);
				//////////////////////////////////////////////////////// STATUS SYMBOL ENCODING END				
				// Change the element status flag in order to avoid encoding extra information
				// next time this source appears
				msab->eLinesInfo->elements[row]->sStatus = 0x1;
			}
			// The current source information was already encoded before
			else
			{
				
				// The previous MSAB had an 'e' line of this same source?
				if(msab->eLinesInfo->elements[row]->closeELine == 0x1)
				{
					// Verify if the status symbol is the same as the previous one encoded
					// for the same source
					if(statusSymbol != msab->eLinesInfo->elements[row]->lastStatusSymbol)
					{
						
						// Sinalize to the decoder the presence of an irregular status symbol
						binaryFlag = 0x1;
						ComputePModel2(cModels->eLineIrregularStatusCModel, acModels->binaryACModel);
						acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
						// DEBUG MODE
						#ifdef DEBUG_1
						acEncodeBinary(aceDebuggers->aceELineIrregularStatus[threadNumber], 
							acModels->binaryACModel, binaryFlag);
						#endif // DEBUG_1
						UpdateCModelCounter(cModels->eLineIrregularStatusCModel, binaryFlag);
						UpdateCModelIdx(cModels->eLineIrregularStatusCModel, binaryFlag);
						
						// Encode the irregular status symbol itself
						//////////////////////////////////////////////////////// STATUS SYMBOL ENCODING BEGIN
						GetPModelIdx3(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, cModels->eLineStatusCModel);
						ComputePModel(cModels->eLineStatusCModel, acModels->statusACModel);
						ac_encode_symbol(acEncoder->globalEncoder, acModels->statusACModel, statusSymbol);
						// DEBUG MODE
						#ifdef DEBUG_1
							ac_encode_symbol(aceDebuggers->aceELineStatus[threadNumber], 
								acModels->statusACModel, statusSymbol);
						#endif // DEBUG_1
						UpdateCModelCounter(cModels->eLineStatusCModel, statusSymbol);
						ShiftBuffer(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, statusSymbol);
						//////////////////////////////////////////////////////// STATUS SYMBOL ENCODING END	
						
						// Update the number of irregular status symbols in 'e' lines
						globalInfo->totalIrregularStatusE[threadNumber]++;
					}
					else
					{
						// Sinalize to the decoder the presence of a regular status symbol
						binaryFlag = 0x0;

						ComputePModel2(cModels->eLineIrregularStatusCModel, acModels->binaryACModel);
						acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
						// DEBUG MODE
						#ifdef DEBUG_1
						acEncodeBinary(aceDebuggers->aceELineIrregularStatus[threadNumber], 
							acModels->binaryACModel, binaryFlag);
						#endif // DEBUG_1
						UpdateCModelCounter(cModels->eLineIrregularStatusCModel, binaryFlag);
						UpdateCModelIdx(cModels->eLineIrregularStatusCModel, binaryFlag);
						
					}
				}
				// There are not any 'e' lines of the current source in the previous MSAB, however
				// the status symbol can be obtained by the last 'i' line right status. Despite this,
				// it is necessary to test here in the encoder if the current status is a 
				// regular (it is the same as the last right status symbol of the last 'i' line) or 
				// irregular (different from the previous right status symbol of the last 'i' line) 
				// symbol.
				
				// There are 'i' lines of this source encoded before
				else if(msab->eLinesInfo->elements[row]->iStatus == 0x1)
				{
					// Is this an irregular status symbol?
					if(statusSymbol != msab->eLinesInfo->elements[row]->lastRightStatus)
					{	
						// Sinalize to the decoder the presence of an irregular status symbol
						binaryFlag = 0x1;
						ComputePModel2(cModels->eLineIrregularStatusCModel, acModels->binaryACModel);
						acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
						// DEBUG MODE
						#ifdef DEBUG_1
						acEncodeBinary(aceDebuggers->aceELineIrregularStatus[threadNumber], 
							acModels->binaryACModel, binaryFlag);
						#endif // DEBUG_1
						UpdateCModelCounter(cModels->eLineIrregularStatusCModel, binaryFlag);
						UpdateCModelIdx(cModels->eLineIrregularStatusCModel, binaryFlag);

						// Encode the irregular status symbol itself
						//////////////////////////////////////////////////////// STATUS SYMBOL ENCODING BEGIN
						GetPModelIdx3(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, cModels->eLineStatusCModel);
						ComputePModel(cModels->eLineStatusCModel, acModels->statusACModel);
						ac_encode_symbol(acEncoder->globalEncoder, acModels->statusACModel, statusSymbol);
						// DEBUG MODE
						#ifdef DEBUG_1
							ac_encode_symbol(aceDebuggers->aceELineStatus[threadNumber], 
								acModels->statusACModel, statusSymbol);
						#endif // DEBUG_1
						UpdateCModelCounter(cModels->eLineStatusCModel, statusSymbol);
						ShiftBuffer(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, statusSymbol);
						//////////////////////////////////////////////////////// STATUS SYMBOL ENCODING END	
						
						// Update the number of irregular status symbols in 'e' lines
						globalInfo->totalIrregularStatusEI[threadNumber]++;
					}
					// Regular status symbol
					else
					{
						// Sinalize to the decoder the presence of a regular status symbol
						binaryFlag = 0x0;
						ComputePModel2(cModels->eLineIrregularStatusCModel, acModels->binaryACModel);
						acEncodeBinary(acEncoder->globalEncoder, acModels->binaryACModel, binaryFlag);
						// DEBUG MODE
						#ifdef DEBUG_1
						acEncodeBinary(aceDebuggers->aceELineIrregularStatus[threadNumber], 
							acModels->binaryACModel, binaryFlag);
						#endif // DEBUG_1
						UpdateCModelCounter(cModels->eLineIrregularStatusCModel, binaryFlag);
						UpdateCModelIdx(cModels->eLineIrregularStatusCModel, binaryFlag);
					}
				}
				// If there are not any 'e' line of the current source in the previous MSAB
				// and for some reason there are no 'i' lines encoded before for the same source
				// it is necessary to send the status symbol to the decoder in order to
				// not affect the performance of the models of irregular status symbols
				// Compress the status symbol
				else
				{
					//////////////////////////////////////////////////////// STATUS SYMBOL ENCODING BEGIN
					GetPModelIdx3(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, cModels->eLineStatusCModel);
					ComputePModel(cModels->eLineStatusCModel, acModels->statusACModel);
					ac_encode_symbol(acEncoder->globalEncoder, acModels->statusACModel, statusSymbol);
					// DEBUG MODE
					#ifdef DEBUG_1
						ac_encode_symbol(aceDebuggers->aceELineStatus[threadNumber], 
							acModels->statusACModel, statusSymbol);
					#endif // DEBUG_1
					UpdateCModelCounter(cModels->eLineStatusCModel, statusSymbol);
					ShiftBuffer(msab->eLinesData->statusBuffer, msab->eLinesData->bufSize, statusSymbol);
					//////////////////////////////////////////////////////// STATUS SYMBOL ENCODING END	
				}
			}
			
			// Update the lastStatus symbol
			msab->eLinesInfo->elements[row]->lastStatusSymbol = statusSymbol;
		}	
		
	}	
	//////////////////////////////////////////////////////// E LINES ENCODING END
	
	// Reset close 'e' lines flag and update the prevELinesInfo field
	ResetELinesInfo(msab);
	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CreateGlobalInfo(uint8_t nThreads)
{
	globalInfo = (GlobalInfo *)Calloc(1, sizeof(GlobalInfo));
	
	globalInfo->totalBlocks = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalLowerCaseBlocks = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalXchrBlocks = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));

	globalInfo->totalSLines = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalQLines = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalILines = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalELines = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	
	globalInfo->totalBases = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalGaps = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalLowerCaseSymbols = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalXcharN = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalXcharNC = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	
	globalInfo->totalNumNegativeOff = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalNumLargeOff = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	
	globalInfo->totalIrregularStatusI = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalIrregularCounts = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	
	globalInfo->totalIrregularStatusE = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	globalInfo->totalIrregularStatusEI = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		
	globalInfo->totalBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));

	globalInfo->maxStartOffset = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeGlobalInfo(uint8_t nThreads)
{
	
	Free(globalInfo->totalBlocks, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalLowerCaseBlocks, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalXchrBlocks, (nThreads+1)*sizeof(uint64_t));
	
	Free(globalInfo->totalSLines, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalQLines, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalILines, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalELines, (nThreads+1)*sizeof(uint64_t));
	
	Free(globalInfo->totalBases, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalGaps, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalLowerCaseSymbols, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalXcharN, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalXcharNC, (nThreads+1)*sizeof(uint64_t));	
	
	Free(globalInfo->totalNumNegativeOff, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalNumLargeOff, (nThreads+1)*sizeof(uint64_t));
	
	Free(globalInfo->totalIrregularStatusI, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalIrregularCounts, (nThreads+1)*sizeof(uint64_t));
	
	Free(globalInfo->totalIrregularStatusE, (nThreads+1)*sizeof(uint64_t));
	Free(globalInfo->totalIrregularStatusEI, (nThreads+1)*sizeof(uint64_t));
	
	Free(globalInfo->totalBits, (nThreads+1)*sizeof(uint64_t));
	
	Free(globalInfo->maxStartOffset, (nThreads+1)*sizeof(uint64_t));
	
	Free(globalInfo, sizeof(GlobalInfo));
	globalInfo = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ComputeGlobalInfo(uint8_t nThreads)
{
	uint8_t n;
	for(n = 0; n != nThreads; ++n)
	{
		globalInfo->totalBlocks[nThreads] += globalInfo->totalBlocks[n];
		globalInfo->totalLowerCaseBlocks[nThreads] += globalInfo->totalLowerCaseBlocks[n];
		globalInfo->totalXchrBlocks[nThreads] += globalInfo->totalXchrBlocks[n];
		
		globalInfo->totalSLines[nThreads] += globalInfo->totalSLines[n];
		globalInfo->totalQLines[nThreads] += globalInfo->totalQLines[n];
		globalInfo->totalILines[nThreads] += globalInfo->totalILines[n];
		globalInfo->totalELines[nThreads] += globalInfo->totalELines[n];
		
		globalInfo->totalBases[nThreads] += globalInfo->totalBases[n];
		globalInfo->totalGaps[nThreads] += globalInfo->totalGaps[n];
		globalInfo->totalLowerCaseSymbols[nThreads] += globalInfo->totalLowerCaseSymbols[n];
		globalInfo->totalXcharN[nThreads] += globalInfo->totalXcharN[n];
		globalInfo->totalXcharNC[nThreads] += globalInfo->totalXcharNC[n];
		
		globalInfo->totalNumNegativeOff[nThreads] += globalInfo->totalNumNegativeOff[n];
		globalInfo->totalNumLargeOff[nThreads] += globalInfo->totalNumLargeOff[n];
		
		globalInfo->totalIrregularStatusI[nThreads] += globalInfo->totalIrregularStatusI[n];
		globalInfo->totalIrregularCounts[nThreads] += globalInfo->totalIrregularCounts[n];
		
		globalInfo->totalIrregularStatusE[nThreads] += globalInfo->totalIrregularStatusE[n];
		globalInfo->totalIrregularStatusEI[nThreads] += globalInfo->totalIrregularStatusEI[n];
		
		globalInfo->totalBits[nThreads] += globalInfo->totalBits[n];
		
		globalInfo->maxStartOffset[nThreads] = ( (globalInfo->maxStartOffset[n] > globalInfo->maxStartOffset[nThreads]) ? 
			globalInfo->maxStartOffset[n] : globalInfo->maxStartOffset[nThreads]);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintGlobalInfo(uint8_t nThreads)
{
	printf("+-------------------------------------------------------------------------+\n");
	printf("| Total block read:                                   %19"PRIu64" |\n", globalInfo->totalBlocks[nThreads]);
	printf("| Total block with lower case symbols:                %19"PRIu64" |\n", globalInfo->totalLowerCaseBlocks[nThreads]);
	printf("| Total block with extra symbols (N's and n's):       %19"PRIu64" |\n", globalInfo->totalXchrBlocks[nThreads]);
	printf("+-------------------------------------------------------------------------+\n");
	printf("| Total number of 's' lines:                          %19"PRIu64" |\n", globalInfo->totalSLines[nThreads]);
	printf("| Total number of 'q' lines:                          %19"PRIu64" |\n", globalInfo->totalQLines[nThreads]);
	printf("| Total number of 'i' lines:                          %19"PRIu64" |\n", globalInfo->totalILines[nThreads]);
	printf("| Total number of 'e' lines:                          %19"PRIu64" |\n", globalInfo->totalELines[nThreads]);
	printf("+-------------------------------------------------------------------------+\n");
	printf("| Total number of bases (with the gaps)               %19"PRIu64" |\n", globalInfo->totalBases[nThreads]);
	printf("| Total number of gaps                                %19"PRIu64" |\n", globalInfo->totalGaps[nThreads]);
	printf("| Total number of lower case bases                    %19"PRIu64" |\n", globalInfo->totalLowerCaseSymbols[nThreads]);
	printf("| Total number of extra symbols {N and n}             %19"PRIu64" |\n", globalInfo->totalXcharN[nThreads]);
	printf("| Total number of extra symbols {N, n, C, c}          %19"PRIu64" |\n", globalInfo->totalXcharNC[nThreads]);
	printf("+-------------------------------------------------------------------------+\n");
	printf("| Total number of negative start offsets              %19"PRIu64" |\n", globalInfo->totalNumNegativeOff[nThreads]);
	printf("| Total number of large start offsets                 %19"PRIu64" |\n", globalInfo->totalNumLargeOff[nThreads]);
	printf("| Maximum start offset value processed                %19"PRIu64" |\n", globalInfo->maxStartOffset[nThreads]);
	printf("+-------------------------------------------------------------------------+\n");
	printf("| Total number of irregular status symbols 'i'        %19"PRIu64" |\n", globalInfo->totalIrregularStatusI[nThreads]);
	printf("| Total number of irregular count values 'i'          %19"PRIu64" |\n", globalInfo->totalIrregularCounts[nThreads]);
	printf("+-------------------------------------------------------------------------+\n");
	printf("| Total number of irregular status symbols 'e'        %19"PRIu64" |\n", globalInfo->totalIrregularStatusE[nThreads]);
	printf("| Total number of irregular status symbols 'e' - 'i'  %19"PRIu64" |\n", globalInfo->totalIrregularStatusEI[nThreads]);
	printf("+-------------------------------------------------------------------------+\n");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		
#ifdef DEBUG_1

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void CreateACEDebugger(uint8_t nThreads)
	{
		uint8_t n;
		
		aceDebuggers = (AceDebuggers *)Calloc(1, sizeof(AceDebuggers));
		
		aceDebuggers->aceHeader = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceScore = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceSNRows = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceSNCols = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceSourceName = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceSLines = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceCase = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceCaseFlag = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceXchr = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceXchrFlag = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceStartPosRaw = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceStartOffsetSign = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceStartOffset = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceStrand = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceSourceSize = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		
		aceDebuggers->aceQLines = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceQLineFlag = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceQLineInFlag = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		
		aceDebuggers->aceILineFlag = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceStatus = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceCounts = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceIrregularStatus = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceIrregularCounts = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		
		aceDebuggers->aceELineFlag = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceENRows = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceESourceName = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceEStartPosRaw = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceEStartOffsetSign = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceEStartOffset = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceESeqSize = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceEStrand = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceESourceSize = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceELineStatus = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		aceDebuggers->aceELineIrregularStatus = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
		
		aceDebuggers->aceHashInfo = (ac_encoder **)Calloc(nThreads, sizeof(ac_encoder *));
				
		for(n = 0; n < nThreads; n++)
		{
			aceDebuggers->aceHeader[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceScore[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceSNRows[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceSNCols[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceSourceName[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceSLines[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceCase[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceCaseFlag[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceXchr[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceXchrFlag[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceStartPosRaw[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceStartOffsetSign[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceStartOffset[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceStrand[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceSourceSize[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			
			aceDebuggers->aceQLines[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceQLineFlag[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceQLineInFlag[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			
			aceDebuggers->aceILineFlag[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceStatus[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceCounts[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceIrregularStatus[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceIrregularCounts[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			
			aceDebuggers->aceELineFlag[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceENRows[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceESourceName[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceEStartPosRaw[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceEStartOffsetSign[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceEStartOffset[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceESeqSize[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceEStrand[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceESourceSize[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceELineStatus[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			aceDebuggers->aceELineIrregularStatus[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			
			aceDebuggers->aceHashInfo[n] = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
			
			ac_encoder_init(aceDebuggers->aceHeader[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceScore[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceSNRows[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceSNCols[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceSourceName[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceSLines[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceCase[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceCaseFlag[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceXchr[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceXchrFlag[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceStartPosRaw[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceStartOffsetSign[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceStartOffset[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceStrand[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceSourceSize[n], DEFAULT_ACE_DEBUGGER_FILE);
			
			ac_encoder_init(aceDebuggers->aceQLines[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceQLineFlag[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceQLineInFlag[n], DEFAULT_ACE_DEBUGGER_FILE);
			
			ac_encoder_init(aceDebuggers->aceILineFlag[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceStatus[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceCounts[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceIrregularStatus[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceIrregularCounts[n], DEFAULT_ACE_DEBUGGER_FILE);
			
			ac_encoder_init(aceDebuggers->aceELineFlag[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceENRows[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceESourceName[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceEStartPosRaw[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceEStartOffsetSign[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceEStartOffset[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceESeqSize[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceEStrand[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceESourceSize[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceELineStatus[n], DEFAULT_ACE_DEBUGGER_FILE);
			ac_encoder_init(aceDebuggers->aceELineIrregularStatus[n], DEFAULT_ACE_DEBUGGER_FILE);
			
			ac_encoder_init(aceDebuggers->aceHashInfo[n], DEFAULT_ACE_DEBUGGER_FILE);
		}
	}
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	void FreeACEDebugger(uint8_t nThreads)
	{
		uint8_t n;
		for(n = 0; n < nThreads; n++)
		{
			Free(aceDebuggers->aceHeader[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceScore[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceSNRows[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceSNCols[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceSourceName[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceSLines[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceCase[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceCaseFlag[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceXchr[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceXchrFlag[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceStartPosRaw[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceStartOffsetSign[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceStartOffset[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceStrand[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceSourceSize[n], sizeof(ac_encoder));
			
			Free(aceDebuggers->aceQLines[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceQLineFlag[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceQLineInFlag[n], sizeof(ac_encoder));
			
			Free(aceDebuggers->aceILineFlag[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceStatus[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceCounts[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceIrregularStatus[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceIrregularCounts[n], sizeof(ac_encoder));
			
			Free(aceDebuggers->aceELineFlag[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceENRows[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceESourceName[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceEStartPosRaw[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceEStartOffsetSign[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceEStartOffset[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceESeqSize[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceEStrand[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceESourceSize[n], sizeof(ac_encoder));			
			Free(aceDebuggers->aceELineStatus[n], sizeof(ac_encoder));
			Free(aceDebuggers->aceELineIrregularStatus[n], sizeof(ac_encoder));
			
			Free(aceDebuggers->aceHashInfo[n], sizeof(ac_encoder));
		}
		
		Free(aceDebuggers->aceHeader, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceScore, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceSNRows, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceSNCols, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceSourceName, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceSLines, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceCase, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceCaseFlag, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceXchr, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceXchrFlag, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceStartPosRaw, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceStartOffsetSign, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceStartOffset, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceStrand, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceSourceSize, nThreads*sizeof(ac_encoder *));

		Free(aceDebuggers->aceQLines, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceQLineFlag, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceQLineInFlag, nThreads*sizeof(ac_encoder *));
		
		Free(aceDebuggers->aceILineFlag, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceStatus, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceCounts, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceIrregularStatus, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceIrregularCounts, nThreads*sizeof(ac_encoder *));
		
		Free(aceDebuggers->aceELineFlag, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceENRows, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceESourceName, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceEStartPosRaw, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceEStartOffsetSign, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceEStartOffset, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceESeqSize, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceEStrand, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceESourceSize, nThreads*sizeof(ac_encoder *));		
		Free(aceDebuggers->aceELineStatus, nThreads*sizeof(ac_encoder *));
		Free(aceDebuggers->aceELineIrregularStatus, nThreads*sizeof(ac_encoder *));
		
		Free(aceDebuggers->aceHashInfo, nThreads*sizeof(ac_encoder *));

		Free(aceDebuggers, sizeof(AceDebuggers));
		aceDebuggers = NULL;
	}
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void ACEDebuggersDone(uint8_t nThreads)
	{
		uint8_t n;
		
		for(n = 0; n < nThreads; n++)
		{
			ac_encoder_done(aceDebuggers->aceHeader[n]);
			ac_encoder_done(aceDebuggers->aceScore[n]);
			ac_encoder_done(aceDebuggers->aceSNRows[n]);
			ac_encoder_done(aceDebuggers->aceSNCols[n]);
			ac_encoder_done(aceDebuggers->aceSourceName[n]);
			ac_encoder_done(aceDebuggers->aceSLines[n]);
			ac_encoder_done(aceDebuggers->aceCase[n]);
			ac_encoder_done(aceDebuggers->aceCaseFlag[n]);
			ac_encoder_done(aceDebuggers->aceXchr[n]);
			ac_encoder_done(aceDebuggers->aceXchrFlag[n]);
			ac_encoder_done(aceDebuggers->aceStartPosRaw[n]);
			ac_encoder_done(aceDebuggers->aceStartOffsetSign[n]);
			ac_encoder_done(aceDebuggers->aceStartOffset[n]);
			ac_encoder_done(aceDebuggers->aceStrand[n]);
			ac_encoder_done(aceDebuggers->aceSourceSize[n]);
			
			ac_encoder_done(aceDebuggers->aceQLines[n]);
			ac_encoder_done(aceDebuggers->aceQLineFlag[n]);
			ac_encoder_done(aceDebuggers->aceQLineInFlag[n]);
			
			ac_encoder_done(aceDebuggers->aceILineFlag[n]);
			ac_encoder_done(aceDebuggers->aceStatus[n]);
			ac_encoder_done(aceDebuggers->aceCounts[n]);
			ac_encoder_done(aceDebuggers->aceIrregularStatus[n]);
			ac_encoder_done(aceDebuggers->aceIrregularCounts[n]);
			
			ac_encoder_done(aceDebuggers->aceELineFlag[n]);
			ac_encoder_done(aceDebuggers->aceENRows[n]);
			ac_encoder_done(aceDebuggers->aceESourceName[n]);
			ac_encoder_done(aceDebuggers->aceEStartPosRaw[n]);
			ac_encoder_done(aceDebuggers->aceEStartOffsetSign[n]);
			ac_encoder_done(aceDebuggers->aceEStartOffset[n]);
			ac_encoder_done(aceDebuggers->aceESeqSize[n]);
			ac_encoder_done(aceDebuggers->aceEStrand[n]);
			ac_encoder_done(aceDebuggers->aceESourceSize[n]);
			ac_encoder_done(aceDebuggers->aceELineStatus[n]);
			ac_encoder_done(aceDebuggers->aceELineIrregularStatus[n]);
			
			ac_encoder_done(aceDebuggers->aceHashInfo[n]);
			
			debugInfo->headerBits[nThreads] += ac_encoder_bits(aceDebuggers->aceHeader[n]);
			debugInfo->scoreBits[nThreads] += ac_encoder_bits(aceDebuggers->aceScore[n]);
			debugInfo->sNRowsBits[nThreads] += ac_encoder_bits(aceDebuggers->aceSNRows[n]);
			debugInfo->sNColsBits[nThreads] += ac_encoder_bits(aceDebuggers->aceSNCols[n]);
			debugInfo->sourceNameBits[nThreads] += ac_encoder_bits(aceDebuggers->aceSourceName[n]);
			debugInfo->sLinesBits[nThreads] += ac_encoder_bits(aceDebuggers->aceSLines[n]);
			debugInfo->caseBits[nThreads] += ac_encoder_bits(aceDebuggers->aceCase[n]);
			debugInfo->caseFlagBits[nThreads] += ac_encoder_bits(aceDebuggers->aceCaseFlag[n]);
			debugInfo->xchrBits[nThreads] += ac_encoder_bits(aceDebuggers->aceXchr[n]);
			debugInfo->xchrFlagBits[nThreads] += ac_encoder_bits(aceDebuggers->aceXchrFlag[n]);
			debugInfo->startPosRawBits[nThreads] += ac_encoder_bits(aceDebuggers->aceStartPosRaw[n]);
			debugInfo->startOffsetSignBits[nThreads] += ac_encoder_bits(aceDebuggers->aceStartOffsetSign[n]);
			debugInfo->startOffsetBits[nThreads] += ac_encoder_bits(aceDebuggers->aceStartOffset[n]);
			debugInfo->strandBits[nThreads] += ac_encoder_bits(aceDebuggers->aceStrand[n]);
			debugInfo->sourceSizeBits[nThreads] += ac_encoder_bits(aceDebuggers->aceSourceSize[n]);
			
			debugInfo->qLinesBits[nThreads] += ac_encoder_bits(aceDebuggers->aceQLines[n]);
			debugInfo->qLineFlagBits[nThreads] += ac_encoder_bits(aceDebuggers->aceQLineFlag[n]);
			debugInfo->qLineInFlagBits[nThreads] += ac_encoder_bits(aceDebuggers->aceQLineInFlag[n]);
			
			debugInfo->iLineFlagBits[nThreads] += ac_encoder_bits(aceDebuggers->aceILineFlag[n]);
			debugInfo->statusBits[nThreads] += ac_encoder_bits(aceDebuggers->aceStatus[n]);
			debugInfo->countBits[nThreads] += ac_encoder_bits(aceDebuggers->aceCounts[n]);
			debugInfo->irregularStatusBits[nThreads] += ac_encoder_bits(aceDebuggers->aceIrregularStatus[n]);
			debugInfo->irregularCountBits[nThreads] += ac_encoder_bits(aceDebuggers->aceIrregularCounts[n]);
			
			debugInfo->eLineFlagBits[nThreads] += ac_encoder_bits(aceDebuggers->aceELineFlag[n]);			
			debugInfo->eNRowsBits[nThreads] += ac_encoder_bits(aceDebuggers->aceENRows[n]);
			debugInfo->eSourceNameBits[nThreads] += ac_encoder_bits(aceDebuggers->aceESourceName[n]);
			debugInfo->eStartPosRawBits[nThreads] += ac_encoder_bits(aceDebuggers->aceEStartPosRaw[n]);
			debugInfo->eStartOffsetSignBits[nThreads] += ac_encoder_bits(aceDebuggers->aceEStartOffsetSign[n]);
			debugInfo->eStartOffsetBits[nThreads] += ac_encoder_bits(aceDebuggers->aceEStartOffset[n]);
			debugInfo->eSeqSizeBits[nThreads] += ac_encoder_bits(aceDebuggers->aceESeqSize[n]);
			debugInfo->eStrandBits[nThreads] += ac_encoder_bits(aceDebuggers->aceEStrand[n]);
			debugInfo->eSourceSizeBits[nThreads] += ac_encoder_bits(aceDebuggers->aceESourceSize[n]);
			debugInfo->eLineStatusBits[nThreads] += ac_encoder_bits(aceDebuggers->aceELineStatus[n]);
			debugInfo->eLineIrregularStatusBits[nThreads] += ac_encoder_bits(aceDebuggers->aceELineIrregularStatus[n]);
			
			debugInfo->hashInfoBits[nThreads] += ac_encoder_bits(aceDebuggers->aceHashInfo[n]);
		}
		
		debugInfo->totalBits += debugInfo->headerBits[nThreads];
		debugInfo->totalBits += debugInfo->scoreBits[nThreads];
		debugInfo->totalBits += debugInfo->sNRowsBits[nThreads];
		debugInfo->totalBits += debugInfo->sNColsBits[nThreads];
		debugInfo->totalBits += debugInfo->sourceNameBits[nThreads];
		debugInfo->totalBits += debugInfo->sLinesBits[nThreads];
		debugInfo->totalBits += debugInfo->caseBits[nThreads];
		debugInfo->totalBits += debugInfo->caseFlagBits[nThreads];
		debugInfo->totalBits += debugInfo->xchrBits[nThreads];
		debugInfo->totalBits += debugInfo->xchrFlagBits[nThreads];
		debugInfo->totalBits += debugInfo->startPosRawBits[nThreads];
		debugInfo->totalBits += debugInfo->startOffsetSignBits[nThreads];
		debugInfo->totalBits += debugInfo->startOffsetBits[nThreads];
		debugInfo->totalBits += debugInfo->strandBits[nThreads];
		debugInfo->totalBits += debugInfo->sourceSizeBits[nThreads];
		
		debugInfo->totalBits += debugInfo->qLinesBits[nThreads];
		debugInfo->totalBits += debugInfo->qLineFlagBits[nThreads];
		debugInfo->totalBits += debugInfo->qLineInFlagBits[nThreads];
		
		debugInfo->totalBits += debugInfo->iLineFlagBits[nThreads];
		debugInfo->totalBits += debugInfo->statusBits[nThreads];
		debugInfo->totalBits += debugInfo->countBits[nThreads];
		debugInfo->totalBits += debugInfo->irregularStatusBits[nThreads];
		debugInfo->totalBits += debugInfo->irregularCountBits[nThreads];
		
		debugInfo->totalBits += debugInfo->eLineFlagBits[nThreads];
		debugInfo->totalBits += debugInfo->eNRowsBits[nThreads];
		debugInfo->totalBits += debugInfo->eSourceNameBits[nThreads];
		debugInfo->totalBits += debugInfo->eStartPosRawBits[nThreads];
		debugInfo->totalBits += debugInfo->eStartOffsetSignBits[nThreads];
		debugInfo->totalBits += debugInfo->eStartOffsetBits[nThreads];
		debugInfo->totalBits += debugInfo->eSeqSizeBits[nThreads];
		debugInfo->totalBits += debugInfo->eStrandBits[nThreads];
		debugInfo->totalBits += debugInfo->eSourceSizeBits[nThreads];
		debugInfo->totalBits += debugInfo->eLineStatusBits[nThreads];
		debugInfo->totalBits += debugInfo->eLineIrregularStatusBits[nThreads];
		
		debugInfo->totalBits += debugInfo->hashInfoBits[nThreads];
	}
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		
	void CreateDebugInfo(uint8_t nThreads)
	{
		debugInfo = (DebugInfo *)Calloc(1, sizeof(DebugInfo));
		
		debugInfo->headerBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->scoreBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->sNRowsBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->sNColsBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->sourceNameBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->sLinesBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->caseBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->caseFlagBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->xchrBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->xchrFlagBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->startPosRawBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->startOffsetSignBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->startOffsetBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->strandBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->sourceSizeBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		
		debugInfo->qLinesBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->qLineFlagBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->qLineInFlagBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		
		debugInfo->iLineFlagBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->statusBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->countBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->irregularStatusBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->irregularCountBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		
		debugInfo->eLineFlagBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->eNRowsBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->eSourceNameBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->eStartPosRawBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->eStartOffsetSignBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->eStartOffsetBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->eSeqSizeBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->eStrandBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->eSourceSizeBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));		
		debugInfo->eLineStatusBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		debugInfo->eLineIrregularStatusBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
		
		debugInfo->hashInfoBits = (uint64_t *)Calloc(nThreads+1, sizeof(uint64_t));
	}
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	void FreeDebugInfo(uint8_t nThreads)
	{
		Free(debugInfo->headerBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->scoreBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->sNRowsBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->sNColsBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->sourceNameBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->sLinesBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->caseBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->caseFlagBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->xchrBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->xchrFlagBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->startPosRawBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->startOffsetSignBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->startOffsetBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->strandBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->sourceSizeBits, (nThreads+1) * sizeof(uint64_t));
		
		Free(debugInfo->qLinesBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->qLineFlagBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->qLineInFlagBits, (nThreads+1) * sizeof(uint64_t));

		Free(debugInfo->iLineFlagBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->statusBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->countBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->irregularStatusBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->irregularCountBits, (nThreads+1) * sizeof(uint64_t));

		Free(debugInfo->eLineFlagBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->eNRowsBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->eSourceNameBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->eStartPosRawBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->eStartOffsetSignBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->eStartOffsetBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->eSeqSizeBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->eStrandBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->eSourceSizeBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->eLineStatusBits, (nThreads+1) * sizeof(uint64_t));
		Free(debugInfo->eLineIrregularStatusBits, (nThreads+1) * sizeof(uint64_t));
		
		Free(debugInfo->hashInfoBits, (nThreads+1) * sizeof(uint64_t));
		
		Free(debugInfo, sizeof(DebugInfo));
		debugInfo = NULL;
	}
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	void PrintDebugInfo(uint8_t nThreads)
	{
		printf("+--------------------------------------------------------------------+\n");
		printf("| Header bits                   : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->headerBits[nThreads], ((double)debugInfo->headerBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Score bits                    : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->scoreBits[nThreads], ((double)debugInfo->scoreBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| N. Rows S Lines bits          : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->sNRowsBits[nThreads], ((double)debugInfo->sNRowsBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| N. Cols S Lines bits          : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->sNColsBits[nThreads], ((double)debugInfo->sNColsBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Source name bits              : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->sourceNameBits[nThreads], ((double)debugInfo->sourceNameBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| sLines bits                   : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->sLinesBits[nThreads], ((double)debugInfo->sLinesBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Case bits                     : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->caseBits[nThreads], ((double)debugInfo->caseBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Case flag bits                : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->caseFlagBits[nThreads], ((double)debugInfo->caseFlagBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Extra char bits               : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->xchrBits[nThreads], ((double)debugInfo->xchrBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Extra flag char bits          : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->xchrFlagBits[nThreads], ((double)debugInfo->xchrFlagBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Start (RAW) bits              : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->startPosRawBits[nThreads], ((double)debugInfo->startPosRawBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Start offset sign bits        : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->startOffsetSignBits[nThreads], ((double)debugInfo->startOffsetSignBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Start offset bits             : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->startOffsetBits[nThreads], ((double)debugInfo->startOffsetBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Strand bits                   : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->strandBits[nThreads], ((double)debugInfo->strandBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Source size bits              : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->sourceSizeBits[nThreads], ((double)debugInfo->sourceSizeBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("+--------------------------------------------------------------------+\n");
		printf("| qLines bits                   : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->qLinesBits[nThreads], ((double)debugInfo->qLinesBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| qLines flag bits              : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->qLineFlagBits[nThreads], ((double)debugInfo->qLineFlagBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| qLines in flag bits           : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->qLineInFlagBits[nThreads], ((double)debugInfo->qLineInFlagBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("+--------------------------------------------------------------------+\n");
		printf("| iLines flag bits              : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->iLineFlagBits[nThreads], ((double)debugInfo->iLineFlagBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| iLines status bits            : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->statusBits[nThreads], ((double)debugInfo->statusBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| Counts bits                   : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->countBits[nThreads], ((double)debugInfo->countBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| iLines Irregular status bits  : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->irregularStatusBits[nThreads], ((double)debugInfo->irregularStatusBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| iLines Irregular counts bits  : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->irregularCountBits[nThreads], ((double)debugInfo->irregularCountBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("+--------------------------------------------------------------------+\n");
		printf("| eLines flag bits              : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eLineFlagBits[nThreads], ((double)debugInfo->eLineFlagBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| eLines N. Rows bits           : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eNRowsBits[nThreads], ((double)debugInfo->eNRowsBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| eLines source name bits       : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eSourceNameBits[nThreads], ((double)debugInfo->eSourceNameBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| eLines Start (RAW) bits       : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eStartPosRawBits[nThreads], ((double)debugInfo->eStartPosRawBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| eLines start offset sign bits : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eStartOffsetSignBits[nThreads], ((double)debugInfo->eStartOffsetSignBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| eLines start offset bits      : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eStartOffsetBits[nThreads], ((double)debugInfo->eStartOffsetBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| eLines sequence size bits     : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eSeqSizeBits[nThreads], ((double)debugInfo->eSeqSizeBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| eLines strand bits            : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eStrandBits[nThreads], ((double)debugInfo->eStrandBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| eLines source size bits       : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eSourceSizeBits[nThreads], ((double)debugInfo->eSourceSizeBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| eLines status bits            : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eLineStatusBits[nThreads], ((double)debugInfo->eLineStatusBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("| eLines Irregular status bits  : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->eLineIrregularStatusBits[nThreads], ((double)debugInfo->eLineIrregularStatusBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("+--------------------------------------------------------------------+\n");
		printf("| Hash info bits                : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->hashInfoBits[nThreads], ((double)debugInfo->hashInfoBits[nThreads]/debugInfo->totalBits)*100.0);
		printf("+--------------------------------------------------------------------+\n");
		printf("| TOTAL BITS                    : %19"PRIu64" bits (%6.2lf%%) |\n", debugInfo->totalBits, 100.0);
		printf("+--------------------------------------------------------------------+\n");
	}
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
#endif // DEBUG_1
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
