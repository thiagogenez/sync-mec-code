/*
 * mec-utils.cpp
 *
 *  Created on: 14 Jun 2017
 *      Author: thiagogenez
 */

#include "mec-utils-helper.h"

#include "ns3/node-list.h"
#include "ns3/assert.h"
#include "ns3/ipv4.h"
#include "ns3/double.h"

#include "ns3/mec-constants-helper.h"
//#include "ns3/mec-vm-hypervisor.h"

#include <ctime>

namespace ns3 {

MecUtilsHelper::MecUtilsHelper() {
}

//MecUtilsHelper::~MecUtilsHelper() {
//}

void MecUtilsHelper::ExtractFlowNetworkNodeInformation(Ptr<MecFlow> flow,
		uint32_t *srcAddr, uint32_t *dstAddr, uint8_t* protocol,
		uint16_t *srcPort, uint16_t *dstPort) {

	Ipv4Address srcAddrIp = GetIpv4Address(flow->GetSrcNodeId(), 1, 0);
	Ipv4Address dstAddrIp = GetIpv4Address(flow->GetDstNodeId(), 1, 0);

	*srcAddr = srcAddrIp.Get();
	*dstAddr = dstAddrIp.Get();

	*protocol =
			flow->GetUseTCP() ?
					MecConstantsHelper::POLICY_TCP_PROTOCOL :
					MecConstantsHelper::POLICY_UDP_PROTOCOL;

	*srcPort = flow->GetSrcPort();
	*dstPort = flow->GetDstPort();

}

Ptr<MecVmHypervisor> MecUtilsHelper::GetHyperVisor(uint32_t nodeId) {

	Ptr<MecVmHypervisor> hypervisor;

	uint32_t totalApps = NodeList::GetNode(nodeId)->GetNApplications();

	for (uint32_t i = 0; i < totalApps; i++) {
		Ptr<Application> app = NodeList::GetNode(nodeId)->GetApplication(i);

		NS_ASSERT(app != 0);

		hypervisor = app->GetObject<MecVmHypervisor>();

		if (hypervisor != 0) {
			return hypervisor;
		}
	}

	NS_ASSERT_MSG(hypervisor != 0,
			"MecUtilsHelper::GetHyperVisor --> Can't find the MecVmHypervisor, the hypervisor returned is useless");

	return hypervisor;
}

uint32_t MecUtilsHelper::GetRandomIntegerNumber(uint32_t min, uint32_t max) {

	Ptr<UniformRandomVariable> m_uniformRand = CreateObject<
			UniformRandomVariable>();

	m_uniformRand->SetAttribute("Min", DoubleValue(min));
	m_uniformRand->SetAttribute("Max", DoubleValue(max));

	if (min <= max)
		return m_uniformRand->GetInteger(min, max);

	return m_uniformRand->GetInteger(max, min);
}

Ipv4Address MecUtilsHelper::GetIpv4Address(Ptr<Node> node, uint32_t interface,
		uint32_t addressIndex) {
	return node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
}

Ipv4Address MecUtilsHelper::GetIpv4Address(uint32_t nodeId, uint32_t interface,
		uint32_t addressIndex) {
	Ptr<Node> node = MecUtilsHelper::GetNode(nodeId);
	return MecUtilsHelper::GetIpv4Address(node, interface, addressIndex);
}

Ptr<Node> MecUtilsHelper::GetNode(uint32_t nodeId) {
	return NodeList::GetNode(nodeId);
}

} /* namespace ns3 */
