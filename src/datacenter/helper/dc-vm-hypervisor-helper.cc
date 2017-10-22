/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 * Author: cuilin
 */

//#include "ns3/inet-socket-address.h"
//#include "ns3/packet-socket-address.h"
#include "ns3/string.h"
#include "ns3/names.h"

//#include "ns3/dc-vm-hypervisor.h"
#include "dc-vm-hypervisor-helper.h"

namespace ns3 {

DcVmHypervisorApplicationHelper::DcVmHypervisorApplicationHelper(
		std::string protocol, Address address) {
	m_factory.SetTypeId("ns3::DcVmHypervisorApplication");
	m_factory.Set("Protocol", StringValue(protocol));
	m_factory.Set("Remote", AddressValue(address));
}

DcVmHypervisorApplicationHelper::DcVmHypervisorApplicationHelper() {
	m_factory.SetTypeId("ns3::DcVmHypervisorApplication");
}

void DcVmHypervisorApplicationHelper::SetAttribute(std::string name,
		const AttributeValue &value) {
	m_factory.Set(name, value);
}

ApplicationContainer DcVmHypervisorApplicationHelper::Install(
		Ptr<Node> node) const {
	return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer DcVmHypervisorApplicationHelper::Install(
		std::string nodeName) const {
	Ptr<Node> node = Names::Find<Node>(nodeName);
	return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer DcVmHypervisorApplicationHelper::Install(
		NodeContainer c) const {
	ApplicationContainer apps;
	for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
		apps.Add(InstallPriv(*i));
	}

	return apps;
}

Ptr<Application> DcVmHypervisorApplicationHelper::InstallPriv(
		Ptr<Node> node) const {
	Ptr<Application> app = m_factory.Create<DcVmHypervisorApplication>();
	node->AddApplication(app);

	return app;
}

} // namespace ns3
