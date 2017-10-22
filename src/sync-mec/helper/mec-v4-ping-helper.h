/*
 * mec-latency-monitor-helper.h
 *
 *  Created on: 10 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_LATENCY_MONITOR_HELPER_H_
#define MEC_LATENCY_MONITOR_HELPER_H_

#include <vector>

#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/object-factory.h"
#include "ns3/nstime.h"

//#include "ns3/mec-e2e-checker-helper.h"

namespace ns3 {

class MecE2ECheckerHelper;
/**
 * \ingroup v4ping
 * \brief Create a IPv5 ping application and associate it to a node
 *
 * This class creates one or multiple instances of ns3::V4Ping and associates
 * it/them to one/multiple node(s).
 */
class MecV4PingHelper {
public:
	/**
	 * Create a V4PingHelper which is used to make life easier for people wanting
	 * to use ping Applications.
	 *
	 * \param remote The address which should be pinged
	 */
	MecV4PingHelper(Ipv4Address remote);

	/**
	 * Install a Ping application on each Node in the provided NodeContainer.
	 *
	 * \param nodes The NodeContainer containing all of the nodes to get a V4Ping
	 *              application.
	 *
	 * \returns A list of Ping applications, one for each input node
	 */
	ApplicationContainer Install(NodeContainer c,
			Callback<void, uint32_t, double> pongerCallback, Time inteval);

	/**
	 * \brief Configure ping applications attribute
	 * \param name   attribute's name
	 * \param value  attribute's value
	 */
	void SetAttribute(std::string name, const AttributeValue &value);
private:
	/**
	 * \brief Do the actual application installation in the node
	 * \param node the node
	 * \returns a Smart pointer to the installed application
	 */
	Ptr<Application> InstallPriv(Ptr<Node> node,
			Callback<void, uint32_t, double> pongerCallback, Time inteval);
	/// Object factory
	ObjectFactory m_factory;
};

} // namespace ns3

#endif /* MEC_LATENCY_MONITOR_HELPER_H_ */
