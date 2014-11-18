// Tentative questions: 
// In email, when describing f(x), the 3rd line is "z = x". Why isn't it z = 0? 
// Try setting number of threads to 1.5X number of procs to do hyperthreading? 
// ARE COMPILER OPTIMIZATIONS ALLOWED!?!?!? -O3? 
// Change order by which elements are added to global buffer 
// 	try to take elements from the from of queue rather than those that would have been 
//	added to the back.

#include "helperfunctions.h"

using namespace std;

int main(int argc, char** argv)
{
	if(argc != 5){
		cout << "Wrong number of arguments\n";
		return 0;
	}
	const int A = strtod(argv[1], NULL);
	const int B = strtod(argv[2], NULL);
	const int E = strtod(argv[3], NULL);
	const int S = strtod(argv[4], NULL);
	cout << A << "-" << B << "-" << E << "-" << S << endl;
	int threads = omp_get_num_procs();
	//int threads = 2;
	omp_set_num_threads(threads);

	// Vars to be used
	// double intervalSpan = END_B - START_A;
	// double chunkSize = intervalSpan/threads;
	double chunkSize = (END_B - START_A)/threads;

	printf("\nNumber of threads: %d\n", threads);

	gInitBuffer();
	bool *gThreadsFinished = new bool[threads];
	for(int i=0; i<threads; i++)
	{
		gThreadsFinished[i] = false;
	}
	
	double start = omp_get_wtime();
	#pragma omp parallel
	{
		// Init local variables
		double lBuffer[LOCAL_BUFF_SIZE];
		double lC = 0;
		double lD = 0;
		int lHead = 0;
		int lTail = 0;
		int lStatus = 0;
	
		// Add init interval to queue
		int threadID = omp_get_thread_num();
		lWorkQueue(threadID*chunkSize+START_A, (threadID+1)*chunkSize+START_A, lBuffer, &lHead, &lTail, &lStatus);

		// Print each thread's interval
		//printf("Thread %d: [%f, %f]\n", threadID, threadID*chunkSize+START_A, (threadID+1)*chunkSize+START_A);
		
		int debugCount = 0;

		bool lContinue = true;
		while(lContinue)
		{
			
			// FOR DEBUGGING
			debugCount++; 
			if(debugCount == DEBUG_FREQ)
			{
				//printBuff(lBuffer, LOCAL_BUFF_SIZE, lHead, lTail, 10); 
				printf("GlobalSpaceLeft: %d\t", spaceLeft(GLOBAL_BUFF_SIZE, gHead, gTail, gStatus));
				printf("tNum: %d\tStatus: %d\tSpacLeft: %d\t\tCurMax: %2.30f\tPercentLeft: %f\tAvgSubIntSize: %1.8f\n", threadID, lStatus, spaceLeft(LOCAL_BUFF_SIZE, lHead, lTail, lStatus), gMax, intervalLeft(END_B-START_A, lBuffer, LOCAL_BUFF_SIZE, lHead, lTail, lStatus), averageSubintervalSize(lBuffer, LOCAL_BUFF_SIZE, lHead, lTail, lStatus));
				debugCount = 0; 
			}
			
			bool cont = false;	
			// Get work from a queue
			if(lStatus != 0)
			{
				// Local buffer still has work so we get some from there
				lWorkDeque(&lC, &lD, lBuffer, &lHead, &lTail, &lStatus);
				gThreadsFinished[threadID] = false; 
				cont = true; 
			}
			
			else
			{
				gThreadsFinished[threadID] = true; 
				while(!allThreadsFinished(gThreadsFinished, threads) && !cont)
				{
					if(gStatus != 0)
					{
						cont = gWorkBuffer(FUN_DEQUEUE, &lC, &lD, 0, 0);
					}
				}
				if(cont)
				{
					gThreadsFinished[threadID] = false; 
				}
			}
		
			if(cont)
			{	
				// Check if possible larger
				if(intervalIsValid(gMax, lC, lD))
				{
					gSetMax(f(lC), f(lD)); 
					
					// IF FULL, SEND WORK TO GLOBAL BUFF AT A RATE DETERMINED BY A CONSTANT

					// Two intervals will not fit in local buffer
					if(spaceLeft(LOCAL_BUFF_SIZE, lHead, lTail, lStatus) == 2)
					{
						// Global buffer is full too - so we shrink the current interval instead of splitting it
						if(gStatus == 2)
						{
							// NEED TO FIX THIS FUNCTION BELOW
							narrowInterval(gMax, &lC, &lD);
							// Queue up shrunken interval back into local buffer
							lWorkQueue(lC, lD, lBuffer, &lHead, &lTail, &lStatus); 
						}
						else 
						{
							double pC = lC;
							double pD = ((lD-lC)/2)+lC;
							double pC2 = ((lD-lC)/2)+lC;
							double pD2 = lD; 
							if(!gWorkBuffer(FUN_DOUBLE_Q, &pC, &pD, pC2, pD2))
							{
								narrowInterval(gMax, &lC, &lD);
								lWorkQueue(lC, lD, lBuffer, &lHead, &lTail, &lStatus); 
							}
								
						}
					}
					else
					{
						lWorkQueue(lC, ((lD-lC)/2)+lC, lBuffer, &lHead, &lTail, &lStatus);
						lWorkQueue(((lD-lC)/2)+lC, lD, lBuffer, &lHead, &lTail, &lStatus);	
					}
				}
			}
			else
			{
				lContinue = false; 
			}
		}
		gThreadsFinished[threadID] = true; 	
	} // END PARALLEL
	double end = omp_get_wtime();

	//delete[] gThreadsFinished;
	printf("Max = %2.30f\nTime=%f", gMax, end-start); 
	return 0;
}
