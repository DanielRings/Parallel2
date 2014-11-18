#include "helperfunctions.h"

double gMax; 
double *gBuffer;
int gHead; 
int gTail; 
int gStatus;

void gInitBuffer()
{
	gMax = 0;
	gBuffer = new double[GBUFFERSIZE]; 
	gHead = 0; 
	gTail = 0; 
	gStatus = 0; 

}

///////////////////////
bool gWorkBuffer(int function, double *c, double *d, double c2, double d2)
{
	bool ret = true; 
	#pragma omp critical
	{
		// Dequeue function
		if(function == FUN_DEQUEUE)
		{
			if(gStatus == 0)
				ret = false;
			else
			{
				// Get from circular buffer
				*c = gBuffer[gHead];
				*d = gBuffer[gHead+1]; 
				gHead = (gHead+2)%GBUFFERSIZE;  
				if(gTail == gHead)
					gStatus = 0;
				else
					gStatus = 1; 
			
			}

		}
		// Insert into buffer
		else
		{
			if(gStatus == 2)
			{
				ret = false;
			}
			else
			{
				// Check if inserting two intervals
				if(function == FUN_DOUBLE_Q)
				{
					if(spaceLeft(GBUFFERSIZE, gHead, gTail, gStatus) >= 4)
					{
						// Insert both intervals
						gBuffer[gTail] = *c;
						gBuffer[gTail+1] = *d; 
						gBuffer[gTail+2] = c2;
						gBuffer[gTail+3] = d2; 
						gTail = (gTail+4)%GBUFFERSIZE;  
					}
					else
					{
						// Cannot insert both succesfully so NONE will be inserted
						ret = false; 
					}
				}
				else
				{
					// Already checked to make sure it is not full so insert
					gBuffer[gTail] = *c;
					gBuffer[gTail+1] = *d; 
					gTail = (gTail+2)%GBUFFERSIZE;  
				}
				// Add to circular buffer
				if(gTail == gHead)
					gStatus = 2;
				else
					gStatus = 1; 
			}
		}
	}
	
	return ret; 
}

bool gSetMax(double fc, double fd)
{
	bool ret = false; 
	#pragma omp critical
	{
		if(gMax + E < fc)
		{
			gMax = fc + E;
			ret = true;
		}
		if(gMax + E < fd)
		{
			gMax = fd + E;
			ret = true;
		}
	}
	return ret; 
}

//the function
double f(double x)
{
	double outerSum = 0; 
	for(unsigned int i = 100; i >= 1; --i)
	{
		double innerSum = 0; 
		for(unsigned int j = i; j >= 1; --j)
		{
			innerSum += pow((x + j), -3.1);
		}
		
		outerSum += sin(x + innerSum)/pow(1.2, i);
	}

	return outerSum; 
}

// Local Circular Queue 
bool lWorkPush(double c, double d, double *buffer, int *head, int *tail, int *status)
{

	if(*status == 2)
	{
		return false;
	}
	else
	{
		// Add to circular buffer
		buffer[*tail] = c;
		buffer[*tail+1] = d; 
		*tail = (*tail+2)%LBUFFERSIZE;  
		if(*tail == *head)
			*status = 2;
		else
			*status = 1; 

		return true; 
	}
}

//////////////////////////////
bool lWorkPop(double *c, double *d, double *buffer, int *head, int *tail, int *status)
{
	if(*status == 0)
	{
		return false;
	}
	else
	{
		*c = buffer[*head];
		*d = buffer[*head+1]; 
		*head = (*head+2)%LBUFFERSIZE;  
		if(*tail == *head)
			*status = 0;
		else
			*status = 1; 

		return true; 
	}
}

bool intervalIsValid(double currentMax, double c, double d)
{
	if(S * (d - c) < E)
		return false; 
	if(((f(c) + f(d) + S*(d - c))/2) > (currentMax + E))
		return true; 
	else
		return false;
}

bool narrowInterval(double currentMax, double *c, double *d)
{
	double C = *c; 
	double D = *d; 
	
	// Left
	while(intervalIsValid(currentMax, C, D))
	{
		D = (D - C)/2 + C; 
	}
	*c = D;
	C = D; 
	D = *d; 	

	// Right
	while(intervalIsValid(currentMax, C, D))
	{
		C = (D - C)/2 + C; 
	}
	*d = C;

	return true;
}
 
int spaceLeft(int bufferSize, int head, int tail, int status)
{
	if(status == 0)
		return bufferSize;
	else if(status == 2)
		return 0; 
	else
	{
		if(tail > head)
			return bufferSize - (tail - head); 
		else
			return bufferSize - ((bufferSize - head) + tail); 		
	}
}

bool allThreadsFinished(bool *threadsFinished, int size)
{
	for(int i=0; i<size; i++)
	{
		if(!threadsFinished[i])
			return false;
	}
	
	return true; 
}

// FOR DEBUGGING
double intervalLeft(double originalSize, double *buffer, int bufferSize, int head, int tail, int status)
{
	if(status == 0)
		return 0; 
	else 
	{
		double runSum = 0; 
		do
		{
			runSum += (buffer[head+1] - buffer[head]);
			head = (head+2)%bufferSize;
		}while(head != tail);
		
		return 100*runSum/originalSize; 
	}
}

// Returns the average size of the subintervals in the buffer
// FOR DEBUGGING ONLY
double averageSubintervalSize(double *buffer, int bufferSize, int head, int tail, int status)
{
	if(status == 0)
		return 0; 
	else
	{
		double runSum = 0;
		int itemCount = 0;  
		do
		{
			runSum += (buffer[head+1] - buffer[head]);
			head = (head+2)%bufferSize;
			itemCount++; 
		}while(head != tail);
		return runSum/(itemCount); 
	}
}

// Prints the intervals in the buffer
// FOR DEBUGGING ONLY
void printBuff(double *buffer, int bufferSize, int head, int tail, int count)
{
	int iterCount = 0;  
	do
	{
		printf("[%f, %f]\t", buffer[head], buffer[head+1]);
		head = (head+2)%bufferSize;
		iterCount++; 
	}while(head != tail && iterCount < count);
	
	printf("\n");  
}

// FOR DEBUGGING
void spinWait()
{
	while(1);
}
