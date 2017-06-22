#ifndef A_H
#define A_H

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {

	/* Human Resources */
	size_t staff_number; /* default 10 */

	/* Mangement */
	size_t chairman_number; /* only one */
	size_t general_manger_number; /* only one */
	size_t CEO_number; /* Chief Execute Officer: only one */

	/* Junior */
	size_t cleaners_number; /* default 1 */
	size_t cook_number; /* default 1 */
	size_t dishwasher_number; /* default 1 */
	size_t lobby_manager_number; /* default 1 */
	size_t reception_number; /* default 1 */
	size_t typist_number; /* default 1 */
	size_t waiter_number; /* default x */


	/* Logging */
	size_t cache_max;
	size_t size_max;


} a_t;


#include "common.h"


#include "reception.h"
#include "waiter.h"

#endif
