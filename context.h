// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Copyright 2013/2014 IEETA/DETI - University of Aveiro, Portugal.		 -
//	All Rights Reserved.													 -
//																			 -
//	These programs are supplied free of charge for research purposes only,   -
//	and may not be sold or incorporated into any commercial product. There   -
//	is ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they     -
//	are fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you	 - 
//	do happen to find a bug, or have modifications to suggest, please 		 -
//	report the same to Armando J. Pinho, ap@ua.pt. The copyright notice      -
//	above and this statement of conditions must remain an integral part of   -
//	each and every copy made of these files.								 -
//																			 -
//	Description: functions for handling finite-context models.				 -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include "defs.h"
#include "ac.h"
#include "msab.h"
#include "models.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct
{
	int8_t row;
	int8_t col;
} Coords;

typedef struct
{
	uint8_t size;
	Coords *position;
} CTemplate;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CModel			*CreateCModel		(uint32_t, uint32_t, uint32_t, uint32_t);
void			FreeCModel			(CModel*);
void 			ResetCModel			(CModel *);

inline void		GetPModelIdx		(MSAB *msab, uint32_t, uint32_t, CTemplate *, 
									CModel *);
inline void 	GetPModelIdx2		(UChar, CModel *);
inline void 	GetPModelIdx3		(uint8_t *, uint8_t, CModel *);

inline void 	ComputePModel		(CModel *, ac_model *);
inline void 	ComputePModel2		(CModel *, ac_model *);
inline void 	UpdateCModelCounter	(CModel *, UChar);
inline void		UpdateCModelIdx		(CModel *, UChar);

CTemplate 		*InitTemplate		(UChar, uint8_t);
void 			FreeTemplate		(CTemplate *);
void 			ShowTemplate		(CTemplate *);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif


