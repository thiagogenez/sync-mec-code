/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Geoge Riley <riley@ece.gatech.edu>
 * Adapted from OnOffHelper by:
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef DC_TRAFFIC_HELPER_H
#define DC_TRAFFIC_HELPER_H

#include <stdint.h>
#include <string>
#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"

//#include <iostream>
#include <fstream>
//#include <string>
//#include <sys/time.h>

//#include "ns3/core-module.h"
//#include "ns3/network-module.h"

#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-routing-protocol.h"

#include "ns3/packet-sink-helper.h"

#include "ns3/bulk-send-helper.h"



namespace ns3 {


/**
 * \brief A helper to make it easier to instantiate an ns3::BulkSendApplication
 * on a set of nodes.
 */
class DCTrafficHelper
{
public:
  /**
   * Create an BulkSendHelper to make it easier to work with BulkSendApplications
   *
   * \param protocol the name of the protocol to use to send traffic
   *        by the applications. This string identifies the socket
   *        factory type used to create sockets for the applications.
   *        A typical value would be ns3::UdpSocketFactory.
   * \param address the address of the remote node to send traffic
   *        to.
   */
  DCTrafficHelper (std::string protocol, Address address);

  /**
   * Helper function used to set the underlying application attributes, 
   * _not_ the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Install an ns3::BulkSendApplication on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which an BulkSendApplication
   * will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install (NodeContainer c) const;

  /**
   * Install an ns3::BulkSendApplication on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an BulkSendApplication will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Install an ns3::BulkSendApplication on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param nodeName The node on which an BulkSendApplication will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install (std::string nodeName) const;

private:
  /**
   * \internal
   * Install an ns3::BulkSendApplication on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an BulkSendApplication will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  std::string m_protocol;
  Address m_remote;
  ObjectFactory m_factory;
};


class DcTraffic{
public:
	DcTraffic()
	{
		//PrintProgress();
	};
	~DcTraffic()
	{
		//PrintProgress();
	};
	
	void FlowPair(NodeContainer &c, int src, int dst, int iface, int rate, int start, int stop, int port)
	{
		std::ostringstream oss;
		oss<<rate<<"Mbps";

		//uint16_t port = 9;
		Ptr<Ipv4> ipv4node = c.Get(dst)->GetObject<Ipv4>();

		//BulkSendHelper  DCTrafficHelper
		DCTrafficHelper source ("ns3::TcpSocketFactory",
		                         InetSocketAddress (ipv4node->GetAddress(1,0).GetLocal(), port));
		  ApplicationContainer apps = source.Install (c.Get (src));
		  apps.Start (Seconds (start));
		  apps.Stop (Seconds (stop));

	};
	
	void CreateSink(NodeContainer &c, int start, int stop, int port)
	{
		for(uint32_t i=2; i<c.GetN(); i++)
		{
			// Create a packet sink to receive these packets
			PacketSinkHelper sink ("ns3::TcpSocketFactory",
								 InetSocketAddress (Ipv4Address::GetAny (), port));
			ApplicationContainer apps;
			apps = sink.Install (c.Get (i));
			apps.Start (Seconds (start));
			apps.Stop (Seconds (stop));
		}
	};
	
	/**query ip address assigned to each interface of a node.*/
	void PrintAllIps(NodeContainer &c)
	{
		Ptr<Node> node;// = c.Get (1); // Get pointer to ith node in container
		Ptr<Ipv4> ipv4node;// = node->GetObject<Ipv4> (); // Get Ipv4 instance of the node
		Ptr<Ipv4RoutingProtocol> ipv4routing;
	
		int ifc=0;
		for(int j=0; j<(int)c.GetN(); j++)
		{
			node = c.Get(j);
			ipv4node = node->GetObject<Ipv4>();
	
			ifc=ipv4node->GetNInterfaces();
			std::cout<<"\n"<<j<<"-th node has "<<ifc<<"interfaces"<<std::endl;
			for(int i=1; i<ifc; i++)
				std::cout<<"  The "<<i<<"-th address is "<<ipv4node->GetAddress(i,0).GetLocal()<<std::endl;
		}
	
		getchar();
	};

	void PrintProgress()
	{
		//std::cout<<"Simulator Time (ms): "<<Simulator::Now().GetMilliSeconds()<<"\n";
		//Simulator::Schedule (MilliSeconds (10), &DcTraffic::PrintProgress, this);
	};

};


} // namespace ns3

#endif /* ON_OFF_HELPER_H */

