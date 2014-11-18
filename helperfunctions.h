#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <omp.h>

#define GLOBAL_BUFF_SIZE 50000
#define LOCAL_BUFF_SIZE 10
#define DEBUG_FREQ 100

// For global buffer 
#define FUN_DEQUEUE 0
#define FUN_SINGLE_Q 1
#define FUN_DOUBLE_Q 2

// Start, End, Epsilon, and Slope
#define A 1
#define B 100
#define E 0.000001
#define S 12

// Global Stuff
extern double gMax; 
extern double *gBuffer;
extern int gHead; 
extern int gTail; 
extern int gStatus; 

// Initializes the global variables needed for the buffer
void gInitBuffer();

// Global Circular Queue 
bool gWorkBuffer(int function, double *c, double *d, double c2, double d2);

// Returns true only if max changed
bool gSetMax(double fc, double fd);

// Function we want to find the maximum of
double f(double x);

// Local Circular Queue
bool lWorkQueue(double c, double d, double *buffer, int *head, int *tail, int *status);

bool lWorkDeque(double *c, double *d, double *buffer, int *head, int *tail, int *status);

// Returns true only if it is possible to get a higher value in this interval
bool intervalIsValid(double currentMax, double c, double d);

// Attempts to rid itself of a piece of the interval handed to it
bool narrowInterval(double currentMax, double *c, double *d);

// Returns space left in buffer 
int spaceLeft(int bufferSize, int head, int tail, int status);

// Returns true if all processors are done 
bool allThreadsFinished(bool *threadsFinished, int size);

// Returns the amount of the remaining interval represented in the buffer 
// as a percentage
// FOR DEBUGGING
double intervalLeft(double originalSize, double *buffer, int bufferSize, int head, int tail, int status);

// Returns the average size of the subintervals in the buffer
// FOR DEBUGGING ONLY
double averageSubintervalSize(double *buffer, int bufferSize, int head, int tail, int status);

// Prints the intervals in the buffer
// FOR DEBUGGING ONLY
void printBuff(double *buffer, int bufferSize, int head, int tail, int count);

// FOR DEBUGGING
void spinWait();

#endif
