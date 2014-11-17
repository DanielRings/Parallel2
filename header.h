#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>
#include <iostream>

#define START_A 1
#define END_B 100
#define EPSILON 0.000001 // 10^-6
//#define EPSILON 0.0	// FOR DEBUGGING
#define SLOPE 12
#define GLOBAL_BUFF_SIZE 1000
#define LOCAL_BUFF_SIZE 1000
#define DEBUG_FREQ 1000

// For status of buffers
#define STATUS_EMPTY 0
#define STATUS_MID 1
#define STATUS_FULL 2

// For global buffer 
#define FUN_DEQUEUE 0
#define FUN_SINGLE_Q 1
#define FUN_DOUBLE_Q 2


// Global Stuff
extern double global_max; 
extern double * global_buffer;
extern int global_head; 
extern int global_tail; 
extern int global_status; 

// Initializes the global variables needed for the buffer
void global_initBuffer();

// Function we want to find the maximum of
double f(double x);

// Local Circular Queue
bool local_qWork(double c, double d, double * buffer, int * head, int * tail, int * status);

bool local_deqWork(double * c, double * d, double * buffer, int * head, int * tail, int * status);

// Global Circular Queue 
bool global_safeWorkBuffer(int function, double * c, double * d, double c2, double d2);

// Gives front value but does not pop it off the queue
bool local_peek(double * c, double * d, double * buffer, int * head, int * tail, int * status);

// Returns true only if max changed
bool local_setMax(double * currentMax, double fc, double fd);

// Returns true only if max changed
bool global_setMax(double fc, double fd);

// Returns true only if it is possible to get a higher value in this interval
bool validInterval(double currentMax, double c, double d);

// Attempts to rid itself of a piece of the interval handed to it
bool shrinkInterval(double currentMax, double * c, double * d);

// Returns space left in buffer 
int spaceLeft(int bufferSize, int head, int tail, int status);

// Returns true if all processors are done 
bool allDone(bool * doneArr, int size);

// Returns the amount of the remaining interval represented in the buffer 
// as a percentage
// FOR DEBUGGING
double intervalLeft(double originalSize, double * buffer, int bufferSize, int head, int tail, int status);

// Returns the average size of the subintervals in the buffer
// FOR DEBUGGING ONLY
double averageSubintervalSize(double * buffer, int bufferSize, int head, int tail, int status);

// Prints the intervals in the buffer
// FOR DEBUGGING ONLY
void printBuff(double * buffer, int bufferSize, int head, int tail, int count);

// FOR DEBUGGING
void spinWait();

#endif
