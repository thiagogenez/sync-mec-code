/*
 * mec-e2e-checker.h
 *
 *  Created on: 18 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_E2E_CHECKER_H_
#define MEC_E2E_CHECKER_H_

#include<map>

#include "ns3/mec-e2e-checker-helper.h"
#include "ns3/mec-backhaul-core-helper.h"

#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/node.h"

namespace ns3 {

class MecE2EChecker {
public:
	MecE2EChecker();
	virtual ~MecE2EChecker();

	static double GetRtt(Ptr<Node>, Ptr<Node>);


	static void Install(Ptr<MecBackhaulCoreHelper> backhaul, Time const &start,
			Time const &stop);

private:

	static std::map<uint32_t, Ptr<MecE2ECheckerHelper>> m_rtt;

	MecBackhaulCoreHelper m_backhaul;
};

} /* namespace ns3 */

#endif /* MEC_E2E_CHECKER_H_ */
