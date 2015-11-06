/*
 * utils.h
 *
 *  Created on: Nov 7, 2015
 *      Author: rafael
 */

#ifndef SRC_INCLUDE_UTILS_H_
#define SRC_INCLUDE_UTILS_H_

#include "stdio.h"

#define DEBUG(msg) \
	do { \
		printf("%s %d [%s] %s\n", __FILE__, __LINE__, __func__, #msg); \
	} while (0)



#endif /* SRC_INCLUDE_UTILS_H_ */
