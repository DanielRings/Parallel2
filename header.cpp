#include "header.h"


// Global Stuff
double global_max; 
double * global_buffer;
int global_head; 
int global_tail; 
int global_status; 

void global_initBuffer()
{
	global_max = 0;  
	global_buffer = new double[GLOBAL_BUFF_SIZE]; 
	global_head = 0; 
	global_tail = 0; 
	global_status = STATUS_EMPTY; 

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
bool local_qWork(double c, double d, double * buffer, int * head, int * tail, int * status)
{
	if((*tail < 0) || ((*tail + 1) > (LOCAL_BUFF_SIZE -1)))
	{
		while(1)
		{ 
			printf(" OUTOUTOUTOUT");
		}
	}
	if(*status == STATUS_FULL)
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
			*status = STATUS_FULL;
		else
			*status = STATUS_MID; 

		return true; 
	}
}

bool local_deqWork(double * c, double * d, double * buffer, int * head, int * tail, int * status)
{
	if((*head < 0) || ((*head + 1) > (LOCAL_BUFF_SIZE -1)))
	{
		while(1)
		{ 
			printf(" OUTOUTOUTOUT");
		}
	}
	if(*status == STATUS_EMPTY)
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
			*status = STATUS_EMPTY;
		else
			*status = STATUS_MID; 

		return true; 
	}
}


// Global Circular Queue 
bool global_safeWorkBuffer(int function, double * c, double * d, double c2, double d2)
{
	bool ret = true; 
	#pragma omp critical
	{
		// Dequeue function
		if(function == FUN_DEQUEUE)
		{
			if(global_status == STATUS_EMPTY)
				ret = false;
			else
			{
				// Get from circular buffer
				*c = global_buffer[global_head];
				*d = global_buffer[global_head+1]; 
				global_head = (global_head+2)%GLOBAL_BUFF_SIZE;  
				if(global_tail == global_head)
					global_status = STATUS_EMPTY;
				else
					global_status = STATUS_MID; 
			
			}

		}
		// Insert into buffer
		else
		{
			if(global_status == STATUS_FULL)
			{
				ret = false;
			}
			else
			{
				// Check if inserting two intervals
				if(function == FUN_DOUBLE_Q)
				{
					if(spaceLeft(GLOBAL_BUFF_SIZE, global_head, global_tail, global_status) >= 4)
					{
						// Insert both intervals
						global_buffer[global_tail] = *c;
						global_buffer[global_tail+1] = *d; 
						global_buffer[global_tail+2] = c2;
						global_buffer[global_tail+3] = d2; 
						global_tail = (global_tail+4)%GLOBAL_BUFF_SIZE;  
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
					global_buffer[global_tail] = *c;
					global_buffer[global_tail+1] = *d; 
					global_tail = (global_tail+2)%GLOBAL_BUFF_SIZE;  
				}
				// Add to circular buffer
				if(global_tail == global_head)
					global_status = STATUS_FULL;
				else
					global_status = STATUS_MID; 
			}
		}
	} // End Critical Section
	
	return ret; 
}

// Gives front value but does not pop it off the queue
bool local_peek(double * c, double * d, double * buffer, int * head, int * tail, int * status)
{
	if(*status == STATUS_EMPTY)
	{
		return false;
	}
	else
	{
		// Add to circular buffer
		*c = buffer[*head];
		*d = buffer[*head+1]; 
		return true; 
	}
}

// Returns true only if max changed
bool local_setMax(double * currentMax, double fc, double fd)
{
	if(*currentMax + EPSILON < fc)
		*currentMax = fc;
	else if(*currentMax + EPSILON < fd)
		*currentMax = fd; 
	else 
		return false; 

	return true; 
}

// Returns true only if max changed
bool global_setMax(double fc, double fd)
{
	bool ret = true; 
	#pragma omp critical
	{
		if(global_max + EPSILON < fc)
		{
			global_max = fc; 
		}
		else if(global_max + EPSILON < fd)
		{
			global_max = fd; 
		}
		else
		{ 
			ret = false; 
		}
	}
	return ret; 
}

// Returns true only if it is possible to get a higher value in this interval
bool validInterval(double currentMax, double c, double d)
{
	if((d - c) < EPSILON)
		return false; 
	if(((f(c) + f(d) + SLOPE*(d - c))/2) > (currentMax + EPSILON))
		return true; 
	else
		return false;
}

// Does same this as validInterval() but also updates the max
bool validIntervalAndMax(double * currentMax, double c, double d)
{
	double fC = f(c); 
	double fD = f(d); 
	if(local_setMax(currentMax, fC, fD))
	{
		return true; 
	}
	else
	{
		if(((fC + fD + SLOPE*(d - c))/2) > (*currentMax + EPSILON))
			return true; 
		else
			return false;
	}
}



// Attempts to rid itself of a piece of the interval handed to it
bool shrinkInterval(double currentMax, double * c, double * d)
{
	// Save the original values
	double C = *c; 
	double D = *d; 
	
	// Shrink from the left side
	while(validInterval(currentMax, C, D))
	{
		//printf("stuck"); 
		D = (D - C)/2 + C; 
	}

	//printf("\nNOT STUCK\n"); 	
	*c = D;
	C = D; 
	D = *d; 	

	// Shrink from the right side
	while(validInterval(currentMax, C, D))
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
	if(status == STATUS_EMPTY)
		return bufferSize;
	else if(status == STATUS_FULL)
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
	if(status == STATUS_EMPTY)
		return bufferSize;
	else if(status == STATUS_FULL)
		return 0; 
	else
	{
		return (LOCAL_BUFF_SIZE-(head - 2));
	}
}*/

// Returns true if all processors are done
bool allDone(bool * doneArr, int size)
{
	for(int i=0; i<size; i++)
	{
		if(!doneArr[i])
			return false;
	}
	
	return true; 
}

// Returns the amount of the remaining interval represented in the buffer 
// as a percentage
// FOR DEBUGGING
double intervalLeft(double originalSize, double * buffer, int bufferSize, int head, int tail, int status)
{
	if(status == STATUS_EMPTY)
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
double averageSubintervalSize(double * buffer, int bufferSize, int head, int tail, int status)
{
	if(status == STATUS_EMPTY)
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
void printBuff(double * buffer, int bufferSize, int head, int tail, int count)
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
