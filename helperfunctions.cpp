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

bool gWorkBuffer(int function, double *c, double *d, double c2, double d2)
{
	bool r = true; 
	#pragma omp critical
	{
		//Pop
		if(function == 0)
		{
			if(gStatus == 0)
				r = false;
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
		// Push (1 or 2)
		else
		{
			if(gStatus == 2)
			{
				r = false;
			}
			else
			{
				// Push 2
				if(function == 1)
				{
					if(spaceLeft(GBUFFERSIZE, gHead, gTail, gStatus) >= 4)
					{
						gBuffer[gTail] = *c;
						gBuffer[gTail+1] = *d; 
						gBuffer[gTail+2] = c2;
						gBuffer[gTail+3] = d2; 
						gTail = (gTail+4)%GBUFFERSIZE;  
					}
					else
					{
						// Push 2 Fails
						r = false;
					}
				}
				else
				{
					// Push 1
					gBuffer[gTail] = *c;
					gBuffer[gTail+1] = *d; 
					gTail = (gTail+2)%GBUFFERSIZE;  
				}
				if(gTail == gHead)
					gStatus = 2;
				else
					gStatus = 1; 
			}
		}
	}
	
	return r; 
}

bool gSetMax(double fc, double fd)
{
	bool r = false; 
	#pragma omp critical
	{
		if(gMax + E < fc)
		{
			gMax = fc + E;
			r = true;
		}
		if(gMax + E < fd)
		{
			gMax = fd + E;
			r = true;
		}
	}
	return r; 
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

bool lWorkPush(double c, double d, double *buffer, int *head, int *tail, int *status)
{

	if(*status == 2)
	{
		return false;
	}
	else
	{
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