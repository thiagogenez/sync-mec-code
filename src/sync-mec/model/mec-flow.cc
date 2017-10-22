/*
 * mec-flow.cpp
 *
 *  Created on: 9 Jun 2017
 *      Author: thiagogenez
 */

#include <sstream>      // std::ostringstream

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/seq-ts-header.h"
#include "ns3/type-id.h"
#include "ns3/boolean.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/ipv4-address.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/inet-socket-address.h"

#include "mec-flow.h"
#include "ns3/mec-utils-helper.h"
#include "ns3/mec-constants-helper.h"
#include "ns3/mec-logger.h"

#include "ns3/dc-stats-tag.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("MecFlow");
NS_OBJECT_ENSURE_REGISTERED(MecFlow);

//Id initialisation
uint32_t MecFlow::m_counter = 0;

TypeId MecFlow::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::MecFlow").SetParent<Application>().AddConstructor<
					MecFlow>().AddAttribute("SendSize",
					"The amount of data to send each time.",
					UintegerValue(MecConstantsHelper::FLOW_SEND_SIZE_DEFAULT),
					MakeUintegerAccessor(&MecFlow::m_packetSize),
					MakeUintegerChecker<uint32_t>(1))
			//.AddAttribute("Protocol","The type of protocol to use.",TypeIdValue(TcpSocketFactory::GetTypeId()), MakeTypeIdAccessor(&MecFlow::m_tid),
			//		MakeTypeIdChecker())
			.AddAttribute("useTCP", "TCP stream or UDP",
					BooleanValue(MecConstantsHelper::FLOW_USE_TCP),
					MakeBooleanAccessor(&MecFlow::m_useTCP),
					MakeBooleanChecker()).AddAttribute("DataRate",
					"The data rate in on state.",
					DataRateValue(
							DataRate(
									MecConstantsHelper::FLOW_DATA_RATE_DEFAULT)),
					MakeDataRateAccessor(&MecFlow::m_dataRate),
					MakeDataRateChecker()).AddAttribute("nPackets",
					"The amount of packets to sent.",
					UintegerValue(
							MecConstantsHelper::MAX_FLOW_NPACKETS_DEFAULT),
					MakeUintegerAccessor(&MecFlow::m_nPackets),
					MakeUintegerChecker<uint32_t>(1));
	return tid;
}

MecFlow::MecFlow() {
	NS_LOG_FUNCTION(this);

	m_id = MecFlow::m_counter;
	MecFlow::m_counter++;
}

MecFlow::~MecFlow() {
	NS_LOG_FUNCTION(this);
}

void MecFlow::DoDispose(void) {
	NS_LOG_FUNCTION(this);

	m_socket = 0;
	m_socketUDP = 0;
	m_socketTCP = 0;

	// chain up
	Application::DoDispose();
}

void MecFlow::SendTCPData() {
	NS_LOG_FUNCTION(this);
	NS_LOG_LOGIC("sending packet at " << Simulator::Now ());

	Ptr<Packet> packet = Create<Packet>(m_packetSize);
	int actual = m_socketTCP->Send(packet);
	if (actual > 0) {
		m_totBytes += actual;
	}

	if (m_nPackets == 0 || ++m_packetsSent < m_nPackets) {
		ScheduleTxTCP();
	}
}

void MecFlow::ScheduleTxTCP() {
	if (m_running) {
		Time tNext(
				Seconds(
						m_packetSize * 8
								/ static_cast<double>(m_dataRate.GetBitRate())));

		m_sendEvent = Simulator::Schedule(tNext, &MecFlow::SendTCPData, this);
	}
}

void MecFlow::ConnectionSucceeded(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	NS_LOG_LOGIC("MecFlow Connection succeeded");

	m_connected = true;

	SendTCPData();
}

void MecFlow::ConnectionFailed(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	NS_LOG_LOGIC("MecFlow, Connection Failed");
}

void MecFlow::SetPacketSize(uint32_t size) {
	m_packetSize = size;
}

void MecFlow::SendUDP(void) {
	NS_ASSERT(m_sendEvent.IsExpired());

	SeqTsHeader seqTs;
	//seqTs.SetSeq(m_stats.GetTxPackets());

	// 8 + 4 : the size of the seqTs header
	Ptr<Packet> p = Create<Packet>(m_packetSize - (8 + 4));
	p->AddHeader(seqTs);

	DcStatsTag statsTag;
	statsTag.SetNumForward(0);
	statsTag.SetTimestamp(Simulator::Now());
	statsTag.SetFlowID(m_id);
	p->AddPacketTag(statsTag);

	Ipv4Address ipv4addr = MecUtilsHelper::GetIpv4Address(m_dstNodeId, 1, 0);

	if ((m_socketUDP->Send(p)) >= 0) {

//		m_stats.SetTxPackets(m_stats.GetTxPackets() + 1);
//		m_stats.SetTxBytes(m_stats.GetTxBytes() + m_packetSize);
		//++m_sent;

//		NS_LOG_INFO(
//				"TraceDelay TX " << m_packetSize << " m_packetsSent=" << m_packetsSent << " nPacket=" << m_nPackets << " bytes to " << ipv4addr << " Uid: " << p->GetUid () << " Time: " << (Simulator::Now ()).GetSeconds ());

	} else {
		NS_LOG_INFO(
				"Error while sending " << m_packetSize << " bytes to "<< ipv4addr);
	}

	if (++m_packetsSent < m_nPackets) {
		ScheduleTxUDP();
	}
}

void MecFlow::ScheduleTxUDP() {
	if (m_running) {
		Time tNext(
				Seconds(
						m_packetSize * 8
								/ static_cast<double>(m_dataRate.GetBitRate())));

		EventId event = Simulator::Schedule(tNext, &MecFlow::SendUDP, this);

		m_allPacketsEvents.push_back(event);

	}
}

void MecFlow::StartApplication() {
	NS_LOG_FUNCTION(this);

	std::ostringstream msg;
	msg << "Flow id=" << GetId() << " started ";
	MecLogger::Logging(msg.str());
	msg.clear();

	m_running = true;

	Ptr<Node> currNode = MecUtilsHelper::GetNode(m_srcNodeId);
	Ipv4Address ipv4addr = MecUtilsHelper::GetIpv4Address(m_dstNodeId, 1, 0);
	InetSocketAddress remoteAddr = InetSocketAddress(ipv4addr, m_dstPort);
	InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(),
			m_srcPort);

	if (m_useTCP) {
		if (!m_socketTCP) {
			TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
			m_socketTCP = Socket::CreateSocket(currNode, tid);

			m_connected = false;

			m_socketTCP->Bind(local);
			m_socketTCP->Connect(remoteAddr);
			m_socketTCP->ShutdownRecv();
			m_socketTCP->SetConnectCallback(
					MakeCallback(&MecFlow::ConnectionSucceeded, this),
					MakeCallback(&MecFlow::ConnectionFailed, this));
		}
	}

// Otherwise, it will use UDP
	else {

		m_allPacketsEvents.clear();

		//UDP stream
		if (m_socketUDP == 0) {

			TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
			m_socketUDP = Socket::CreateSocket(currNode, tid);

			int result = m_socketUDP->Bind(local);

			if (result != 0) {
				std::cout << "ERROR: Bind to local port failed" << std::endl;
				StopApplication();
				m_socketUDP->Close();
				m_socketUDP = 0;
				return;
			}
		}

		m_socketUDP->Connect(remoteAddr);
		m_socketUDP->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
		NS_LOG_INFO(
				"UDP Traffic started on Node "<<currNode->GetId() << " at Time: " << (Simulator::Now ()).GetSeconds ());
		SendUDP();
	}
}

void MecFlow::StopApplication() {
	NS_LOG_FUNCTION(this);
	std::ostringstream msg;
	msg << "- FlowId=" << GetId() << " stopped";
	MecLogger::Logging(msg.str());
	msg.clear();

	m_running = false;

//	if (m_sendEvent.IsRunning()) {
//		Simulator::Cancel(m_sendEvent);
//
//	}

	for (uint32_t i = 0; i < m_allPacketsEvents.size(); i++) {
		EventId event = m_allPacketsEvents[i];
		Simulator::Cancel(event);

	}

	m_allPacketsEvents.clear();

	if (m_useTCP) {

		if (m_socket != 0) {
			m_socket->Close();
			m_connected = false;
		}

		if (m_socketTCP != 0) {
			m_socketTCP->Close();
			m_socketTCP = 0;
			m_connected = false;
		}

	}

	if (m_socketUDP != 0) {
		m_socketUDP->Close();
		m_socketUDP = 0;
	}

	m_packetsSent = 0;
}

uint32_t MecFlow::GetId() const {
	return m_id;
}

void MecFlow::SetSrcEntity(Ptr<MecVm> entity) {
	m_srcEntity = entity;
}

void MecFlow::SetDstEntity(Ptr<MecVm> entity) {
	m_dstEntity = entity;
}

uint16_t MecFlow::GetSrcPort() const {
	return m_srcPort;
}

uint16_t MecFlow::GetDstPort() const {
	return m_dstPort;
}

void MecFlow::SetSrcPort(uint16_t srcPort) {
	m_srcPort = srcPort;
}

void MecFlow::SetDstPort(uint16_t dstPort) {
	m_dstPort = dstPort;
}

void MecFlow::SetUseTCP(bool useTCP) {
	m_useTCP = useTCP;
}
bool MecFlow::GetUseTCP() const {
	return m_useTCP;
}

uint64_t MecFlow::GetDataBitRate() const {
	return m_dataRate.GetBitRate() / 1000; //kbps
}

void MecFlow::SetDataRate(std::string dataRate) {
	m_dataRate = DataRate(dataRate);
}

void MecFlow::Print(std::ostream& os) {
	os << "flowId=" << m_id;
	os << ", [BS=" << m_srcNodeId << ", "
			<< (m_srcEntity->IsIsUe() ? "ueId=" : "vmId=")
			<< m_srcEntity->GetId() << "]-->";
	os << "[BS=" << m_dstNodeId << ", "
			<< (m_dstEntity->IsIsUe() ? "ueId=" : "vmId=")
			<< m_dstEntity->GetId() <<"]";

	Ipv4Address srcAddr = MecUtilsHelper::GetIpv4Address(m_srcNodeId, 1, 0);
	Ipv4Address dstAddr = MecUtilsHelper::GetIpv4Address(m_dstNodeId, 1, 0);

	os << "\t" << srcAddr << ":" << m_srcPort << " --> " << dstAddr << ":"
			<< m_dstPort << "\n";
}

//std::string MecFlow::GetFlowStats() const {
//
//	std::stringstream ss;
//
//	ss << m_id << "\t" << m_stats.GetRxBytes() << "\t" << m_stats.GetRxPackets()
//			<< "\t" << m_stats.GetDelaySum().GetMilliSeconds() << "\t"
//			<< m_stats.GetJitterSum().GetMilliSeconds() << "\t"
//			<< m_stats.GetTimesForwarded() << "\t" << m_stats.GetLostPackets()
//			<< "\n";
//
//	return ss.str();
//}

uint32_t MecFlow::GetSrcEntityId() const {
	return m_srcEntity->GetId();
}

uint32_t MecFlow::GetDstEntityId() const {
	return m_dstEntity->GetId();
}

uint32_t MecFlow::GetSrcNodeId() const {
	return m_srcNodeId;
}

uint32_t MecFlow::GetDstNodeId() const {
	return m_dstNodeId;
}

void MecFlow::SetSrcNodeId(uint32_t srcNodeId) {
	m_srcNodeId = srcNodeId;
}
void MecFlow::SetDstNodeId(uint32_t dstNodeId) {
	m_dstNodeId = dstNodeId;
}

}
/* namespace ns3 */
