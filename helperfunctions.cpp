#include "helperfunctions.h"


// Global Stuff
double gMax; 
double *gBuffer;
int gHead; 
int gTail; 
int gStatus; 

void gInitBuffer()
{
	gMax = 0;  
	gBuffer = new double[GLOBAL_BUFF_SIZE]; 
	gHead = 0; 
	gTail = 0; 
	gStatus = 0; 

}

// Global Circular Queue 
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
				gHead = (gHead+2)%GLOBAL_BUFF_SIZE;  
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
					if(spaceLeft(GLOBAL_BUFF_SIZE, gHead, gTail, gStatus) >= 4)
					{
						// Insert both intervals
						gBuffer[gTail] = *c;
						gBuffer[gTail+1] = *d; 
						gBuffer[gTail+2] = c2;
						gBuffer[gTail+3] = d2; 
						gTail = (gTail+4)%GLOBAL_BUFF_SIZE;  
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
					gTail = (gTail+2)%GLOBAL_BUFF_SIZE;  
				}
				// Add to circular buffer
				if(gTail == gHead)
					gStatus = 2;
				else
					gStatus = 1; 
			}
		}
	} // End Critical Section
	
	return ret; 
}

// Returns true only if max changed
bool gSetMax(double fc, double fd, double e)
{
	bool ret = false; 
	#pragma omp critical
	{
		if(gMax + e < fc)
		{
			gMax = fc;
			ret = true;
		}
		if(gMax + e < fd)
		{
			gMax = fd;
			ret = true;
		}
	}
	return ret; 
}

// Function we want to find the maximum of
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
bool lWorkQueue(double c, double d, double *buffer, int *head, int *tail, int *status)
{
	if((*tail < 0) || ((*tail + 1) > (LOCAL_BUFF_SIZE -1)))
	{
		while(1)
		{ 
			printf(" OUTOUTOUTOUT");
		}
	}
	if(*status == 2)
	{
		return false;
	}
	else
	{
		// Add to circular buffer
		buffer[*tail] = c;
		buffer[*tail+1] = d; 
		*tail = (*tail+2)%LOCAL_BUFF_SIZE;  
		if(*tail == *head)
			*status = 2;
		else
			*status = 1; 

		return true; 
	}
}

bool lWorkDeque(double *c, double *d, double *buffer, int *head, int *tail, int *status)
{
	if((*head < 0) || ((*head + 1) > (LOCAL_BUFF_SIZE -1)))
	{
		while(1)
		{ 
			printf(" OUTOUTOUTOUT");
		}
	}
	if(*status == 0)
	{
		return false;
	}
	else
	{
		// Get from circular buffer
		*c = buffer[*head];
		*d = buffer[*head+1]; 
		*head = (*head+2)%LOCAL_BUFF_SIZE;  
		if(*tail == *head)
			*status = 0;
		else
			*status = 1; 

		return true; 
	}
}

// Returns true only if it is possible to get a higher value in this interval
bool intervalIsValid(double currentMax, double c, double d, double s, double e)
{
	if(s * (d - c) < e)
		return false; 
	if(((f(c) + f(d) + s*(d - c))/2) > (currentMax + e))
		return true; 
	else
		return false;
}

// Attempts to rid itself of a piece of the interval handed to it
bool narrowInterval(double currentMax, double *c, double *d, double s, double e)
{
	// Save the original values
	double C = *c; 
	double D = *d; 
	
	// Shrink from the left side
	while(intervalIsValid(currentMax, C, D, s, e))
	{
		//printf("stuck"); 
		D = (D - C)/2 + C; 
	}

	//printf("\nNOT STUCK\n"); 	
	*c = D;
	C = D; 
	D = *d; 	

	// Shrink from the right side
	while(intervalIsValid(currentMax, C, D, s, e))
	{
		C = (D - C)/2 + C; 
	}

	*d = C; 
	//*c = retC; 
	
	//printf("Getting Out"); 
	// THIS SHOULD CHECK IF FAILED OR NOT, SOMEHOW? 
	return true;
}

// Returns space left in buffer 
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

// THIS VERSION ONLY WORKS WITH STACK VERSION OF q/deqWork
/*int spaceLeft(int bufferSize, int head, int tail, int status)
{
	if(status == 0)
		return bufferSize;
	else if(status == 2)
		return 0; 
	else
	{
		return (LOCAL_BUFF_SIZE-(head - 2));
	}
}*/

// Returns true if all processors are done
bool allThreadsFinished(bool *threadsFinished, int size)
{
	for(int i=0; i<size; i++)
	{
		if(!threadsFinished[i])
			return false;
	}
	
	return true; 
}

// Returns the amount of the remaining interval represented in the buffer 
// as a percentage
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
