#ifndef A_H
#define A_H

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct a_s {

	size_t staff_number; // NOTE: default 10

	size_t cleaners_number; // NOTE: default 1
	size_t cook_number; // NOTE: default 1
	size_t dishwasher_number; // NOTE: default 1
	size_t lobby_manager_number; // NOTE: default 1
	size_t reception_number; // NOTE: default 1
	size_t typist_number; // NOTE: default 1
	size_t waiter_number; // NOTE: default x


} a_t;


#include "common.h"


#include "reception.h"
#include "waiter.h"

#endif
