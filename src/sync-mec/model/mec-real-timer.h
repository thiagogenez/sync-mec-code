/*
 * mec-real-timer.h
 *
 *  Created on: 27 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_REAL_TIMER_H_
#define MEC_REAL_TIMER_H_

#include <iostream>
#include <chrono>

namespace ns3 {

class MecRealTimer {
public:
	MecRealTimer();
	virtual ~MecRealTimer();

	void Reset();
	double Elapsed() const;

private:
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1> > second_;
	std::chrono::time_point<clock_> beg_;
};

} /* namespace ns3 */

#endif /* MEC_REAL_TIMER_H_ */
