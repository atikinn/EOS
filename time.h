/*
 * time.h
 *
 *  Created on: May 16, 2014
 *      Author: isinyagin
 */

#ifndef TIME_H_
#define TIME_H_

struct tval {
	uint64_t tval_sec;
	uint16_t tval_usec;
};

void getcurrenttime(struct tval *ptr);
void setcurrenttime(uint64_t *time);
void setpdb(long sec, int (*fcn_p)(void));

#endif /* TIME_H_ */
