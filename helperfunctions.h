#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <omp.h>

#define GBUFFERSIZE 128000
#define LBUFFERSIZE 8
#define DEBUG_FREQ 5000

// For global buffer 
#define FUN_DEQUEUE 0
#define FUN_SINGLE_Q 1
#define FUN_DOUBLE_Q 2

// Start, End, Epsilon, and Slope
#define A 1
#define B 100
#define E 0.000001
#define S 12

extern double gMax; 
extern double *gBuffer;
extern int gHead; 
extern int gTail; 
extern int gStatus; 

void gInitBuffer();
 
bool gWorkBuffer(int function, double *c, double *d, double c2, double d2);

bool gSetMax(double fc, double fd);

// the function
double f(double x);

bool lWorkPush(double c, double d, double *buffer, int *head, int *tail, int *status);

bool lWorkPop(double *c, double *d, double *buffer, int *head, int *tail, int *status);

bool intervalIsValid(double currentMax, double c, double d);

bool narrowInterval(double currentMax, double *c, double *d);

int spaceLeft(int bufferSize, int head, int tail, int status);
 
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