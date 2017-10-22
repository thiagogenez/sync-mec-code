/*
 * mec-real-timer.cc
 *
 *  Created on: 27 Jul 2017
 *      Author: thiagogenez
 */

#include "mec-real-timer.h"

namespace ns3 {

MecRealTimer::MecRealTimer() :
		beg_(clock_::now()) {

}

MecRealTimer::~MecRealTimer() {
}

void MecRealTimer::Reset() {
	beg_ = clock_::now();
}
double MecRealTimer::Elapsed() const {
	return std::chrono::duration_cast<second_>(clock_::now() - beg_).count();
}

} /* namespace ns3 */
