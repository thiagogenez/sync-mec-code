/*
 * mec-policy-routing.cc
 *
 *  Created on: 18 Jun 2017
 *      Author: thiagogenez
 */
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"

#include "ns3/dc-stats-tag.h"
#include "ns3/policy-header.h"
#include "ns3/log-macros-disabled.h"

#include "ns3/mec-utils-helper.h"
#include "mec-policy-routing.h"

namespace ns3 {

TypeId MecPolicyRouting::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::MecPolicyRouting").SetParent<
			Ipv4GlobalRouting>().SetGroupName("Internet").AddConstructor<
			MecPolicyRouting>();
	return tid;
}

MecPolicyRouting::MecPolicyRouting() {
	m_policyEnable = false;
}

//MecPolicyRouting::~MecPolicyRouting() {
//}

void MecPolicyRouting::SetIpv4(Ptr<Ipv4> ipv4) {

	//NS_LOG_FUNCTION(this << ipv4);
	NS_ASSERT(m_ipv4 == 0 && ipv4 != 0);
	m_ipv4 = ipv4;

	Ipv4GlobalRouting::SetIpv4(m_ipv4);
}

bool MecPolicyRouting::RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
		Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
		MulticastForwardCallback mcb, LocalDeliverCallback lcb,
		ErrorCallback ecb) {

	if (m_policyEnable
			&& (header.GetProtocol() == 6 || header.GetProtocol() == 17)) {

		Ptr<Packet> modifiedPacket = p->Copy();
		Ptr<Packet> backupPacket = p->Copy();
		Ptr<Node> currentNode = m_ipv4->GetObject<Node>();

		//Only works on Edge/Aggr/Core
		if (!m_backHaulCoreHelper->IsHost(currentNode->GetId())) {

			SetStatsTag(modifiedPacket);
			SetStatsTag(backupPacket);

			//declare header
			Ipv4Header ipv4Header = header;
			PolicyHeader policyHeader;
			UdpHeader udpHeader;
			TcpHeader tcpHeader;

			Ptr<MecPolicy> policy;
			bool hasPolicyHeader = false;
			bool isPolicyOK = false;
			bool isPolicyComplete = false;

			uint32_t srcAddr = header.GetSource().Get();
			uint32_t dstAddr = header.GetDestination().Get();
			uint8_t protocol = header.GetProtocol();
			uint16_t srcPort = 0;
			uint16_t dstPort = 0;

			// UDP packet
			if (header.GetProtocol() == 17) {

				modifiedPacket->RemoveHeader(udpHeader);
				srcPort = udpHeader.GetSourcePort();
				dstPort = udpHeader.GetDestinationPort();
			}
			// TCP packet
			else if (header.GetProtocol() == 6) {

				modifiedPacket->RemoveHeader(tcpHeader);
				srcPort = tcpHeader.GetSourcePort();
				dstPort = tcpHeader.GetDestinationPort();
			}

			//			std::cout << "modifiedPacket->GetSize()="
			//					<< modifiedPacket->GetSize() << std::endl;

			if (modifiedPacket->GetSize() >= policyHeader.GetSerializedSize()) {

				modifiedPacket->PeekHeader(policyHeader);

				//If there is policy header
				if (policyHeader.IsPolicyHeader()) {

					//policy complete
					if (policyHeader.GetIndex() == 255) {

						if (!hasPolicyHeader || !isPolicyOK
								|| isPolicyComplete) {

							return Ipv4GlobalRouting::RouteInput(backupPacket,
									header, idev, ucb, mcb, lcb, ecb);
						}

					}

					else {

						modifiedPacket->RemoveHeader(policyHeader);
						hasPolicyHeader = true;

						policy = m_policyContainer->GetPolicyById(
								policyHeader.GetPolicyID());

						isPolicyOK = false;

						if (m_policyContainer->IsValidPolicy(policy)
								&& policy->IsIsOk()) {
							isPolicyOK = true;
						}
					}
				}
			}

			//if at Edge, check the policy database
			if (!hasPolicyHeader
					&& m_backHaulCoreHelper->IsEdge(currentNode->GetId())) {



				policy = m_policyContainer->Search(srcAddr, dstAddr, protocol,
						srcPort, dstPort);


				//matched a valid policy, add the policy header and then routed to next MB
				if (m_policyContainer->IsValidPolicy(policy)
						&& policy->IsIsOk()) {

					policyHeader.SetIndex(0);
					policyHeader.SetPolicyID((uint16_t) policy->GetId());
					ipv4Header.SetPayloadSize(
							ipv4Header.GetPayloadSize()
									+ policyHeader.GetSerializedSize());

					hasPolicyHeader = true;
					isPolicyOK = true;

				}

			}


			// no policy header, or there is an invalid policy header, or the policy header indicates policy complete
			if (!hasPolicyHeader || !isPolicyOK || isPolicyComplete) {


				return Ipv4GlobalRouting::RouteInput(backupPacket, header, idev,
						ucb, mcb, lcb, ecb);	 //use packetCopy for DcStatsTag

			}

			//===========================
			//There is a valid policy header

			uint8_t index = policyHeader.GetIndex();
			NS_ASSERT_MSG(index < policy->GetMiddleBoxListSize(),
					"ERROR: not a valid index in PolicyHeader: index=" <<index<<", total="<< policy->GetMiddleBoxListSize());

			Ptr<MecMiddleBox> middlebox = policy->GetMiddleBoxByIndex(index);

			//			std::cout << "Middlebox:" << std::endl;
			//			middlebox->Print(std::cout);

			isPolicyComplete = false;

			//reach a MB
			if (middlebox->GetAttachedNodeId() == currentNode->GetId()) {

				//				std::cout
				//						<< "middlebox->GetAttachedNodeId() == currentNode->GetId()"
				//						<< std::endl;

				//there is a Next MB in the list
				if (index + 1 < (uint8_t) policy->GetMiddleBoxListSize()) {
					middlebox = policy->GetMiddleBoxByIndex(index + 1);

					//					std::cout << "Next Middlebox:" << std::endl;
					//					middlebox->Print(std::cout);

					policyHeader.SetIndex(index + 1);
				}

				//NO more MB, this is the last one
				else {
					isPolicyComplete = true;
				}
				//
				//				std::cout << "isPolicyComplete=" << isPolicyComplete
				//						<< std::endl;
			}

			//policy complete, no more policy header, use normal routing
			if (isPolicyComplete) {

				//complete
				policyHeader.SetIndex(255);
				modifiedPacket->AddHeader(policyHeader);

				// UDP packet
				if (header.GetProtocol() == 17) {
					udpHeader.EnableChecksums();
					modifiedPacket->AddHeader(udpHeader);
				}

				// TCP packet
				else if (header.GetProtocol() == 6) {
					tcpHeader.EnableChecksums();
					modifiedPacket->AddHeader(tcpHeader);
				}

				ipv4Header.SetPayloadSize(
						ipv4Header.GetPayloadSize()
								- policyHeader.GetSerializedSize());

				return Ipv4GlobalRouting::RouteInput(modifiedPacket, ipv4Header,
						idev, ucb, mcb, lcb, ecb);
			}

			modifiedPacket->AddHeader(policyHeader);
			ipv4Header.SetDestination(middlebox->GetAddress());

			// UDP packet
			if (header.GetProtocol() == 17) {
				udpHeader.EnableChecksums();
				modifiedPacket->AddHeader(udpHeader);
			}

			// TCP packet
			else if (header.GetProtocol() == 6) {
				tcpHeader.EnableChecksums();
				modifiedPacket->AddHeader(tcpHeader);
			}

			if (ipv4Header.GetDestination() == header.GetDestination()) {
				std::cout
						<< "ipv4Header.GetDestination() == header.GetDestination()"
						<< std::endl;
			}

			Ptr<Ipv4Route> rtentry = LookupGlobal(ipv4Header.GetDestination());
			ipv4Header.SetDestination(header.GetDestination());
			ucb(rtentry, modifiedPacket, ipv4Header);
			return true;
		}
	}

	return Ipv4GlobalRouting::RouteInput(p, header, idev, ucb, mcb, lcb, ecb);
}

Ptr<Ipv4Route> MecPolicyRouting::RouteOutput(Ptr<Packet> p,
		const Ipv4Header &header, Ptr<NetDevice> oif,
		Socket::SocketErrno &sockerr) {

	Ptr<Ipv4Route> rtentry;

	if (m_policyEnable
			&& (header.GetProtocol() == 6 || header.GetProtocol() == 17)) {

		Ptr<Node> currentNode = m_ipv4->GetObject<Node>();

		//only do for packets from host
		if (m_backHaulCoreHelper->IsHost(currentNode->GetId())) {

			//Check if destinatin is local
			Ipv4Address currentNodeAddr =
			//	MecUtilsHelper::GetIpv4Address(currentNode->GetId(), 1, 0);
					currentNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

			uint32_t srcAddr = currentNodeAddr.Get();

			//only when the dest is local, forward packt to edge
			if (srcAddr == header.GetDestination().Get()) {

				Ipv4Header ipv4Header = header;
				Ptr<Node> destNode = m_backHaulCoreHelper->CoreNodes().Get(0);
				ipv4Header.SetDestination(
						destNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()); //set another node, for getting a route out current node

				rtentry = Ipv4GlobalRouting::RouteOutput(p, ipv4Header, oif,
						sockerr);

				//				std::cout << "new header.GetDestination()="
				//						<< header.GetDestination() << ", ipv4Header="
				//						<< ipv4Header.GetDestination() << std::endl;

				return rtentry;
			}

		}
	}

	rtentry = Ipv4GlobalRouting::RouteOutput(p, header, oif, sockerr);

	return rtentry;
}

void MecPolicyRouting::SetStatsTag(Ptr<Packet> packetCopy) {

	//set dcStatsTag
	DcStatsTag statsTag;
	packetCopy->RemovePacketTag(statsTag);

	statsTag.increaseNumForward();
	packetCopy->AddPacketTag(statsTag);

}

void MecPolicyRouting::SetMecBackHaulCoreHelper(
		const Ptr<MecBackhaulCoreHelper>& mecBackHaulCoreHelper) {
	m_backHaulCoreHelper = mecBackHaulCoreHelper;
}

void MecPolicyRouting::SetMecPolicyContainer(
		MecPolicyContainer* mecPolicyContainer) {
	m_policyContainer = mecPolicyContainer;
}

bool MecPolicyRouting::IsPolicyEnable() const {
	return m_policyEnable;
}

void MecPolicyRouting::SetPolicyEnable(bool policyEnable) {
	m_policyEnable = policyEnable;
}

} /* namespace ns3 */
