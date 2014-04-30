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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ac.h"
#include "mem.h"
#include "common.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define Code_value_bits    32
#define Code_Value_bitsL2  (Code_value_bits-2)
#define Top_value          (((uint64_t)1<<Code_value_bits)-1)    // 4294967295
#define First_qtr          (Top_value/4+1)                       // 1073741824
#define First_qtrL1        (First_qtr - 1)
#define Half	           (2*First_qtr)                         // 2147483648
#define HalfL1             (Half - 1)
#define Third_qtr          (3*First_qtr)                         // 3221225472

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline static void       output_bit      (ac_encoder *, uint32_t);
inline static void       bit_plus_follow (ac_encoder *, uint32_t);
inline static uint32_t   input_bit       (ac_decoder *);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void writeNBits(uint64_t bits, uint32_t nBits, ac_encoder *ace, ac_model *acm)
{
	while(nBits)
		((uint64_t)bits >> --nBits) & 0x1 ? acEncodeBin1(ace, acm) : 
			acEncodeBin0(ace, acm);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t readNBits(uint32_t nBits, ac_decoder *acd, ac_model *acm)
{
	uint64_t bits = 0;

	while(nBits)
	{
		bits <<= 1;
		bits |= acDecodeBinary(acd, acm);
		--nBits;
    }
  
	return bits;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void writeString (const char str[], size_t strMaxSize, ac_encoder *ace, ac_model *acm)
{
	size_t strSize, n;

	// Get String size
	strSize = Strlen(str);
	if(strSize > strMaxSize) 
    {
		fprintf(stderr, "Error (writeString): input string length is bigger than %"PRIu64".\n", (uint64_t)strSize);
		fprintf(stderr, "String to encode: '%s'\n", str);
		fprintf(stderr, "The encoder is only prepared to encode %"PRIu64" out of %"PRIu64" characters of the input string.\n", (uint64_t)strMaxSize, (uint64_t)strSize);
		exit(EXIT_FAILURE);
    }
  
	// Encode each character
	for(n = 0; n != strMaxSize; ++n)
	{
		if(str[n] < 0)
		{
			fprintf(stderr, "Error (writeString): unable to encode a negative character in this context.\n");
			fprintf(stderr, "Error (writeString): value to encode = %"PRId8".\n", str[n]);
			exit(EXIT_FAILURE);
		}
		writeNBits((uint8_t)str[n], 8, ace, acm);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void readString (char str[], size_t strMaxSize, ac_decoder *acd, ac_model *acm)
{
	size_t n;
	for(n = 0; n != strMaxSize; ++n)
	{
		str[n] = readNBits(8, acd, acm);
	}
	str[strMaxSize] = '\0';
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline static void output_bit(ac_encoder *ace, uint32_t bit)
{
	ace->buffer     >>= 0x1;
	if(bit)                 
		ace->buffer    |= 0x80;
	--ace->bits_to_go;
	++ace->total_bits;
	if(!ace->bits_to_go)   
    {
		ace->cBuf[ace->index++] = ace->buffer;
		if(ace->index == CBUF_SIZE)
		{
			fwrite(ace->cBuf, SCHAR, ace->index, ace->fp);
			ace->index    = 0x0;
		}
		ace->bits_to_go = 8;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline static void bit_plus_follow(ac_encoder *ace, uint32_t bit)
{
	output_bit(ace, bit);
	while(ace->fbits)                                           
    {
		output_bit(ace, !bit);
		--ace->fbits;
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline static uint32_t input_bit(ac_decoder *acd)
{
	uint32_t iB;

	if(!acd->bits_to_go)
    {
		if(acd->indexBuf == acd->maxBuf)
		{
			acd->maxBuf   = fread(acd->buf, 0x1, CBUF_SIZE, acd->fp); 
			acd->indexBuf = 0x0;
		}
		acd->buffer     = acd->buf[acd->indexBuf++];
		acd->bits_to_go = 8;
	}

	iB = acd->buffer & 0x1, acd->buffer >>= 0x1, --acd->bits_to_go;

	return iB;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ac_encoder_init(ac_encoder *ace, const char *fn)
{
	ace->fp         = Fopen(fn, "wb"); 
	ace->bits_to_go = 8;
	ace->low        = 0;
	ace->high       = Top_value;
	ace->fbits      = 0;
	ace->buffer     = 0;
	ace->index      = 0;
	ace->total_bits = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ac_encoder_done(ac_encoder *ace)
{
	++ace->fbits;
	bit_plus_follow(ace, ace->low < First_qtr ? 0 : 1);

	if(ace->fp)
    {
		fwrite(ace->cBuf, sizeof(uint8_t), ace->index, ace->fp);
		putc  (ace->buffer >> ace->bits_to_go,         ace->fp);
		//printf("Error HERE?\n");
		Fclose(ace->fp);
		//printf("NO?\n");
		//fclose(ace->fp);
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void reset_ac_encoder_buffer(ac_encoder *ace)
{
	uint32_t i;
	for(i = 0; i != CBUF_SIZE; ++i)
		ace->cBuf[i] = 0x0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ac_decoder_init(ac_decoder *acd, const char *fn, off_t offset)
{
	uint32_t n;

	acd->fp           = (fn ? Fopen(fn, "rb") : NULL);
  
	// Used to start decoding at a specific position
	Fseeko(acd->fp, offset, SEEK_SET);	

	acd->bits_to_go   = 0;
	acd->indexBuf     = 0;
	acd->maxBuf       = 0;
	acd->value        = 0;
	
	for(n = 1 ; !(n > Code_value_bits) ; ++n)
    	acd->value = 2 * acd->value + input_bit(acd);
	
	acd->low          = 0;
	acd->high         = Top_value;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ac_decoder_done(ac_decoder *acd)
{
	Fclose(acd->fp);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void reset_ac_decoder_buffer(ac_decoder *acd)
{
	uint32_t i;
	for(i = 0; i != CBUF_SIZE; ++i)
		acd->buf[i] = 0x0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ac_model_init(ac_model *acm, uint32_t nsym)
{
	uint32_t n;

	acm->nsym  = nsym;

	for(n = 0 ; n <= nsym ; ++n)	// This is crucial when using an
		acm->cfreq[n] = nsym - n;	// uniform distribution (writeNBits)
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ac_model_done(ac_model *acm)
{
	acm->nsym  = 0; 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t ac_encoder_bits(ac_encoder *ace)
{
	return ace->total_bits;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void shotgunEncode(ac_encoder *ace, uint32_t shotgun[], uint32_t i)
{
	uint64_t ran;
	uint32_t n = 0;

	while(n != i)
    {
		ran        = (uint64_t) (ace->high-ace->low) + 1;
		ace->high  = ace->low + (ran * shotgun[1+n]) / shotgun[n] - 1; 
		ace->low  += (          (ran * shotgun[2+n]) / shotgun[n]); 

		while(1)
		{
			if(ace->high < Half)
				bit_plus_follow(ace, 0);
			else if(ace->low > HalfL1)
        	{
				bit_plus_follow(ace, 1);
				ace->low -= Half, ace->high -= Half;
			}
			else if(ace->low > First_qtrL1 && ace->high < Third_qtr)
				++ace->fbits, ace->low -= First_qtr, ace->high -= First_qtr;
			else
				break;

			ace->low <<= 0x1, ace->high <<= 0x1, ace->high |= 0x1;
		}

		n += 3;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ac_encode_symbol(ac_encoder *ace, ac_model *acm, uint32_t sym)
{
	uint64_t ran = (uint64_t) (ace->high-ace->low) + 1;
	ace->high    = ace->low + (ran*acm->cfreq[sym  ]) / acm->cfreq[0] - 1; 
	ace->low    += (          (ran*acm->cfreq[++sym]) / acm->cfreq[0]);    

	while(1)
    {
		if(ace->high < Half)
			bit_plus_follow(ace, 0);
		else if(ace->low > HalfL1)
		{
			bit_plus_follow(ace, 1);
			ace->low -= Half, ace->high -= Half;
		}
		else if(ace->low > First_qtrL1 && ace->high < Third_qtr)
			++ace->fbits, ace->low -= First_qtr, ace->high -= First_qtr;
		else
			break;

		ace->low <<= 0x1, ace->high <<= 0x1, ace->high |= 0x1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void acEncodeBin0(ac_encoder *ace, ac_model *acm)
{
	ace->low += (((ace->high - ace->low + 1) * acm->cfreq[1]) / acm->cfreq[0]); 

	while(1)
    {
		if(ace->high < Half)
			bit_plus_follow(ace, 0);
		else if(ace->low > HalfL1)
		{
			bit_plus_follow(ace, 1);
			ace->low -= Half, ace->high -= Half;
		}
		else if(ace->low > First_qtrL1 && ace->high < Third_qtr)
			++ace->fbits, ace->low -= First_qtr, ace->high -= First_qtr;
		else
			break;

		ace->low <<= 0x1, ace->high <<= 0x1, ace->high |= 0x1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void acEncodeBin1(ac_encoder *ace, ac_model *acm)
{
	ace->high = ace->low + ((ace->high - ace->low + 1) * acm->cfreq[1]) / 
		acm->cfreq[0] - 1;

	while(1)
    {
		if(ace->high < Half)
			bit_plus_follow(ace, 0);
		else if(ace->low > HalfL1)
		{
			bit_plus_follow(ace, 1);
			ace->low -= Half, ace->high -= Half;
		}
		else if(ace->low > First_qtrL1 && ace->high < Third_qtr)
			++ace->fbits, ace->low -= First_qtr, ace->high -= First_qtr;
		else
			break;

		ace->low <<= 0x1, ace->high <<= 0x1, ace->high |= 0x1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void acEncode0(ac_encoder *ace, ac_model *acm)
{
	ace->low += (((ace->high - ace->low + 1) * acm->cfreq[1]) / acm->cfreq[0]); 

	while(1)
    {
		if(ace->high < Half)
			bit_plus_follow(ace, 0);
		else if(ace->low > HalfL1)
		{
			bit_plus_follow(ace, 1);
			ace->low -= Half, ace->high -= Half;
		}
		else if(ace->low > First_qtrL1 && ace->high < Third_qtr)
			++ace->fbits, ace->low -= First_qtr, ace->high -= First_qtr;
		else
			break;

		ace->low <<= 0x1, ace->high <<= 0x1, ace->high |= 0x1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void acEncode1(ac_encoder *ace, ac_model *acm)
{
	uint64_t ran = (uint64_t) (ace->high-ace->low) + 1;
	ace->high    = ace->low + (ran*acm->cfreq[1]) / acm->cfreq[0] - 1;
	ace->low    += (          (ran*acm->cfreq[2]) / acm->cfreq[0]   ); 

	while(1)
    {
		if(ace->high < Half)
			bit_plus_follow(ace, 0);
		else if(ace->low > HalfL1)
		{
			bit_plus_follow(ace, 1);
			ace->low -= Half, ace->high -= Half;
		}
		else if(ace->low > First_qtrL1 && ace->high < Third_qtr)
			++ace->fbits, ace->low -= First_qtr, ace->high -= First_qtr;
		else
			break;

		ace->low <<= 0x1, ace->high <<= 0x1, ace->high |= 0x1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void acEncode2(ac_encoder *ace, ac_model *acm)
{
	uint64_t ran = (uint64_t) (ace->high-ace->low) + 1;
	ace->high    = ace->low + (ran*acm->cfreq[2]) / acm->cfreq[0] - 1;
	ace->low    += (          (ran*acm->cfreq[3]) / acm->cfreq[0]   );  

	while(1)
    {
		if(ace->high < Half)
			bit_plus_follow(ace, 0);
		else if(ace->low > HalfL1)
		{
			bit_plus_follow(ace, 1);
			ace->low -= Half, ace->high -= Half;
		}
		else if(ace->low > First_qtrL1 && ace->high < Third_qtr)
			++ace->fbits, ace->low -= First_qtr, ace->high -= First_qtr;
		else
			break;

		ace->low <<= 0x1, ace->high <<= 0x1, ace->high |= 0x1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void acEncode3(ac_encoder *ace, ac_model *acm)
{
	ace->high    = ace->low + ((ace->high-ace->low + 1) * acm->cfreq[3]) / 
		acm->cfreq[0] - 1;

	while(1)
    {
    	if(ace->high < Half)
			bit_plus_follow(ace, 0);
		else if(ace->low > HalfL1)
		{
			bit_plus_follow(ace, 1);
			ace->low -= Half, ace->high -= Half;
		}
		else if(ace->low > First_qtrL1 && ace->high < Third_qtr)
			++ace->fbits, ace->low -= First_qtr, ace->high -= First_qtr;
		else
			break;

		ace->low <<= 0x1, ace->high <<= 0x1, ace->high |= 0x1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void acEncodeBinary(ac_encoder *ace, ac_model *acm, uint32_t sym)
{
	uint64_t ran = (uint64_t) (ace->high-ace->low) + 1;
	ace->high    = ace->low + (ran*acm->cfreq[sym  ]) / acm->cfreq[0] - 1; 
	ace->low    += (          (ran*acm->cfreq[++sym]) / acm->cfreq[0]   ); 

	while(1)
    {
		if(ace->high < Half)
			bit_plus_follow(ace, 0);
		else if(ace->low > HalfL1)
		{
			bit_plus_follow(ace, 1);
			ace->low -= Half, ace->high -= Half;
		}
		else if(ace->low > First_qtrL1 && ace->high < Third_qtr)
			++ace->fbits, ace->low -= First_qtr, ace->high -= First_qtr;
		else
			break;

		ace->low <<= 0x1, ace->high <<= 0x1, ace->high |= 0x1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t acDecode4Symbols(ac_decoder *acd, ac_model *acm)
{
	uint64_t ran =   (uint64_t)(acd->high -acd->low)+1;
 	uint64_t cum = (((uint64_t)(acd->value-acd->low)+1) *acm->cfreq[0] - 1)/ran;
 	uint32_t sym;
 	sym          = 	(acm->cfreq[2] > cum ? (acm->cfreq[3] > cum ? 3 : 2) : 
					(acm->cfreq[1] > cum ? 1 : 0));
	acd->high    = acd->low + (ran*acm->cfreq[sym]  )   /acm->cfreq[0] - 1;       
	acd->low    += (          (ran*acm->cfreq[sym+1])   /acm->cfreq[0]   );      

	while(1)
    {
		if(acd->high < Half)
			;
		else if(acd->low > HalfL1)
			acd->value -= Half, acd->low -= Half, acd->high -= Half;
		else if(acd->low > First_qtrL1 && acd->high < Third_qtr)
			acd->value -= First_qtr, acd->low -= First_qtr, acd->high -= First_qtr;
		else
			break;

		acd->low   <<= 0x1;
		acd->high  <<= 0x1, acd->high  |= 0x1;
		acd->value <<= 0x1, acd->value |= input_bit(acd);
	}

	return sym;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t acDecodeBinary(ac_decoder *acd, ac_model *acm)
{
	uint64_t ran =   (uint64_t)(acd->high -acd->low)+1;
	uint64_t cum = (((uint64_t)(acd->value-acd->low)+1) *acm->cfreq[0] - 1)/ran;
	uint32_t sym = (acm->cfreq[1] > cum ? 1 : 0);   
	acd->high    = acd->low + (ran*acm->cfreq[sym]  )   /acm->cfreq[0] - 1;       
	acd->low    += (          (ran*acm->cfreq[sym+1])   /acm->cfreq[0]   );      

	while(1)
    {
		if(acd->high < Half)
			;
		else if(acd->low > HalfL1)
			acd->value -= Half, acd->low -= Half, acd->high -= Half;
		else if(acd->low > First_qtrL1 && acd->high < Third_qtr)
			acd->value -= First_qtr, acd->low -= First_qtr, acd->high -= First_qtr;
		else
			break;

		acd->low   <<= 0x1;
		acd->high  <<= 0x1, acd->high  |= 0x1;
		acd->value <<= 0x1, acd->value |= input_bit(acd);
	}

	return sym;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t acDecSymLowSizeVar(ac_decoder *acd, ac_model *acm)
{
	uint64_t ran  =   (uint64_t)(acd->high -acd->low)+1;
	uint64_t cum  = (((uint64_t)(acd->value-acd->low)+1)*acm->cfreq[0]-1)/ran; 
	uint32_t sym;

	for(sym = 0 ; acm->cfreq[sym+1] > cum ; ++sym)  
		; 

	acd->high  = acd->low + (ran*acm->cfreq[sym  ]) / acm->cfreq[0] - 1; 
	acd->low  += (          (ran*acm->cfreq[sym+1]) / acm->cfreq[0]   ); 

	while(1)
    {
		if(acd->high < Half)
			;
		else if(acd->low > HalfL1)
			acd->value -= Half, acd->low -= Half, acd->high -= Half;
		else if(acd->low > First_qtrL1 && acd->high < Third_qtr)
			acd->value -= First_qtr, acd->low -= First_qtr, acd->high -= First_qtr;
		else
			break;

		acd->low   <<= 0x1;
		acd->high  <<= 0x1, acd->high  |= 0x1;
		acd->value <<= 0x1, acd->value |= input_bit(acd);
    }

	return sym;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t acDecSymHighSizeVar(ac_decoder *acd, ac_model *acm)
{
	uint64_t ran  =   (uint64_t)(acd->high -acd->low)+1;
	uint64_t cum  = (((uint64_t)(acd->value-acd->low)+1)*acm->cfreq[0]-1)/ran; 
	uint32_t left = 0, right = acm->nsym, mid;
 
	while(!(left > right))
    {
		mid = (right + left) >> 0x1;
		if(cum < acm->cfreq[mid])      
			left  = ++mid;
		else
			right = --mid;
	}

	acd->high  = acd->low + (ran*acm->cfreq[--left]) / acm->cfreq[0] - 1; 
	acd->low  += (          (ran*acm->cfreq[left+1]) / acm->cfreq[0]   ); 

	while(1)
    {
		if(acd->high < Half)
			;
		else if(acd->low > HalfL1)
			acd->value -= Half, acd->low -= Half, acd->high -= Half;
		else if(acd->low > First_qtrL1 && acd->high < Third_qtr)
			acd->value -= First_qtr, acd->low -= First_qtr, acd->high -= First_qtr;
		else
			break;

		acd->low   <<= 0x1;
		acd->high  <<= 0x1, acd->high  |= 0x1;
		acd->value <<= 0x1, acd->value |= input_bit(acd);
	}

	return left;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void print_ac_encoder_info(ac_encoder *ace)
{
	printf("ACE LOW %"PRIu64" | ACE HIGH %"PRIu64"\n", ace->low, ace->high);
}

void print_ac_decoder_info(ac_decoder *acd)
{
	printf("ACD LOW %"PRIu64" | ACD HIGH %"PRIu64"\n", acd->low, acd->high);
}

void print_ac_model_info(ac_model *acm)
{
	uint32_t n;
	for(n = 0; n < acm->nsym; n++)
	{
		printf("Symbol %"PRIu32" | cfreq = %"PRIu32"\n", n, acm->cfreq[n]);
	}
	printf("\n");
}
