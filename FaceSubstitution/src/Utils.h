/*
 * Utils.h
 *
 *  Created on: 04/03/2012
 *      Author: arturo
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "ofMath.h"

inline int randomDifferent(int low, int high, int old) {
	int cur = ofRandom(low, high - 1);
	if(cur >= old) {
		cur++;
		cur = cur % high;
	}
	return cur;
}



#endif /* UTILS_H_ */
