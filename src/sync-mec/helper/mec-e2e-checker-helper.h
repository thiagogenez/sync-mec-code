/*
 * mec-e2e-checker-helper.h
 *
 *  Created on: 18 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_E2E_CHECKER_HELPER_H_
#define MEC_E2E_CHECKER_HELPER_H_

#include <map>

#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/object.h"

#include "ns3/mec-v4-ping-helper.h"

namespace ns3 {

class MecE2ECheckerHelper: public Object {
public:

	static TypeId GetTypeId();

	MecE2ECheckerHelper();
	virtual ~MecE2ECheckerHelper();

	void Install(Ptr<Node> remote, NodeContainer &allNodes);
	void PongCallback( uint32_t nodeId,
			double rtt);

	void Install(Ptr<Node> remote, NodeContainer &allNodes, Time start,
			Time stop);

	double GetRtt(uint32_t) ;

	uint32_t GetId() const;

private:

	// general uses
	static uint32_t m_counter;
	uint32_t m_id;

	Ptr<Node> m_remote;
	std::map<uint32_t, double> m_rtt;
};

} /* namespace ns3 */

#endif /* MEC_E2E_CHECKER_HELPER_H_ */
