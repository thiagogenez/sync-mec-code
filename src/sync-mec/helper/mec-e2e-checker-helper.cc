/*
 * mec-e2e-checker-helper.cc
 *
 *  Created on: 18 Jul 2017
 *      Author: thiagogenez
 */

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/mec-utils-helper.h"
#include "ns3/mec-constants-helper.h"
#include "mec-e2e-checker-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MecE2ECheckerHelper");
NS_OBJECT_ENSURE_REGISTERED(MecE2ECheckerHelper);

//Id initialisation
uint32_t MecE2ECheckerHelper::m_counter = 0;

MecE2ECheckerHelper::MecE2ECheckerHelper() {

	m_id = MecE2ECheckerHelper::m_counter;
	MecE2ECheckerHelper::m_counter++;

	NS_LOG_FUNCTION(this << m_id);
}

MecE2ECheckerHelper::~MecE2ECheckerHelper() {
	NS_LOG_FUNCTION(this);
}

TypeId MecE2ECheckerHelper::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::MecE2ECheckerHelper").SetParent<Object>().SetGroupName(
					"Network");
	return tid;
}

void MecE2ECheckerHelper::PongCallback(uint32_t pingerId, double rtt) {

	NS_LOG_FUNCTION(this << "from" <<pingerId <<" rtt" << rtt);

//	std::cout << "Time=" << Simulator::Now() << "\tPing from: " << pingerId
//			<< " to " << m_remote->GetId() << " " << rtt << "ms\thelperId="
//			<< GetId() << std::endl;

	std::map<uint32_t, double>::iterator it = m_rtt.find(pingerId);
	if (it != m_rtt.end()) {
		it->second = rtt;
	}

	else {
		m_rtt.insert(std::pair<uint32_t, double>(pingerId, rtt));
	}

}

uint32_t MecE2ECheckerHelper::GetId() const {
	return m_id;
}

void MecE2ECheckerHelper::Install(Ptr<Node> remote, NodeContainer &allNodes,
		Time start, Time stop) {
	NodeContainer pingerNodes;

	m_remote = remote;
	for (uint32_t i = 0; i < allNodes.GetN(); i++) {
		Ptr<Node> node = allNodes.Get(i);

		if (node->GetId() != remote->GetId()) {
			pingerNodes.Add(node);
		}
	}

	// create the ping application for each pinger node
	ApplicationContainer pingerApps;

	// pong callback
	Callback<void, uint32_t, double> pongCallback = MakeCallback(
			&MecE2ECheckerHelper::PongCallback, this);

	// if there is at least one pinger node
	if (pingerNodes.GetN() > 0) {

		// create the remote ping (target)
		MecV4PingHelper remotePing = MecV4PingHelper(
				MecUtilsHelper::GetIpv4Address(remote, 1, 0));

		// create the ping application for each pinger node
		pingerApps = remotePing.Install(pingerNodes, pongCallback,
				MecConstantsHelper::E2E_CHECKER_PING_INTERVAL_TIME);

	}

	pingerApps.Start(start);
	pingerApps.Stop(stop);
}

double MecE2ECheckerHelper::GetRtt(uint32_t pingerId) {

	std::map<uint32_t, double>::iterator it;
	it = m_rtt.find(pingerId);
	if (it != m_rtt.end()) {
		return it->second;
	}

	return std::numeric_limits<double>::max();
}

} /* namespace ns3 */
