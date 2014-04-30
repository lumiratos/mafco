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

#include "models.h"
#include "models.h"
#include "context.h"
#include "ac.h"
#include "mem.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

CModels *CreateCModels(ModelsOrder *modelsOrder)
{
	CModels *cModels = Calloc(1, sizeof(CModels));
	
	cModels->sValuesCModel = CreateCModel(modelsOrder->sLinesModelOrder, 
											N_SYMBOLS, 1, 1);
	cModels->caseCModel = CreateCModel(modelsOrder->caseModelOrder, 2, 1, 1);
	cModels->caseFlagCModel = CreateCModel(modelsOrder->caseFlagModelOrder, 
											2, 1, 1);
	cModels->xchrCModel = CreateCModel(modelsOrder->xchrModelOrder, 2, 1, 1);
	cModels->xchrFlagCModel = CreateCModel(modelsOrder->xchrFlagModelOrder, 
											2, 1, 1);
	cModels->strandCModel = CreateCModel(modelsOrder->strandModelOrder, 
											2, 1, 1);
	cModels->startOffsetSignCModel = CreateCModel(modelsOrder->startOffsetSignModelOrder, 
											2, 1, 1);
	
	cModels->qValuesCModel = CreateCModel(modelsOrder->qLinesModelOrder, 
											N_QVALUES_SYMBOLS, 1, 1);
	
	cModels->qLineFlagCModel = CreateCModel(modelsOrder->qLineFlagModelOrder, 
											2, 1, 1);
	cModels->qLineInFlagCModel = CreateCModel(modelsOrder->qLineInFlagModelOrder, 
											2, 1, 1);
	
	cModels->iLineFlagCModel = CreateCModel(modelsOrder->iLineFlagModelOrder, 
											2, 1, 1);
	cModels->statusCModel = CreateCModel(modelsOrder->statusModelOrder, 
											N_STATUS_SYMBOLS, 1, 1);
	cModels->irregularStatusCModel = CreateCModel(modelsOrder->irregularStatusModelOrder, 
											2, 1, 1);
	cModels->irregularCountCModel = CreateCModel(modelsOrder->irregularCountModelOrder, 
											2, 1, 1);
	
	cModels->eLineFlagCModel = CreateCModel(modelsOrder->eLineFlagModelOrder, 
											2, 1, 1);
	cModels->eLineStatusCModel = CreateCModel(modelsOrder->statusModelOrder, 
											N_STATUS_SYMBOLS, 1, 1);
	cModels->eLineIrregularStatusCModel = CreateCModel(modelsOrder->eLineIrregularStatusModelOrder, 
											2, 1, 1);
	
	return cModels;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeCModels(CModels *cModels)
{
	FreeCModel(cModels->sValuesCModel);
	FreeCModel(cModels->caseCModel);
	FreeCModel(cModels->caseFlagCModel);
	FreeCModel(cModels->xchrCModel);
	FreeCModel(cModels->xchrFlagCModel);
	FreeCModel(cModels->strandCModel);
	FreeCModel(cModels->startOffsetSignCModel);
	
	FreeCModel(cModels->qValuesCModel);
	FreeCModel(cModels->qLineFlagCModel);
	FreeCModel(cModels->qLineInFlagCModel);
	
	FreeCModel(cModels->iLineFlagCModel);
	FreeCModel(cModels->statusCModel);
	FreeCModel(cModels->irregularStatusCModel);
	FreeCModel(cModels->irregularCountCModel);
	
	FreeCModel(cModels->eLineFlagCModel);
	FreeCModel(cModels->eLineStatusCModel);
	FreeCModel(cModels->eLineIrregularStatusCModel);
	
	Free(cModels, sizeof(CModels));
	cModels = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetCModels(CModels *cModels)
{
	ResetCModel(cModels->sValuesCModel);
	ResetCModel(cModels->caseCModel);
	ResetCModel(cModels->caseFlagCModel);
	ResetCModel(cModels->xchrCModel);
	ResetCModel(cModels->xchrFlagCModel);
	ResetCModel(cModels->strandCModel);
	ResetCModel(cModels->startOffsetSignCModel);
	
	ResetCModel(cModels->qValuesCModel);
	ResetCModel(cModels->qLineFlagCModel);
	ResetCModel(cModels->qLineInFlagCModel);
	
	ResetCModel(cModels->iLineFlagCModel);
	ResetCModel(cModels->statusCModel);
	ResetCModel(cModels->irregularStatusCModel);
	ResetCModel(cModels->irregularCountCModel);
	
	ResetCModel(cModels->eLineFlagCModel);
	ResetCModel(cModels->eLineStatusCModel);
	ResetCModel(cModels->eLineIrregularStatusCModel);
	
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ACModels *CreateACModels()
{
	ACModels *acModels = (ACModels *)Calloc(1, sizeof(ACModels));
	
	acModels->sValuesACModel = (ac_model *)Calloc(1, sizeof(ac_model));
	acModels->qValuesACModel = (ac_model *)Calloc(1, sizeof(ac_model));
	acModels->statusACModel = (ac_model *)Calloc(1, sizeof(ac_model));
	acModels->binaryACModel = (ac_model *)Calloc(1, sizeof(ac_model));
	acModels->binaryUniformACModel = (ac_model *)Calloc(1, sizeof(ac_model));
	
	ResetACModels(acModels);
	/*
	ac_model_init(acModels->sValuesACModel, N_SYMBOLS);
	ac_model_init(acModels->qValuesACModel, N_QVALUES_SYMBOLS);
	ac_model_init(acModels->statusACModel, N_STATUS_SYMBOLS);
	ac_model_init(acModels->binaryACModel, 2);
	ac_model_init(acModels->binaryUniformACModel, 2);
	*/
	return acModels;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeACModels(ACModels *acModels)
{
	// It is not really necessary?
	/*
	ac_model_done(acModels->sValuesModel);
	ac_model_done(acModels->qValuesModel);
	ac_model_done(acModels->statusModel);
	ac_model_done(acModels->binaryModel);
	*/
	
	Free(acModels->sValuesACModel, sizeof(ac_model));
	Free(acModels->qValuesACModel, sizeof(ac_model));
	Free(acModels->statusACModel, sizeof(ac_model));
	Free(acModels->binaryACModel, sizeof(ac_model));
	Free(acModels->binaryUniformACModel, sizeof(ac_model));
		
	Free(acModels, sizeof(ACModels));
	acModels = NULL;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResetACModels(ACModels *acModels)
{
	// The ac_model_init function does initialize the counter
	// for each symbol
	
	ac_model_init(acModels->sValuesACModel, N_SYMBOLS);
	ac_model_init(acModels->qValuesACModel, N_QVALUES_SYMBOLS);
	ac_model_init(acModels->statusACModel, N_STATUS_SYMBOLS);
	ac_model_init(acModels->binaryACModel, 2);
	ac_model_init(acModels->binaryUniformACModel, 2);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ACEncoder *CreateACEncoder(const char *name)
{
	ACEncoder *acEncoder = (ACEncoder *)Calloc(1, sizeof(ACEncoder));
	acEncoder->globalEncoder = (ac_encoder *)Calloc(1, sizeof(ac_encoder));
	ac_encoder_init(acEncoder->globalEncoder, name);
	
	return acEncoder;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeACEncoder(ACEncoder *acEncoder)
{
	//ac_encoder_done(acEncoders->globalEncoder);
	Free(acEncoder->globalEncoder, sizeof(ac_encoder));
	Free(acEncoder, sizeof(ACEncoder));
	acEncoder = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResumeACEncoder(ACEncoder *acEncoder, const char *name)
{
	ac_encoder_init(acEncoder->globalEncoder, name);
	
	// This function will set all values of cBuf to 0x0
	//reset_ac_encoder_buffer(acEncoder->globalEncoder);
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ACEncoderDone(ACEncoder *acEncoder)
{
	ac_encoder_done(acEncoder->globalEncoder);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ACDecoder *CreateACDecoder(const char *name, off_t offset)
{
	ACDecoder *acDecoder = (ACDecoder *)Calloc(1, sizeof(ACDecoder));
	acDecoder->globalDecoder = (ac_decoder *)Calloc(1, sizeof(ac_decoder));
	ac_decoder_init(acDecoder->globalDecoder, name, offset);
	
	return acDecoder;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void FreeACDecoder(ACDecoder *acDecoder)
{
	//ac_decoder_done(acDecoder->globalDecoder);
	Free(acDecoder->globalDecoder, sizeof(ac_encoder));
	Free(acDecoder, sizeof(ACEncoder));
	acDecoder = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ResumeACDecoder(ACDecoder *acDecoder, const char *name, off_t offset)
{
	ac_decoder_init(acDecoder->globalDecoder, name, offset);
	
	// This function will set all values of buf to 0x0
	//reset_ac_decoder_buffer(acDecoder->globalDecoder);
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ACDecoderDone(ACDecoder *acDecoder)
{
	ac_decoder_done(acDecoder->globalDecoder);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
