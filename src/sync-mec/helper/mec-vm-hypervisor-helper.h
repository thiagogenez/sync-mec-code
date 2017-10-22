/*
 * mec-vm-hypervisor-application-helper.h
 *
 *  Created on: 7 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_VM_HYPERVISOR_APPLICATION_HELPER_H_
#define MEC_VM_HYPERVISOR_APPLICATION_HELPER_H_

#include <stdint.h>
#include <string>
#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/attribute.h"
#include "ns3/application-container.h"
#include "ns3/node-container.h"

namespace ns3 {

class MecVmHypervisorHelper {
public:
	MecVmHypervisorHelper();
	MecVmHypervisorHelper(std::string protocol, Address address);
	virtual ~MecVmHypervisorHelper();

	/**
	 * Install an ns3::BulkSendApplication on the node configured with all the
	 * attributes set with SetAttribute.
	 *
	 * \param node The node on which an BulkSendApplication will be installed.
	 * \returns Container of Ptr to the applications installed.
	 */
	ApplicationContainer Install(NodeContainer c) const;

private:
	/**
	 * \internal
	 * Install an ns3::BulkSendApplication on the node configured with all the
	 * attributes set with SetAttribute.
	 *
	 * \param node The node on which an BulkSendApplication will be installed.
	 * \returns Ptr to the application installed.
	 */
	Ptr<Application> InstallPriv(Ptr<Node> node) const;

	std::string m_protocol;
	Address m_remote;
	ObjectFactory m_factory;
};

} /* namespace ns3 */

#endif /* SRC_SYNC_MEC_HELPER_MEC_VM_HYPERVISOR_APPLICATION_HELPER_H_ */
