// +--------------------------------------------------------------------------+
// | Copyright 2014 IEETA/DETI - University of Aveiro, Portugal.              |
// | All Rights Reserved.                                                     |
// |                                                                          |
// | These programs are supplied free of charge for research purposes only,   |
// | and may not be sold or incorporated into any commercial product. There   |
// | is ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they     |
// | are fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you    |
// | do happen to find a bug, or have modifications to suggest, please report |
// | the same to Luis M. O. Matos, luismatos@ua.pt. The copyright notice      |
// | above and this statement of conditions must remain an integral part of   |
// | each and every copy made of these files.                                 |
// |                                                                          |
// | Description: functions definition for gtod module                        |
// +--------------------------------------------------------------------------+

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "gtod.h"

// ----------------------------------------------------------------------------
// Static variables
// ----------------------------------------------------------------------------
static struct timeval start_tv; 
static struct timeval tic_tv;
static struct timeval tac_tv;

// ----------------------------------------------------------------------------
// Init GTOD Clock
// ----------------------------------------------------------------------------
void InitClock()
{
	if(gettimeofday(&start_tv, NULL) != 0)
	{
		fprintf(stderr, "Error (InitGTODClock): unable to get time of day!\n");
		exit(EXIT_FAILURE);
	}
	
	tic_tv.tv_sec = start_tv.tv_sec;
	tac_tv.tv_sec = start_tv.tv_sec;
	
	tic_tv.tv_usec = start_tv.tv_usec;
	tac_tv.tv_usec = start_tv.tv_usec;
}

// ----------------------------------------------------------------------------
// Init Local GTOD Clock
// ----------------------------------------------------------------------------
void InitLocalClock(GTODClock *gc)
{
	if(gettimeofday(&gc->start_tv, NULL) != 0)
	{
		fprintf(stderr, "Error (InitGTODClock): unable to get time of day!\n");
		exit(EXIT_FAILURE);
	}
	
	gc->tic_tv.tv_sec = gc->start_tv.tv_sec;
	gc->tac_tv.tv_sec = gc->start_tv.tv_sec;
	
	gc->tic_tv.tv_usec = gc->start_tv.tv_usec;
	gc->tac_tv.tv_usec = gc->start_tv.tv_usec;
}

// ----------------------------------------------------------------------------
// Tic GTOD Clock
// ----------------------------------------------------------------------------
void Tic()
{
	if(gettimeofday(&tic_tv, NULL) != 0)
	{
		fprintf(stderr, "Error (TicGTODClock): unable to get time of day!\n");
		exit(EXIT_FAILURE);
	}
}

// ----------------------------------------------------------------------------
// Tic GTOD Clock
// ----------------------------------------------------------------------------
void TicLocal(GTODClock *gc)
{
	if(gettimeofday(&gc->tic_tv, NULL) != 0)
	{
		fprintf(stderr, "Error (TicLocalGTODClock): unable to get time of day!\n");
		exit(EXIT_FAILURE);
	}
}

// ----------------------------------------------------------------------------
// Tac GTOD Clock
// ----------------------------------------------------------------------------
void Tac()
{
	if(gettimeofday(&tac_tv, NULL) != 0)
	{
		fprintf(stderr, "Error (TacGTODClock): unable to get time of day!\n");
		exit(EXIT_FAILURE);
	}
}

// ----------------------------------------------------------------------------
// Tac GTOD Clock
// ----------------------------------------------------------------------------
void TacLocal(GTODClock *gc)
{
	if(gettimeofday(&gc->tac_tv, NULL) != 0)
	{
		fprintf(stderr, "Error (TacLocalGTODClock): unable to get time of day!\n");
		exit(EXIT_FAILURE);
	}
}

// ----------------------------------------------------------------------------
// Get elapsed time in seconds from the start
// ----------------------------------------------------------------------------
double GetElapsedTimeFromTheStart()
{
	double t1, t2;
	t1 = start_tv.tv_sec + (start_tv.tv_usec / 1000000.0); 
	t2 = tic_tv.tv_sec + (tic_tv.tv_usec / 1000000.0); 
	
	return (t2 - t1);
}

// ----------------------------------------------------------------------------
// Get Local elapsed time in seconds from the start
// ----------------------------------------------------------------------------
double GetLocalElapsedTimeFromTheStart(GTODClock *gc)
{
	double t1, t2;
	t1 = gc->start_tv.tv_sec + (gc->start_tv.tv_usec / 1000000.0); 
	t2 = gc->tic_tv.tv_sec + (gc->tic_tv.tv_usec / 1000000.0); 
	
	return (t2 - t1);
}

// ----------------------------------------------------------------------------
// Get elapsed time in seconds
// ----------------------------------------------------------------------------
double GetElapsedTime()
{
	double t1, t2;
	t1 = tic_tv.tv_sec + (tic_tv.tv_usec / 1000000.0); 
	t2 = tac_tv.tv_sec + (tac_tv.tv_usec / 1000000.0); 
	
	return (t2 - t1);
}

// ----------------------------------------------------------------------------
// Get Local elapsed time in seconds
// ----------------------------------------------------------------------------
double GetLocalElapsedTime(GTODClock *gc)
{
	double t1, t2;
	t1 = gc->tic_tv.tv_sec + (gc->tic_tv.tv_usec / 1000000.0); 
	t2 = gc->tac_tv.tv_sec + (gc->tac_tv.tv_usec / 1000000.0); 
	
	return (t2 - t1);
}