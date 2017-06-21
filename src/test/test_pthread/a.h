#ifndef A_H
#define A_H

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct {

	size_t staff_number; /* default 10 */

	size_t cleaners_number; /* default 1 */
	size_t cook_number; /* default 1 */
	size_t dishwasher_number; /* default 1 */
	size_t lobby_manager_number; /* default 1 */
	size_t reception_number; /* default 1 */
	size_t typist_number; /* default 1 */
	size_t waiter_number; /* default x */


} a_t;


#include "common.h"


#include "reception.h"
#include "waiter.h"

#endif
