// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Copyright 2014 IEETA/DETI - University of Aveiro, Portugal.				 -
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
//	Description: functions for handling with the encoding models			 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef MODELS_H_INCLUDED
#define MODELS_H_INCLUDED

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "ac.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct 
{
	uint8_t			sLinesModelOrder;				// Model order to encode the 's' 
													// lines (DNA bases and gaps)
	uint8_t			statusModelOrder;				// Model order to encode the status 
													// symbol of 'i' and 'e' lines
	uint8_t			caseModelOrder;					// Model order to encode the case 
													// information (lower or upper symbols)
	uint8_t			caseFlagModelOrder;				// Model order to encode teh flag that 
													// sinalizes the presence of lower bases in the MSAB
	uint8_t			xchrModelOrder;					// Model order to encode the extra bases (N's n's)
	uint8_t			xchrFlagModelOrder;				// Model order to encode the flag 
													// that sinalizes the presence of extra bases (N's n's)
	uint8_t			strandModelOrder;				// Model order to encode the 
													// strand informations
	uint8_t			startOffsetSignModelOrder;		// Model order to encode the start offset sign
	
	
	uint8_t			qLinesModelOrder;				// Model order to encode the 'q' 
													// lines (quality values)
	uint8_t			qLineFlagModelOrder;			// Mode order to encode the 
													// presence of 'q' lines in the MSAB
	uint8_t			qLineInFlagModelOrder;			// Mode order to encode the 
													// position of each 'q' lines in the MSAB
	
	uint8_t			iLineFlagModelOrder;			// Model order to encode the 
													// presence of 'i' lines in the MSAB
	uint8_t			irregularStatusModelOrder;		// Model order to encode the status symbol
													// irregularities
	uint8_t			irregularCountModelOrder;		// Model order to encode the count value
													// irregularities
	
	uint8_t			eLineFlagModelOrder;			// Model order to encode the 
													// presence of 'e' lines in the MSAB
	uint8_t			eLineIrregularStatusModelOrder;	// Model order to encode the status symbol
													// irregularities of the 'e' lines
	
} ModelsOrder;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	ACCounter  *counters;				// FCM Counters
	uint32_t   *multipliers;			// Calculated products
	uint64_t   nPModels;				// Maximum number of probability models
	uint32_t   ctxSize;					// Current depth of context template
	uint32_t   nSymbols;				// Number of coding symbols
	uint32_t   pModelIdx;				// Index of probabilistic model
	uint32_t   kMinusOneMask;			// e.g. ...0001111111111, if ctxSize = 6
	uint8_t    deltaNum;				// Numerator of delta
	uint8_t    deltaDen;				// Denominator of delta
} CModel;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{	
	CModel			*sValuesCModel;					// CModel to encode the 's' lines 
													// (DNA bases and gaps)
	CModel			*caseCModel;					// CModel to encode the case 
													// information (lower or upper symbols)
	CModel			*caseFlagCModel;				// CModel to encode the flag that 
													// sinalizes the presence of lower base 
													// in the MSAB
	CModel			*xchrCModel;					// CModel to encode the extra bases (N's n's)
	CModel			*xchrFlagCModel;				// CModel to encode the flag the 
													// sinalizes the presence of extra 
													// bases in the MSAB
	CModel			*strandCModel;					// CModel to encode the strand informations
	CModel			*startOffsetSignCModel;			// CModel to encode the start offset sign
	
	
	CModel			*qValuesCModel;					// CModel to encode the 'q' lines 
													// (quality values)
	CModel			*qLineFlagCModel;				// CModel to encode the presence of 
													// the 'q' lines in the MSAB
	CModel			*qLineInFlagCModel;				// CModel to encode the position of 
													// the 'q' lines in the MSAB
	
	CModel			*iLineFlagCModel;				// CModel to encode the presence of 
													// the 'i' lines in the MSAB
	CModel			*statusCModel;					// CModel to encode the status symbol 
													// of 'i' and 'e' lines
	
	CModel			*irregularStatusCModel;			// CModel to encode the flag that indicates
													// an irregular status symbol 
	
	CModel			*irregularCountCModel;			// CModel to encode the flag that indicates
													// an irregular count value
	
	
	CModel			*eLineFlagCModel;				// CModel to encode the presence of 
													// the 'e' lines in the MSAB
	CModel			*eLineStatusCModel;				// CModel to encode the status symbol 
													// of the 'e' lines
	CModel			*eLineIrregularStatusCModel;	// CModel to encode the flag that indicates
													// an irregular status symbol 
	
} CModels;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{	
	ac_model		*sValuesACModel;
	ac_model		*qValuesACModel;
	ac_model		*statusACModel;
	ac_model		*binaryACModel;
	ac_model		*binaryUniformACModel;	// Used only in writeNBits and readNBits
} ACModels;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	ac_encoder		*globalEncoder;
} ACEncoder;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	ac_decoder		*globalDecoder;
} ACDecoder;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CModels		*CreateCModels			(ModelsOrder*);
void		FreeCModels				(CModels*);
void 		ResetCModels			(CModels *);

ACModels 	*CreateACModels			();
void 		FreeACModels			(ACModels *);
void		ResetACModels			(ACModels *);
	
ACEncoder	*CreateACEncoder		(const char *);
void 		FreeACEncoder			(ACEncoder *);
void	 	ResumeACEncoder			(ACEncoder *, const char *);
void 		ACEncoderDone			(ACEncoder *);

ACDecoder	*CreateACDecoder		(const char *, off_t);
void		FreeACDecoder			(ACDecoder *);
void 		ResumeACDecoder			(ACDecoder *, const char *, off_t);
void		ACDecoderDone			(ACDecoder *);
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
