/*
 * mec-vm-hypervisor-application-helper.cc
 *
 *  Created on: 7 Jun 2017
 *      Author: thiagogenez
 */

#include "mec-vm-hypervisor-helper.h"
#include "ns3/string.h"
#include "ns3/names.h"
#include "ns3/mec-vm-hypervisor.h"

namespace ns3 {

MecVmHypervisorHelper::MecVmHypervisorHelper() {
	m_factory.SetTypeId("ns3::MecVmHypervisor");
}

MecVmHypervisorHelper::MecVmHypervisorHelper(std::string protocol,
		Address address) {

	m_factory.SetTypeId("ns3::MecVmHypervisorHelper");
	m_factory.Set("Protocol", StringValue(protocol));
	m_factory.Set("Remote", AddressValue(address));
}

MecVmHypervisorHelper::~MecVmHypervisorHelper() {

}

ApplicationContainer MecVmHypervisorHelper::Install(NodeContainer c) const {

	ApplicationContainer apps;

	for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
		apps.Add(InstallPriv(*i));
	}

	return apps;
}

Ptr<Application> MecVmHypervisorHelper::InstallPriv(Ptr<Node> node) const {
	Ptr<Application> app = m_factory.Create<MecVmHypervisor>();
	node->AddApplication(app);

	return app;
}

} /* namespace ns3 */
