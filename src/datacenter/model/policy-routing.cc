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

#include <queue>
#include <iomanip>

#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"

#include "policy-routing.h"
#include "ns3/policy-header.h"

#include "ns3/dc-stats-tag.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PolocyRouting");

NS_OBJECT_ENSURE_REGISTERED (PolicyRouting);

//bool PolicyRouting::g_isCacheDirty = false;

TypeId 
PolicyRouting::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PolicyRouting")
    .SetParent<Ipv4GlobalRouting> ()//.SetParent<Ipv4NixVectorRouting> ()
    .SetGroupName ("Internet")//.SetGroupName ("NixVectorRouting")
    .AddConstructor<PolicyRouting> ()
  ;
  return tid;
}

PolicyRouting::PolicyRouting ()
{
  	//Ipv4NixVectorRouting::Ipv4NixVectorRouting();
  	Ipv4GlobalRouting();
  	m_policyEnable = true;  	
  	
  	//std::cout<<"Policy Routing initialized..., m_ipv4"<<m_ipv4<<std::endl;
}

//PolicyRouting::~PolicyRouting ()
//{
//  Ipv4NixVectorRouting::~Ipv4NixVectorRouting();
//}

Ptr<Ipv4Route> 
PolicyRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
	//std::cout<<"PolicyRouting::RouteOutput"<<std::endl;
	Ptr<Ipv4Route> rtentry;

	//Ipv4Address a = header.GetDestination();
		
	if(m_policyEnable
		&& (header.GetProtocol() == 6 || header.GetProtocol() == 17)) 
	{	
		Ptr<Node> currNode = m_ipv4->GetObject<Node>();

		//std::cout<<"PolicyRouting::RouteOutput: An output data packet, on Node:"<<currNode->GetId()<<std::endl;
		
		if(fattree->IsHost(currNode->GetId())) //only do for packets from host
		{		
			//Check if destinatin is local
			Ipv4Address currentNodeAddr = currNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
			uint32_t saddr =  currentNodeAddr.Get();

			//std::cout<<"Test: saddr="<<Ipv4Address(saddr)<<", header.GetDestination()="<<header.GetDestination()<<std::endl;

			if(saddr==header.GetDestination().Get()) //only when the dest is local, forward packt to edge
			{
				Ipv4Header iph = header;	
				Ptr<Node> destNode = fattree->CoreNodes().Get(0);
				iph.SetDestination(destNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal()); //set another node, for getting a route out current node
				//rtentry = Ipv4NixVectorRouting::RouteOutput(p, iph, oif,sockerr);
				rtentry = Ipv4GlobalRouting::RouteOutput(p, iph, oif,sockerr);
  				return rtentry;
			}

		}
	}	

  //rtentry = Ipv4NixVectorRouting::RouteOutput(p, header, oif,sockerr);  
  rtentry = Ipv4GlobalRouting::RouteOutput(p, header, oif,sockerr);

  //std::cout<<"Get a route on host="<<m_ipv4->GetObject<Node>()->GetId()<<", next:"<<rtentry->GetGateway()<<std::endl;

  return rtentry;
}


/*
Ptr<Ipv4Route> 
PolicyRouting::RouteOutput_bak (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
	Ptr<Ipv4Route> rtentry;

	Ipv4Address a = header.GetDestination();
	
	//if (a.IsMulticast() || a.IsBroadcast()) {
	//	NS_LOG_LOGIC("Non-unicast destination is not supported");
	//	sockerr = Socket::ERROR_NOROUTETOHOST;
	//	return 0;
	//};

	//Ipv4Header iph;
	//Ptr<Packet> q = p->Copy();
	//q->RemoveHeader(iph);

	//Ptr<Node> currNodeTest = m_ipv4->GetObject<Node>();
	//uint32_t protoTest = (uint32_t)header.GetProtocol();
	//std::cout<<"Packet for:"<<a<<" orignated at Node:"<<m_ipv4->GetObject<Node>()->GetId()
	//	<<" protocol="<<protoTest<<", enablePolicy="<<(m_policyEnable?"true":"false")<<std::endl;
		
	//header.Print(std::cout);
		
	if(m_policyEnable
		&& (header.GetProtocol() == 6 || header.GetProtocol() == 17)) 
	{
		Ptr<Node> currNode = m_ipv4->GetObject<Node>();
		if(fattree->IsHost(currNode->GetId())) //only do for packets from host
		{	
			
			UdpHeader udph;
			TcpHeader tcph;
			PolicyHeader ph;
		
			
			Ipv4Address currentNodeAddr = currNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
			uint32_t saddr = header.GetSource().Get();
			saddr = currentNodeAddr.Get();
			
			uint32_t daddr = header.GetDestination().Get();
			uint8_t proto = header.GetProtocol();
			uint16_t sport = 0;
			uint16_t dport = 0;

			//Ptr<Packet> packet = p->Copy();

			std::cout<<"New packet"<<std::endl;
			
			if (header.GetProtocol() == 17) {// UDP packet			
				p->RemoveHeader(udph);
				sport = udph.GetSourcePort();
				dport = udph.GetDestinationPort();
				udph.Print( std::cout);
				//std::cout<<"Get UDP, "<<sport<<"-->"<<dport<<std::endl;
			} else if (header.GetProtocol() == 6) {// TCP packet			
				p->RemoveHeader(tcph);
				sport = tcph.GetSourcePort();
				dport = tcph.GetDestinationPort();
			}

			PolicyItemIterator itP = policies->search(saddr,daddr,proto,sport,dport);
			if(policies->isOK(itP)) //matched a valid policy, add policy header and routed to next MB
			{
				std::cout<<"Match a policy id="<<itP->id<<std::endl;
				ph.SetIndex(0);
				ph.SetPolicyID(itP->id);
				p->AddHeader(ph);

				//we can not modify the IP header size, keep the size when removing header.
				//header->SetPayloadSize(header->GetPayloadSize()+ph->GetSerializedSize());
				
				if (header.GetProtocol() == 17) {// UDP packet
					udph.EnableChecksums();			
					p->AddHeader(udph);

				} else if (header.GetProtocol() == 6) {	// TCP packet
					tcph.EnableChecksums();
					p->AddHeader(tcph);
				}

				Ipv4Header iph = header;
				iph.SetDestination(header.GetDestination());
				rtentry = Ipv4NixVectorRouting::RouteOutput(p, iph, oif,sockerr);
  				return rtentry;
			}
			else //if Not a policy flow, restore the packet and use normal IP routing
			{		
				if (header.GetProtocol() == 17) {// UDP packet
					udph.EnableChecksums();			
					p->AddHeader(udph);

				} else if (header.GetProtocol() == 6) {	// TCP packet
					tcph.EnableChecksums();
					p->AddHeader(tcph);
				}
			}
		}
	}


  //MUST SURE: p is original, header can be modified
  rtentry = Ipv4NixVectorRouting::RouteOutput(p, header, oif,sockerr);

  return rtentry;
}
*/

bool 
PolicyRouting::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                  UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                  LocalDeliverCallback lcb, ErrorCallback ecb)
{

  //Ipv4Address a = header.GetDestination();
  //std::cout<<"RouteInput:: Packet for:"<<a<<" at Node:"<<m_ipv4->GetObject<Node>()->GetId()<<std::endl;
  //header.Print( std::cout);

  if(m_policyEnable
  		&& (header.GetProtocol() == 6 || header.GetProtocol() == 17))
    {
       	Ptr<Packet> packetCopy = p->Copy();
       	Ptr<Packet> packetCopyOriginal = p->Copy();
		 
        Ptr<Node> currNode = m_ipv4->GetObject<Node>();
		if(!fattree->IsHost(currNode->GetId())) //Only works on Bridge/Edge/Aggr/Core
	    {
			setStatsTag(packetCopy);
			setStatsTag(packetCopyOriginal);
			
	    	Ipv4Header iph = header;
	    	PolicyHeader ph;
	    	UdpHeader udph;
			TcpHeader tcph;

	    	PolicyItemIterator itP;
	    	bool hasPolicyHeader = false;
	    	bool isPolicyOK = false;
	    	bool isPolicyComplete = false;

	    	uint32_t saddr = header.GetSource().Get();
			uint32_t daddr = header.GetDestination().Get();
			uint8_t proto = header.GetProtocol();
			uint16_t sport = 0;
			uint16_t dport = 0;

	    	if (header.GetProtocol() == 17) {// UDP packet			
				packetCopy->RemoveHeader(udph);
				sport = udph.GetSourcePort();
				dport = udph.GetDestinationPort();
				//udph.Print( std::cout);
			} else if (header.GetProtocol() == 6) {// TCP packet			
				packetCopy->RemoveHeader(tcph);
				sport = tcph.GetSourcePort();
				dport = tcph.GetDestinationPort();
			}

			if(packetCopy->GetSize()>=ph.GetSerializedSize())
			{
				packetCopy->PeekHeader(ph);
				if(ph.IsPolicyHeader()) //If there is policy header
				{
					if(ph.GetIndex()==255)//policy complete
					{
						if(!hasPolicyHeader || !isPolicyOK || isPolicyComplete)//  policyheader indicates policycomplete
				    	{
				    		//std::cout<<"Packet policy complete="<<isPolicyComplete<<", "
				    		//		<<header.GetSource()<<" --> "<<header.GetDestination()<<std::endl;
				    		//Ipv4GlobalRouting::RouteInput(p,header,idev,ucb,mcb,lcb,ecb);
				    		Ipv4GlobalRouting::RouteInput(packetCopyOriginal,header,idev,ucb,mcb,lcb,ecb);//use packetCopy for DcStatsTag
				    		return true;
				    	}
					}
					else
					{
						packetCopy->RemoveHeader(ph);
						hasPolicyHeader = true;					
						//std::cout<<"Get policyHeader in the packet"<<std::endl;

						itP = policies->Get(ph.GetPolicyID());
						if(policies->isOK(itP))
							isPolicyOK = true;
						else
							isPolicyOK = false;
					}
				}
			}

			if(!hasPolicyHeader
				&& fattree->IsEdge(currNode->GetId())) //if at Edge, check the policy database
			{
				//std::cout<<"This packet don't have policyHeader"<<std::endl;

				itP = policies->search(saddr,daddr,proto,sport,dport);
				if(policies->isOK(itP)) //matched a valid policy, add policy header and routed to next MB
				{
					//std::cout<<"Match a policy id="<<itP->id<<std::endl;
					ph.SetIndex(0);
					ph.SetPolicyID(itP->id);
					//packetCopy->AddHeader(ph);

					iph.SetPayloadSize(iph.GetPayloadSize()+ph.GetSerializedSize());

					hasPolicyHeader = true;
					isPolicyOK = true;

					/*
					if (header.GetProtocol() == 17) {// UDP packet
						udph.EnableChecksums();			
						p->AddHeader(udph);

					} else if (header.GetProtocol() == 6) {	// TCP packet
						tcph.EnableChecksums();
						p->AddHeader(tcph);
					}
					
					iph.SetDestination(header.GetDestination());
					Ipv4NixVectorRouting::RouteInput(p,header,idev,ucb,mcb,lcb,ecb);
		    			return true;
		    			*/
				}
			}

	    	if(!hasPolicyHeader || !isPolicyOK || isPolicyComplete)// no policy header, or there is an invalid policy header, or the policyheader indicates policycomplete
	    	{
	    		//std::cout<<"Packet with no payLoad="<<hasPolicyHeader<<" or invalid policy header="<<isPolicyOK
	    		//		<<", or policy complete="<<isPolicyComplete<<", "
	    		//		<<header.GetSource()<<" --> "<<header.GetDestination();

	    		// No payload and no policyheader, return to Nix Routing.
	    		//Ipv4NixVectorRouting::RouteInput(p,header,idev,ucb,mcb,lcb,ecb);
	    		//Ipv4GlobalRouting::RouteInput(p,header,idev,ucb,mcb,lcb,ecb);
	    		Ipv4GlobalRouting::RouteInput(packetCopyOriginal,header,idev,ucb,mcb,lcb,ecb);//use packetCopy for DcStatsTag
	    		return true;
	    	}

			//===========================
			//There is a valid policy header, can not return to Nix routing directly 

			uint8_t index = ph.GetIndex();
			NS_ASSERT_MSG(index<itP->mbList.GetN(),"ERROR: not a valid index in PolicyHeader: index="
													<<index<<", total="<<itP->mbList.GetN());
			Ptr<DcMiddleBox> mb = itP->mbList.Get(index);
			
			isPolicyComplete = false;
			if(mb->m_nodeID==currNode->GetId())//reach a MB
			{
				if (index+1<(uint8_t)itP->mbList.GetN()) //there is a Next MB in the list
				{
					mb = itP->mbList.Get(index+1);
					ph.SetIndex(index+1);
				}
				else //NO more MB, this is the last one
				{
					isPolicyComplete = true;
				}
			}

			if(isPolicyComplete) //policy complete, no more policy header, use normal routing
			{		
				ph.SetIndex(255); //complete
				packetCopy->AddHeader(ph);
				
				if (header.GetProtocol() == 17) {// UDP packet
					udph.EnableChecksums();			
					packetCopy->AddHeader(udph);
				} else if (header.GetProtocol() == 6) {	// TCP packet
					tcph.EnableChecksums();
					packetCopy->AddHeader(tcph);
				}
			
				iph.SetPayloadSize(iph.GetPayloadSize()-ph.GetSerializedSize());
				//Ipv4NixVectorRouting::RouteInput(packetCopy,iph,idev,ucb,mcb,lcb,ecb);
				Ipv4GlobalRouting::RouteInput(packetCopy,iph,idev,ucb,mcb,lcb,ecb);
				return true;
			}

			packetCopy->AddHeader(ph);
			iph.SetDestination(mb->m_address);

			if (header.GetProtocol() == 17) {// UDP packet
				udph.EnableChecksums();			
				packetCopy->AddHeader(udph);
			} else if (header.GetProtocol() == 6) {	// TCP packet
				tcph.EnableChecksums();
				packetCopy->AddHeader(tcph);
			}

			//std::cout<<"To find route entry for dest:"<<iph.GetDestination()<<std::endl;
			//Ptr<Ipv4Route> rtentry = GetRouteEntry (packetCopy, iph);
			Ptr<Ipv4Route> rtentry = LookupGlobal (iph.GetDestination ());;
			iph.SetDestination(header.GetDestination());			
			ucb(rtentry, packetCopy, iph);
			//std::cout<<"Reroute to MB="<<mb->m_id<<" on Node:"<<mb->m_nodeID<<", next:"<<rtentry->GetGateway()<<std::endl;
	    	return true;		
	    }
    }
    
  //Ipv4NixVectorRouting::RouteInput(p,header,idev,ucb,mcb,lcb,ecb);
  Ipv4GlobalRouting::RouteInput(p,header,idev,ucb,mcb,lcb,ecb);

  return true;
}

/*
//IMPORTANT: exactly the same as Ipv4NixVectorRouting::RouteInput, except the last ucb() call
Ptr<Ipv4Route>
PolicyRouting::GetRouteEntry (Ptr<const Packet> p, const Ipv4Header &header)
{

  CheckCacheStateAndFlush ();

  Ptr<Ipv4Route> rtentry;

  // Get the nix-vector from the packet
  Ptr<NixVector> nixVector = p->GetNixVector ();

  // If nixVector isn't in packet, something went wrong
  NS_ASSERT (nixVector);

  // Get the interface number that we go out of, by extracting
  // from the nix-vector
  if (m_totalNeighbors == 0)
  {
      m_totalNeighbors = FindTotalNeighbors ();
  }
  uint32_t numberOfBits = nixVector->BitCount (m_totalNeighbors);
  uint32_t nodeIndex = nixVector->ExtractNeighborIndex (numberOfBits);

  std::cout<<"total neighbors:"<<m_totalNeighbors<<", nodeIndex:"<<nodeIndex<<std::endl;

  rtentry = GetIpv4RouteInCache (header.GetDestination ());

  // not in cache
  if (!rtentry)
    {
      NS_LOG_LOGIC ("Ipv4Route not in cache, build: ");
      Ipv4Address gatewayIp;
      uint32_t index = FindNetDeviceForNixIndex (nodeIndex, gatewayIp);
      uint32_t interfaceIndex = (m_ipv4)->GetInterfaceForDevice (m_node->GetDevice (index));
      Ipv4InterfaceAddress ifAddr = m_ipv4->GetAddress (interfaceIndex, 0);

	  std::cout<<"index:"<<index<<", nodeIndex:"<<nodeIndex<<std::endl;

      // start filling in the Ipv4Route info
      rtentry = Create<Ipv4Route> ();
      rtentry->SetSource (ifAddr.GetLocal ());

      rtentry->SetGateway (gatewayIp);
      rtentry->SetDestination (header.GetDestination ());
      rtentry->SetOutputDevice (m_ipv4->GetNetDevice (interfaceIndex));

	  std::cout<<"Not in cache, get new rtentry, next:"<<rtentry->GetGateway()<<std::endl;

      // add rtentry to cache
      m_ipv4RouteCache.insert (Ipv4RouteMap_t::value_type (header.GetDestination (), rtentry));
    }
    
	return rtentry;
}
*/

void 
PolicyRouting::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_FUNCTION (this << ipv4);
  NS_ASSERT (m_ipv4 == 0 && ipv4 != 0);
  m_ipv4 = ipv4;

  //std::cout<<"Setting m_ipv4="<<m_ipv4<<std::endl;

  Ipv4GlobalRouting::SetIpv4(m_ipv4);
  //std::cout<<"OK Setting m_ipv4="<<m_ipv4<<std::endl;
}

void 
PolicyRouting::checkPolicyViolation()
{
	return;
}

void PolicyRouting::setStatsTag(Ptr<Packet> packetCopy)
{
	//set dcStatsTag
	DcStatsTag statsTag;
	packetCopy->RemovePacketTag(statsTag);

	//std::cout<<"At Node:"<<m_ipv4->GetObject<Node>()->GetId()<<std::endl;
	//statsTag.Print( std::cout);
	
	statsTag.increaseNumForward();
	packetCopy->AddPacketTag(statsTag);
	return;
}

} // namespace ns3
