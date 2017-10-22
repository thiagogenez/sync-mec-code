#include "policy-routing-helper.h"
#include "ns3/policy-routing.h"

namespace ns3 {

PolicyRoutingHelper::PolicyRoutingHelper() {
	//m_agentFactory.SetTypeId ("ns3::PolicyRouting");
}

PolicyRoutingHelper::PolicyRoutingHelper(const PolicyRoutingHelper &o) {
//: m_agentFactory (o.m_agentFactory)
}

PolicyRoutingHelper* PolicyRoutingHelper::Copy(void) const {
	return new PolicyRoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol> PolicyRoutingHelper::Create(Ptr<Node> node) const {

	Ptr<GlobalRouter> globalRouter = CreateObject<GlobalRouter>();
	node->AggregateObject(globalRouter);

	//NS_LOG_LOGIC ("Adding GlobalRouting Protocol to node " << node->GetId ());
	Ptr<PolicyRouting> globalRouting = CreateObject<PolicyRouting>();
	globalRouter->SetRoutingProtocol(globalRouting);
	//node->AggregateObject (globalRouting);

	return globalRouting;
}
} // namespace ns3
