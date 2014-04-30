#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define _FILE_OFFSET_BITS 								64

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef uint16_t ACCounter;              // Size of context counters for arrays
typedef unsigned char UChar;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Macros used in ac.c
       
#define CBUF_SIZE										65536 	// used in ac
#define SCHAR											sizeof(uint8_t)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Macros used in context.c
#define MAXCNT				 	(((uint64_t) 1 << (sizeof(ACCounter) * 8))-1)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Default parameters
#define	DEFAULT_NUMBER_OF_THREADS						4
#define DEFAULT_NUMBER_OF_GOBS							4
#define DEFAULT_TEMPLATE								'C'
#define DEFAULT_S_LINES_MODEL_ORDER						10
#define DEFAULT_CASE_MODEL_ORDER						5
#define DEFAULT_CASE_FLAG_MODEL_ORDER					5
#define DEFAULT_XCHAR_MODEL_ORDER						5
#define DEFAULT_XCHAR_FLAG_MODEL_ORDER					5
#define DEFAULT_STRAND_MODEL_ORDER						3
#define DEFAULT_START_OFFSET_SIGN_MODEL_ORDER			5

#define DEFAULT_Q_LINES_MODEL_ORDER						5
#define DEFAULT_Q_LINE_FLAG_MODEL_ORDER					5
#define DEFAULT_Q_LINE_IN_FLAG_MODEL_ORDER				5

#define DEFAULT_I_LINE_FLAG_MODEL_ORDER					5
#define DEFAULT_STATUS_MODEL_ORDER						4
#define DEFAULT_IRREGULAR_STATUS_MODEL_ORDER			5
#define DEFAULT_IRREGULAR_COUNT_MODEL_ORDER				5

#define DEFAULT_E_LINE_FLAG_MODEL_ORDER					5
#define DEFAULT_E_LINE_IRREGULAR_STATUS_MODEL_ORDER		5

// Number of column blocks to allocate at a time
#define DEFAULT_ROW_BLOCK_SIZE 							1024	

#define DEFAULT_TMP_ENC_FILE							"mafEnc-"
#define DEFAULT_TMP_DEC_FILE							"mafDec-"
#define DEFAULT_TMP_MAF_FILE							"mafPart-"
#define DEFAULT_MSAB_INFO_FILE							"MSAB-SizesInfo-"
#define DEFAULT_ACE_DEBUGGER_FILE						"/dev/null"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Some maximum default values
#define MAX_NUMBER_OF_THREADS							64
#define MAX_NUMBER_OF_GOBS								512
#define MAX_S_LINES_MODEL_ORDER							14
#define MAX_CASE_MODEL_ORDER							10
#define MAX_CASE_FLAG_MODEL_ORDER						10
#define MAX_XCHAR_MODEL_ORDER							10
#define MAX_XCHAR_FLAG_MODEL_ORDER						10
#define MAX_STRAND_MODEL_ORDER							10
#define MAX_START_OFFSET_SIGN_MODEL_ORDER				10

#define MAX_Q_LINES_MODEL_ORDER							10
#define MAX_Q_LINE_FLAG_MODEL_ORDER						10
#define MAX_Q_LINE_IN_FLAG_MODEL_ORDER					10

#define MAX_I_LINE_FLAG_MODEL_ORDER						10
#define MAX_STATUS_MODEL_ORDER							10
#define MAX_IRREGULAR_STATUS_MODEL_ORDER				10
#define MAX_IRREGULAR_COUNT_MODEL_ORDER					10

#define MAX_E_LINE_FLAG_MODEL_ORDER						10
#define MAX_E_LINE_IRREGULAR_STATUS_MODEL_ORDER			10

#define MAX_MSAB_NROWS							( (1 << (STORAGE_BITS_MSAB_ROWS)) - 1)
#define MAX_MSAB_NCOLS							( (1 << (STORAGE_BITS_MSAB_COLS)) - 1)
#define MAX_ABSOLUTE_SCORE						( (1 << (STORAGE_BITS_ABSOLUTE_SCORE)) - 1)
#define MAX_HEADER_LINE_SIZE					( (1 << (STORAGE_HEADER_LINE_MAX_SIZE)) - 1)
#define MAX_LINE_SIZE							( (1 << (STORAGE_BITS_MSAB_COLS)) - 1)
#define MAX_SRC_NAME_SIZE						( (1 << (STORAGE_BITS_SRC_NAME_SIZE)) - 1)
#define MAX_SRC_SIZE							( (1 << (STORAGE_BITS_SRC_SIZE)) - 1)
#define MAX_START_POSITION						( (1 << (STORAGE_BITS_START_POSITION)) - 1)
#define MAX_START_OFFSET						( (1 << (STORAGE_BITS_START_OFFSET)) - 1)
//#define MAX_SEQUENCE_SIZE						( (1 << (STORAGE_BITS_MSAB_COLS)) - 1)
#define MAX_COUNT_VALUE							( (1 << (STORAGE_BITS_COUNT_VALUE)) - 1)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Some other values
#define ROW_BLOCK_SIZE							1024
#define XCHR_SYMBOL								1
#define GAP_SYMBOL								4
#define Q_GAP_SYMBOL							12
#define BUF_SIZE								65536                     		
#define MEM_BLOCK_SIZE							100
//#define TMP_FILENAME_MAX_SIZE					32

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Symbols definition
#define N_SYMBOLS								5	// 'A', 'C', 'G', 'T' and '-'
#define N_QVALUES_SYMBOLS						12	// [0-9], 'F', '.'
#define N_STATUS_SYMBOLS						6	// 'C', 'I', 'N', 'n', 'M', 'T'
#define N_SYMBOLS_IN_SOURCE_NAME 				69	// 28 + 28 upper/lower letters
													// 10 numbers
													// underscore '_', dot '.' and dash '-'
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Storage bits for 
#define STORAGE_BITS_MSAB_ROWS					8
#define STORAGE_BITS_MSAB_COLS					25
#define STORAGE_BITS_ABSOLUTE_SCORE				30
#define STORAGE_HEADER_LINE_MAX_SIZE			6		// Number of bits to store the length of 
														// each header line (lines started by #..)
#define STORAGE_BITS_SRC_NAME_SIZE				6		// For strings with maximum of 63 characteres
#define STORAGE_BITS_SRC_SIZE					30		// Number of bits to store the source/chromossome size
#define STORAGE_BITS_START_POSITION				30		// Number of bits to store the start position
#define STORAGE_BITS_START_OFFSET				3		// Number of bits to store the start offset
#define STORAGE_BITS_COUNT_VALUE				25		// Number of bits to store the count field in the 'i' lines

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Hash parameters
//#define	HASH_TABLE_SIZE						104729	// Prime number
//#define	HASH_TABLE_SIZE						799999	// Another prime number
#define	HASH_TABLE_SIZE							32099	// Default table size
#define HASH_FUNCTION							0x1
//#define HASH_MAX_ELEMENTS_PER_ENTRY			UINT8_MAX
#define HASH_MAX_ELEMENTS_PER_ENTRY				31

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Some standard compressors
#define GZIPENC								"gzip -c"
#define GZIPDEC								"gzip -d -c"


//#define BZIP2ENC							"bzip2 "
#define BZIP2ENC							"bzip2 -c"
#define BZIP2DEC							"bzip2 -d -c"

// MSAc
#define MSACENC								"wine ctw.exe e"
#define MSACDEC								"wine ctw.exe d"

#ifdef WINDOWS
#define DEV_NULL "null"
#else
#define DEV_NULL "/dev/null"
#endif

#define ENC_VERSION_NUMBER \
"\nMAFCOenc version 1.00\n\n"

#define DEC_VERSION_NUMBER \
"\nMAFCOdec version 1.00\n\n"

#define COPYRIGHT_NOTICE \
"Copyright 2014 IEETA/DETI - University of Aveiro, Portugal.\n\
All Rights Reserved.\n\n\
These programs are supplied free of charge for research purposes only,\n\
and may not be sold or incorporated into any commercial product. There\n\
is ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they\n\
are fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you\n\
do happen to find a bug, or have modifications to suggest, please report\n\
the same to Luis M. O. Matos, luismatos@ua.pt. The copyright notice\n\
above and this statement of conditions must remain an integral part of\n\
each and every copy made of these files.\n\n"

#endif

