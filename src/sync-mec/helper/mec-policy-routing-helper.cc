/*
 * mec-policy-routing-helper.cc
 *
 *  Created on: 19 Jun 2017
 *      Author: thiagogenez
 */


#include "ns3/global-router-interface.h"

#include "ns3/mec-policy-routing.h"

#include "mec-policy-routing-helper.h"

namespace ns3 {

MecPolicyRoutingHelper::MecPolicyRoutingHelper() {

}

//MecPolicyRoutingHelper::~MecPolicyRoutingHelper() {
//}

MecPolicyRoutingHelper::MecPolicyRoutingHelper(
		const MecPolicyRoutingHelper &o) {
//: m_agentFactory (o.m_agentFactory)
}

MecPolicyRoutingHelper* MecPolicyRoutingHelper::Copy(void) const {
	return new MecPolicyRoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol> MecPolicyRoutingHelper::Create(Ptr<Node> node) const {

	Ptr<GlobalRouter> globalRouter = CreateObject<GlobalRouter>();
	node->AggregateObject(globalRouter);

	Ptr<MecPolicyRouting> globalRouting = CreateObject<MecPolicyRouting>();
	globalRouter->SetRoutingProtocol(globalRouting);

	return globalRouting;
}

} /* namespace ns3 */
