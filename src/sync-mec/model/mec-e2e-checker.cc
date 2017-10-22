/*
 * mec-e2e-checker.cc
 *
 *  Created on: 18 Jul 2017
 *      Author: thiagogenez
 */

#include "ns3/double.h"
#include "ns3/node-container.h"
#include "mec-e2e-checker.h"

namespace ns3 {

std::map<uint32_t, Ptr<MecE2ECheckerHelper>> MecE2EChecker::m_rtt;

MecE2EChecker::MecE2EChecker() {

}

MecE2EChecker::~MecE2EChecker() {
}

double MecE2EChecker::GetRtt(Ptr<Node> from, Ptr<Node> to) {
	std::map<uint32_t, Ptr<MecE2ECheckerHelper>>::iterator it =
			MecE2EChecker::m_rtt.find(from->GetId());

	if(from->GetId() == to->GetId()){
		return 0;
	}

	if (it != MecE2EChecker::m_rtt.end()) {
		Ptr<MecE2ECheckerHelper> helper = it->second;

		return helper->GetRtt(to->GetId());
	}

	return std::numeric_limits<double>::max();;
}

void MecE2EChecker::Install(Ptr<MecBackhaulCoreHelper> backhaul,
		Time const &start, Time const & stop) {
	NodeContainer allNodes = backhaul->AllNodes();

	for (uint32_t i = 0; i < allNodes.GetN(); i++) {
		Ptr<Node> node = allNodes.Get(i);

		Ptr<MecE2ECheckerHelper> e2eHelper =
				CreateObject<MecE2ECheckerHelper>();

		e2eHelper->Install(node, allNodes, start, stop);

		MecE2EChecker::m_rtt.insert(
				std::pair<uint32_t, Ptr<MecE2ECheckerHelper>>(node->GetId(),
						e2eHelper));
	}
}

} /* namespace ns3 */
