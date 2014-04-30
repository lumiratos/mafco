// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Copyright 2013/2014 IEETA/DETI - University of Aveiro, Portugal.		 -
//	All Rights Reserved.													 -
//																			 -
//	These programs are supplied free of charge for research purposes only,   -
//	and may not be sold or incorporated into any commercial product. There   -
//	is ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they     -
//	are fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you	 - 
//	do happen to find a bug, or have modifications to suggest, please 		 -
//	report the same to Diogo Pratas, pratas@ua.pt. The copyright notice      -
//	above and this statement of conditions must remain an integral part of   -
//	each and every copy made of these files.								 -
//																			 -
//	Description: functions for the encoder and decoder						 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#ifndef AC_HEADER
#define AC_HEADER

#include <stdio.h>
#include <sys/types.h>
#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	FILE      *fp;
	uint64_t  low;
	uint64_t  high;
	uint64_t  fbits;
	uint64_t  total_bits;
	uint32_t  buffer; 
	uint32_t  bits_to_go;
	uint32_t  index;
	uint8_t   cBuf[CBUF_SIZE];
} ac_encoder;

typedef struct
{
	FILE      *fp;
	uint64_t  value;
	uint64_t  low;
	uint64_t  high;
	uint32_t  buffer;
	uint32_t  indexBuf;
	uint32_t  maxBuf;
	uint8_t   buf[CBUF_SIZE];                               
	uint32_t  bits_to_go;
} ac_decoder;

typedef struct
{
	uint32_t  cfreq[16]; //uint32_t  cfreq[256];
	uint32_t  nsym;
} ac_model;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t readNBits          	(uint32_t, ac_decoder *, ac_model *);
void     writeNBits         	(uint64_t, uint32_t, ac_encoder *, ac_model *);
void     writeString        	(const char [], size_t, ac_encoder *, ac_model *);
void 	 readString	    		(char [], size_t, ac_decoder *, ac_model *);
void     ac_encoder_init    	(ac_encoder *, const char *);
void     ac_decoder_init    	(ac_decoder *, const char *, off_t);
void     ac_model_init      	(ac_model   *, uint32_t);
void     shotgunEncode      	(ac_encoder *, uint32_t [], uint32_t);
void     ac_encode_symbol   	(ac_encoder *, ac_model *, uint32_t);
void     acEncodeBinary     	(ac_encoder *, ac_model *, uint32_t);
void     acEncodeBin0       	(ac_encoder *, ac_model *);
void     acEncodeBin1       	(ac_encoder *, ac_model *);
void     acEncode0          	(ac_encoder *, ac_model *);
void     acEncode1          	(ac_encoder *, ac_model *);
void     acEncode2          	(ac_encoder *, ac_model *);
void     acEncode3          	(ac_encoder *, ac_model *);
uint32_t acDecSymHighSizeVar	(ac_decoder *, ac_model *);
uint32_t acDecSymLowSizeVar		(ac_decoder *, ac_model *);
uint32_t acDecodeBinary			(ac_decoder *, ac_model *);
uint32_t acDecode4Symbols		(ac_decoder *, ac_model *);
void     ac_encoder_done		(ac_encoder *);
void     ac_decoder_done		(ac_decoder *);
uint64_t ac_encoder_bits		(ac_encoder *);
void     ac_model_done			(ac_model   *);

void 	reset_ac_encoder_buffer	(ac_encoder *);
void 	reset_ac_decoder_buffer	(ac_decoder *);

void	print_ac_encoder_info	(ac_encoder *);
void	print_ac_decoder_info	(ac_decoder *);
void	print_ac_model_info		(ac_model *);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
