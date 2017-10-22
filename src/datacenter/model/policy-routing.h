/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 The Georgia Institute of Technology 
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
 * Authors: Josh Pelkey <jpelkey@gatech.edu>
 */

// For policy-aware routing, modified based on  nix-vector-routing

#ifndef POLICY_ROUTING_H
#define POLICY_ROUTING_H

#include <map>

#include "ns3/channel.h"
#include "ns3/node-container.h"
#include "ns3/node-list.h"
#include "ns3/net-device-container.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/nix-vector.h"
#include "ns3/bridge-net-device.h"

#include "ns3/ipv4-nix-vector-routing.h"

#include "ns3/dc-fat-tree-helper.h"
#include "ns3/dc-policy.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"



namespace ns3 {

/**
 * \ingroup nix-vector-routing
 * Map of Ipv4Address to NixVector
 */
//typedef std::map<Ipv4Address, Ptr<NixVector> > NixMap_t;
/**
 * \ingroup nix-vector-routing
 * Map of Ipv4Address to Ipv4Route
 */
//typedef std::map<Ipv4Address, Ptr<Ipv4Route> > Ipv4RouteMap_t;

/**
 * \ingroup nix-vector-routing
 * Nix-vector routing protocol
 */
class PolicyRouting : public Ipv4GlobalRouting//Ipv4NixVectorRouting
{
public:
  PolicyRouting ();
//  ~PolicyRouting ();
  /**
   * @brief The Interface ID of the Global Router interface.
   *
   * @see Object::GetObject ()
   */
  static TypeId GetTypeId (void);

/*  //commented by CUI Lin
  void SetNode (Ptr<Node> node);
  void FlushGlobalNixRoutingCache (void) const;

 */

virtual void SetIpv4 (Ptr<Ipv4> ipv4);


private:

  /*  //commented by CUI Lin

  void FlushNixCache (void) const;
  void FlushIpv4RouteCache (void) const;
  void ResetTotalNeighbors (void);
  Ptr<NixVector> GetNixVector (Ptr<Node>, Ipv4Address, Ptr<NetDevice>);
  Ptr<NixVector> GetNixVectorInCache (Ipv4Address);
  Ptr<Ipv4Route> GetIpv4RouteInCache (Ipv4Address);
  void GetAdjacentNetDevices (Ptr<NetDevice>, Ptr<Channel>, NetDeviceContainer &);
  Ptr<Node> GetNodeByIp (Ipv4Address);
  bool BuildNixVector (const std::vector< Ptr<Node> > & parentVector, uint32_t source, uint32_t dest, Ptr<NixVector> nixVector);
  bool BuildNixVectorLocal (Ptr<NixVector> nixVector);
  uint32_t FindTotalNeighbors (void);
  Ptr<BridgeNetDevice> NetDeviceIsBridged (Ptr<NetDevice> nd) const;
  uint32_t FindNetDeviceForNixIndex (uint32_t nodeIndex, Ipv4Address & gatewayIp);
  bool BFS (uint32_t numberOfNodes,
            Ptr<Node> source,
            Ptr<Node> dest,
            std::vector< Ptr<Node> > & parentVector,
            Ptr<NetDevice> oif);

  void DoDispose (void);

  */

  /* From Ipv4RoutingProtocol */
  virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  Ptr<Ipv4Route> RouteOutput_bak (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  virtual bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                           UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                           LocalDeliverCallback lcb, ErrorCallback ecb);

  
  Ptr<Ipv4Route> GetRouteEntry (Ptr<const Packet> p, const Ipv4Header &header);

  
  //bool PolicyAware((Ptr<Packet> packet, const Ipv4Header &header, Ptr<const NetDevice> idev, UnicastForwardCallback ucb));

  //virtual void NotifyInterfaceUp (uint32_t interface);
  //virtual void NotifyInterfaceDown (uint32_t interface);
  //virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  //virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  //virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  //virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;

 /*  //commented by CUI Lin
  void CheckCacheStateAndFlush (void) const;
  static bool g_isCacheDirty;
  mutable NixMap_t m_nixCache;
  mutable Ipv4RouteMap_t m_ipv4RouteCache;
  Ptr<Ipv4> m_ipv4;
  Ptr<Node> m_node;
  uint32_t m_totalNeighbors;
  */

  Ptr<Ipv4> m_ipv4;

  void checkPolicyViolation();
  void setStatsTag(Ptr<Packet> packetCopy);

public:
  	bool m_policyEnable;

  	Ptr<DcFatTreeHelper> fattree;
	DcPolicy *policies;
	MiddleBoxContainer *mbc;
	//PolicyLogAll * policyLog;
};
} // namespace ns3

#endif /* POLICY_ROUTING_H */
