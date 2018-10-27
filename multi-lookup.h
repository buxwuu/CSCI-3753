//
//  multi-lookup.h
//  
//
//  Created by Kevin Kirk on 10/27/18.
//

#ifndef MULTI_H
#define MULTI_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#include "util.h"
#include "queue.h"


/* Read in name files*/
void* readUrl(void* file);
/* Resolve Ip's and write results */
void* resolve(void* tid);
/* Initialize variables */
void initialize();
/* Destroy and clean up*/
void cleanUp();

#endif
