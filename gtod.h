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
// | Description: functions headers for gtod module                           |
// +--------------------------------------------------------------------------+

#ifndef GTOD_HEADER
#define GTOD_HEADER

// ----------------------------------------------------------------------------
// SimClock data structure
// ----------------------------------------------------------------------------
typedef struct
{
  struct timeval start_tv; 
  struct timeval tic_tv;
  struct timeval tac_tv;
} GTODClock;

// ----------------------------------------------------------------------------
// Init GTOD Clocks
// ----------------------------------------------------------------------------
void InitClock();

// ----------------------------------------------------------------------------
// Init Local GTOD Clocks
// ----------------------------------------------------------------------------
void InitLocalClock(GTODClock *gc);

// ----------------------------------------------------------------------------
// Tic GTOD Clock
// ----------------------------------------------------------------------------
void Tic();

// ----------------------------------------------------------------------------
// Tic GTOD Clock
// ----------------------------------------------------------------------------
void TicLocal(GTODClock *gc);

// ----------------------------------------------------------------------------
// Tac GTOD Clock
// ----------------------------------------------------------------------------
void Tac();

// ----------------------------------------------------------------------------
// Tac GTOD Clock
// ----------------------------------------------------------------------------
void TacLocal(GTODClock *gc);

// ----------------------------------------------------------------------------
// Get elapsed time in seconds from the start
// ----------------------------------------------------------------------------
double GetElapsedTimeFromTheStart();

// ----------------------------------------------------------------------------
// Get Local elapsed time in seconds from the start
// ----------------------------------------------------------------------------
double GetLocalElapsedTimeFromTheStart(GTODClock *gc);

// ----------------------------------------------------------------------------
// Get elapsed time in seconds
// ----------------------------------------------------------------------------
double GetElapsedTime();

// ----------------------------------------------------------------------------
// Get Local elapsed time in seconds
// ----------------------------------------------------------------------------
double GetLocalElapsedTime(GTODClock *gc);
	
	
#endif