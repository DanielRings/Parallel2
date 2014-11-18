#include "helperfunctions.h"

using namespace std;

int main(int argc, char** argv)
{
	int threads = omp_get_num_procs();
	omp_set_num_threads(threads);

	double initialSplit = (B - A)/threads;

	gInitBuffer();
	bool *gThreadsFinished = new bool[threads];
	for(int i=0; i<threads; i++)
	{
		gThreadsFinished[i] = false;
	}
	
	double start = omp_get_wtime();
	#pragma omp parallel
	{
		double lBuffer[LBUFFERSIZE];
		double lC = 0;
		double lD = 0;
		int lHead = 0;
		int lTail = 0;
		int lStatus = 0;
		int threadID = omp_get_thread_num();
		lWorkPush(threadID*initialSplit+A, (threadID+1)*initialSplit+A, lBuffer, &lHead, &lTail, &lStatus);

		bool lContinue = true;
		while(lContinue)
		{
			bool cont = false;	
			
			// If local buffer isn't empty, work on local buffer
			if(lStatus != 0)
			{
				lWorkPop(&lC, &lD, lBuffer, &lHead, &lTail, &lStatus);
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
						cont = gWorkBuffer(0, &lC, &lD, 0, 0);
					}
				}
				if(cont)
				{
					gThreadsFinished[threadID] = false; 
				}
			}
		
			if(cont)
			{
				if(intervalIsValid(gMax, lC, lD))
				{
					gSetMax(f(lC), f(lD));
					
					if(spaceLeft(LBUFFERSIZE, lHead, lTail, lStatus) == 2)
					{
						// Global buffer is full too. Interval must be
						// narrowed rather than cut in half (avoid if possible)
						if(gStatus == 2)
						{
							narrowInterval(gMax, &lC, &lD);
							lWorkPush(lC, lD, lBuffer, &lHead, &lTail, &lStatus); 
						}
						else 
						{
							double pC = lC;
							double pD = ((lD-lC)/2)+lC;
							double pC2 = ((lD-lC)/2)+lC;
							double pD2 = lD;
							if(!gWorkBuffer(1, &pC, &pD, pC2, pD2))
							{
								narrowInterval(gMax, &lC, &lD);
								lWorkPush(lC, lD, lBuffer, &lHead, &lTail, &lStatus); 
							}
								
						}
					}
					else
					{
						lWorkPush(lC, ((lD-lC)/2)+lC, lBuffer, &lHead, &lTail, &lStatus);
						lWorkPush(((lD-lC)/2)+lC, lD, lBuffer, &lHead, &lTail, &lStatus);	
					}
				}
			}
			else
			{
				lContinue = false; 
			}
		}
		gThreadsFinished[threadID] = true; 	
	}
	double end = omp_get_wtime();
	printf("Max: %4.28f\n",gMax);
	cout << "Time: " << end-start << "\n";
	return 0;
}
