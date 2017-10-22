/*
 * dc-traffic.cc
 *
 *  Created on: Mar 16, 2012
 *      Author: poscotso
 */
#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/address.h"
//#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"

//#include "dc-traffic.h"

//#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

#include  <math.h>

#include "dc-flow.h"
//#include "ns3/bulk-send-application.h"
#include "ns3/seq-ts-header.h"

#include "ns3/dc-stats-tag.h"

NS_LOG_COMPONENT_DEFINE("DcFlowApplication");

namespace ns3 {

//unsigned long DcFlowApplication::m_flowsGenerated=0;
//FILE * DcFlowApplication::m_flowCompletion;
//unsigned long DcFlowApplication::m_bytesGenerated=0;

NS_OBJECT_ENSURE_REGISTERED(DcFlowApplication);

TypeId DcFlowApplication::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::DcFlowApplication").SetParent<Application>().AddConstructor<
					DcFlowApplication>().AddAttribute("SendSize",
					"The amount of data to send each time.",
					UintegerValue(1400),
					MakeUintegerAccessor(&DcFlowApplication::m_sendSize),
					MakeUintegerChecker<uint32_t>(1)).AddAttribute("Remote",
					"The address of the destination", AddressValue(),
					MakeAddressAccessor(&DcFlowApplication::m_peer),
					MakeAddressChecker()).AddAttribute("MaxBytes",
					"The total number of bytes to send. "
							"Once these bytes are sent, "
							"no data  is sent again. The value zero means "
							"that there is no limit.", UintegerValue(0),
					MakeUintegerAccessor(&DcFlowApplication::m_maxBytes),
					MakeUintegerChecker<uint32_t>()).AddAttribute("Protocol",
					"The type of protocol to use.",
					TypeIdValue(TcpSocketFactory::GetTypeId()),
					MakeTypeIdAccessor(&DcFlowApplication::m_tid),
					MakeTypeIdChecker()).AddAttribute("useTCP",
					"TCP stream or UDP", BooleanValue(false),
					MakeBooleanAccessor(&DcFlowApplication::useTCP),
					MakeBooleanChecker()).AddAttribute("DataRate",
					"The data rate in on state.",
					DataRateValue(DataRate("500kb/s")),
					MakeDataRateAccessor(&DcFlowApplication::m_dataRate),
					MakeDataRateChecker())
					//.AddTraceSource ("Tx", "A new packet is created and is sent",
					//              MakeTraceSourceAccessor (&DcFlowApplication::m_txTrace))
					;
	return tid;
}

DcFlowApplication::DcFlowApplication() :
		m_socket(0), m_connected(false), m_totBytes(0) {
	m_uniformRand = CreateObject<UniformRandomVariable>();

	useTCP = false;

	//GenFlowTime();  //TODO: ?????

	//if(m_proto==POLICY_UDP_PROTO)
	{

		//m_cbrRate = DataRate ("50kb/s");
		m_dataRate = DataRate("50kb/s");
		m_sent = 0;
		m_socketUDP = 0;
		m_socketTCP = 0;
		m_sendEvent = EventId();
		m_running = false;

		m_nPackets = 0;
		m_packetsSent = 0;

		isOK = true;
	}

	//m_stats.delaySum=0;
	//m_stats.jitterSum=0;
	//m_stats.lastDelay=0;
	m_stats.lostPackets = 0;
	m_stats.rxBytes = 0;
	m_stats.rxPackets = 0;
	m_stats.timesForwarded = 0;
	m_stats.txBytes = 0;
	m_stats.txPackets = 0;
}

DcFlowApplication::~DcFlowApplication() {
	//std::cout<<"Total Number of Flows: \t"<<m_flowsGenerated<<"\t"<<m_totalFlows<<"\n";
	//std::cout<<"Total Number of Bytes: "<<m_bytesGenerated<<"\n";
	//NS_LOG_FUNCTION (this);
}

/*
 void
 DcFlowApplication::GenFlowTime ()
 {
 double lamda=1;

 for (uint32_t i=0; i<10000; i++)
 {
 //double x = (double)RandomVariable(UniformVariable(0, 6)).GetValue();
 double x = m_uniformRand->GetValue(0.0, 6.0);

 //double nextFlowTime = lamda*exp((0-lamda)*x)*1000;
 //std::cout << "nextTime: "<<nextFlowTime<<"\n";
 m_nextFlowTime.push(lamda*exp((0-lamda)*x));
 }
 }

 void
 DcFlowApplication::ScheduleAFlow()
 {
 if(!useTCP)
 return;

 double nextFlowTime = m_nextFlowTime.front();
 //fprintf(m_logFlowTime,"%d\t%f\n", GetNode()->GetId(), nextFlowTime);

 m_nextFlowTime.pop();
 Time nextTime = Time (MilliSeconds (nextFlowTime * m_timeScale));
 Simulator::Schedule (nextTime,&DcFlowApplication::StartApplication, this);
 }
 */

void DcFlowApplication::SetMaxBytes(uint32_t maxBytes) {
	NS_LOG_FUNCTION(this << maxBytes);
	m_maxBytes = maxBytes;
}

Ptr<Socket> DcFlowApplication::GetSocket(void) const {
	//NS_LOG_FUNCTION (this);
	return m_socket;
}

void DcFlowApplication::DoDispose(void) {
	//NS_LOG_FUNCTION (this);

	m_socket = 0;
	m_socketUDP = 0;
	m_socketTCP = 0;
	// chain up
	Application::DoDispose();
}

// Application Methods
void DcFlowApplication::StartApplication(void) // Called at time specified by Start
		{
	//NS_LOG_FUNCTION (this);

	m_running = true;

	Ptr<Node> currNode = NodeList::GetNode(m_srcNodeID);

	if (useTCP) {
		if (!m_socketTCP) {
			TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
			m_socketTCP = Socket::CreateSocket(currNode, tid);

			m_connected = false;

			InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(),
					m_srcPort);
			m_socketTCP->Bind(local);

			Ipv4Address ipv4addr = NodeList::GetNode(m_dstNodeID)->GetObject<
					Ipv4>()->GetAddress(1, 0).GetLocal();
			InetSocketAddress remoteAddr = InetSocketAddress(ipv4addr,
					m_dstPort);
			m_socketTCP->Connect(remoteAddr);

			m_socketTCP->ShutdownRecv();
			m_socketTCP->SetConnectCallback(
					MakeCallback(&DcFlowApplication::ConnectionSucceededNew,
							this),
					MakeCallback(&DcFlowApplication::ConnectionFailed, this));
			//m_socket->SetSendCallback (
			//	MakeCallback (&DcFlowApplication::DataSend, this));
		}

	} else {
		Ipv4Address ipv4addr;
		//UDP stream
		if (m_socketUDP == 0) {
			TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
			m_socketUDP = Socket::CreateSocket(currNode, tid);

			//ipv4addr = NodeList::GetNode(m_srcNodeID)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
			//InetSocketAddress local = InetSocketAddress (ipv4addr, m_srcPort);
			InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(),
					m_srcPort);

			int result = m_socketUDP->Bind(local);
			if (result != 0) {
				std::cout << "ERROR: Bind to local port failed" << std::endl;
				StopApplication();
				m_socketUDP->Close();
				m_socketUDP = 0;
				//Simulator::Schedule (MilliSeconds (10), &DcFlowApplication::StartApplication, this);
				return;
			}
		}

		ipv4addr =
				NodeList::GetNode(m_dstNodeID)->GetObject<Ipv4>()->GetAddress(1,
						0).GetLocal();
		InetSocketAddress remoteAddr = InetSocketAddress(ipv4addr, m_dstPort);
		m_socketUDP->Connect(remoteAddr);

		m_socketUDP->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
		//m_sendEvent = Simulator::Schedule (Seconds (0.0), &DcFlowApplication::SendUDP, this);
		NS_LOG_INFO(
				"UDP Traffic started on Node "<<currNode->GetId() << " at Time: " << (Simulator::Now ()).GetSeconds ());
		SendUDP();
	}
}

void DcFlowApplication::StopApplication(void) // Called at time specified by Stop
		{
	//NS_LOG_FUNCTION (this);

	m_running = false;

	if (m_sendEvent.IsRunning()) {
		Simulator::Cancel(m_sendEvent);
	}

	if (useTCP) {

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
		m_socketUDP->Close(); //
		//m_socketUDP->DeBind();
		m_socketUDP = 0;
	}

	m_packetsSent = 0;
}

// Private helpers

void DcFlowApplication::SendTCPData() {
	//NS_LOG_FUNCTION (this);

	while (m_maxBytes == 0 || m_totBytes < m_maxBytes) {
		// Time to send more
		uint32_t toSend = m_sendSize;
		// Make sure we don't send too many
		if (m_maxBytes > 0) {
			toSend = std::min(m_sendSize, m_maxBytes - m_totBytes);
		}

		NS_LOG_LOGIC("sending packet at " << Simulator::Now ());
		Ptr<Packet> packet = Create<Packet>(toSend);
		m_txTrace(packet);
		int actual = m_socket->Send(packet);
		if (actual > 0) {
			m_totBytes += actual;
			//m_bytesGenerated += actual;
		}
		// We exit this loop when actual < toSend as the send side
		// buffer is full. The "DataSent" callback will pop when
		// some buffer space has freed ip.
		if ((unsigned) actual != toSend) {
			break;
		}
	}		//end while

	// Check if time to close (all sent)
	if (m_totBytes >= m_maxBytes && m_connected) {
		m_socket->Close();
		m_connected = false;
	}
}

void DcFlowApplication::SendTCPDataNew() {
	//NS_LOG_FUNCTION (this);

	NS_LOG_LOGIC("sending packet at " << Simulator::Now ());
	Ptr<Packet> packet = Create<Packet>(m_sendSize);
	//m_txTrace (packet);
	int actual = m_socketTCP->Send(packet);
	if (actual > 0) {
		m_totBytes += actual;
	}

	if (m_nPackets == 0 || ++m_packetsSent < m_nPackets) {
		ScheduleTxTCP();
	}
}

void DcFlowApplication::ScheduleTxTCP() {
	if (m_running) {
		Time tNext(
				Seconds(
						m_sendSize * 8
								/ static_cast<double>(m_dataRate.GetBitRate())));
		m_sendEvent = Simulator::Schedule(tNext,
				&DcFlowApplication::SendTCPDataNew, this);
	}
}

void DcFlowApplication::ConnectionSucceeded(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	NS_LOG_LOGIC("DcFlowApplication Connection succeeded");

	m_connected = true;

	SendTCPData();
}
void DcFlowApplication::ConnectionSucceededNew(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	NS_LOG_LOGIC("DcFlowApplication Connection succeeded");

	m_connected = true;

	SendTCPDataNew();
}

void DcFlowApplication::ConnectionFailed(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	NS_LOG_LOGIC("DcFlowApplication, Connection Failed");
}

void DcFlowApplication::DataSend(Ptr<Socket> socket, uint32_t size) {
	//NS_LOG_FUNCTION (this);

	if (m_connected) {
		//Simulator::ScheduleNow (&DcFlowApplication::SendTCPData, this);
	}
}

void DcFlowApplication::SetUdpPacketSize(uint32_t size) {
	m_sendSize = size;
}

void DcFlowApplication::SendUDP(void) {
	//NS_ASSERT (m_sendEvent.IsExpired ());

	SeqTsHeader seqTs;
	seqTs.SetSeq(m_sent);
	Ptr<Packet> p = Create<Packet>(m_sendSize - (8 + 4)); // 8+4 : the size of the seqTs header
	p->AddHeader(seqTs);

	DcStatsTag statsTag;
	statsTag.SetNumForward(0);
	statsTag.SetTimestamp(Simulator::Now());
	statsTag.SetFlowID(m_flowID);
	p->AddPacketTag(statsTag);

	Ipv4Address ipv4addr =
			NodeList::GetNode(m_dstNodeID)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

	if ((m_socketUDP->Send(p)) >= 0) {
		m_stats.txPackets++;
		m_stats.txBytes += m_sendSize;

		//++m_sent;
		NS_LOG_INFO(
				"TraceDelay TX " << m_sendSize << " bytes to " << ipv4addr << " Uid: " << p->GetUid () << " Time: " << (Simulator::Now ()).GetSeconds ());

		//std::cout<<"TraceDelay TX " << m_sendSize << " bytes to "
		//                           << ipv4addr << " Uid: "
		//                          << p->GetUid () << " Time: "
		//                          << (Simulator::Now ()).GetSeconds ()<<std::endl;
	} else {
		NS_LOG_INFO(
				"Error while sending " << m_sendSize << " bytes to "<< ipv4addr);
	}

	if (m_nPackets == 0 || ++m_packetsSent < m_nPackets) {
		//std::cout<<"Need to send more"<<std::endl;
		ScheduleTxUDP();
	}
}

void DcFlowApplication::ScheduleTxUDP() {
	if (m_running) {
		Time tNext(
				Seconds(
						m_sendSize * 8
								/ static_cast<double>(m_dataRate.GetBitRate())));

		m_sendEvent = Simulator::Schedule(tNext, &DcFlowApplication::SendUDP,
				this);
	}
}

/*
 int DcFlowApplication::ProbabilityGenerator(int probability)
 {
 return ((rand () % 1000 + 1) < probability) ? 1 : 0;
 }

 uint32_t
 DcFlowApplication::GetFlowSize()
 {
 uint32_t flowSize = 0;

 if(ProbabilityGenerator(999)) // flow size, 0-100 MB
 {
 //if(ProbabilityGenerator(60))
 flowSize = 4000;
 //else
 //flowSize = 8000;
 }
 else{
 if(m_bigFlowCount<1)
 {
 flowSize = 100000000;
 std::cout << "**** LARGE Flow size "<<flowSize<<"****\n";
 m_bigFlowCount++;
 }else
 flowSize = 4000;
 }

 //std::cout << "Flow size "<<flowSize<<"\n";

 //return (flowSize/1000);
 //return 4000; //4kB
 //return 8000; //8kB
 //return 100000; //100kB
 return m_uniFlowSize;
 //return flowSize;
 }

 uint32_t
 DcFlowApplication::GetUDPFlowSize()
 {
 uint32_t flowSize = 0;

 if(ProbabilityGenerator(990)) // flow size, 0-100 MB
 {
 if(ProbabilityGenerator(60))
 flowSize = rand () % 10000;
 else
 flowSize = rand () % 10000 + 90000;
 }
 else
 flowSize = rand () % 900000000 + 100000000;

 //std::cout << "Flow size "<<flowSize<<"\n";
 //if (flowSize < m_pktSize) return m_pktSize;

 //flowSize = (uint32_t) flowSize/1000;

 if(flowSize < m_sendSize)
 flowSize = m_sendSize;
 else
 flowSize = (flowSize/m_sendSize) * m_sendSize;

 return flowSize;///1000; //(flowSize < m_pktSize ?  m_pktSize:flowSize);
 //return 1450;
 }
 */

void DcFlowApplication::printFlow(std::ostream& os) {
	os << "Flow ID:" << m_flowID << ", [Node " << m_srcNodeID << ", VM "
			<< m_srcVmID << "] --> [Node " << m_dstNodeID << ", VM "
			<< m_dstVmID << "]" << ", Port " << m_srcPort << " --> "
			<< m_dstPort << ", " << (useTCP ? "TCP" : "UDP") << ", rate:"
			<< m_dataRate << "\n" << "Stats: txPackets=" << m_stats.txPackets
			<< ", txBytes=" << m_stats.txBytes << ", rxPackets="
			<< m_stats.rxPackets << ", rxBytes=" << m_stats.rxBytes
			<< ", delaySum=" << m_stats.delaySum << ", timesForwarded="
			<< m_stats.timesForwarded << "\n";
}

DcFlowContainer::DcFlowContainer() {
}

DcFlowContainer::Iterator DcFlowContainer::Begin(void) const {
	return m_flowList.begin();
}

DcFlowContainer::Iterator DcFlowContainer::End(void) const {
	return m_flowList.end();
}

uint32_t DcFlowContainer::GetN(void) const {
	return m_flowList.size();
}

Ptr<DcFlowApplication> DcFlowContainer::Get(uint32_t i) const {
	return m_flowList[i];
}

DcFlowContainer::Iterator DcFlowContainer::GetFlowByID(uint32_t flowID) const {
	for (DcFlowContainer::Iterator it = m_flowList.begin();
			it != m_flowList.end(); it++) {
		if ((*it)->m_flowID == flowID)
			return it;
	}
	return m_flowList.end();
}

DcFlowContainer::Iterator DcFlowContainer::GetFlow(uint32_t saddr,
		uint32_t daddr, uint8_t proto, uint16_t sport, uint16_t dport) const {
	for (DcFlowContainer::Iterator it = m_flowList.begin();
			it != m_flowList.end(); it++) {
		Ptr<DcFlowApplication> flow = *it;
		uint8_t protoFlow = flow->useTCP ? POLICY_TCP_PROTO : POLICY_UDP_PROTO;

		//do not use IP address(some error may happen due to migration)
		//use src ports to find a flow, because diff flow has diff src ports
		if (protoFlow == proto && flow->m_srcPort == sport
				&& flow->m_dstPort == dport) {
			return it;
		}

		//==================

		if (protoFlow != proto || flow->m_srcPort != sport
				|| flow->m_dstPort != dport)
			continue;

		Ptr<Node> srcNode = NodeList::GetNode(flow->m_srcNodeID);
		Ptr<Node> dstNode = NodeList::GetNode(flow->m_dstNodeID);
		Ipv4Address sAddr =
				srcNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
		Ipv4Address dAddr =
				dstNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

		if (sAddr.Get() == saddr && dAddr.Get() == daddr)
			return it;
	}
	return m_flowList.end();
}

bool DcFlowContainer::IsFlowExist(uint32_t flowID) const {
	Iterator it = GetFlowByID(flowID);
	if (it == m_flowList.end())
		return false;
	else
		return true;
}

void DcFlowContainer::Add(Ptr<DcFlowApplication> flow) {
	m_flowList.push_back(flow);
}

void DcFlowContainer::Add(DcFlowContainer other) {
	for (Iterator i = other.Begin(); i != other.End(); i++) {
		if (IsFlowExist((*i)->m_flowID))
			continue;
		m_flowList.push_back(*i);
	}
}

void DcFlowContainer::Remove(Ptr<DcFlowApplication> flow) {

	std::vector<Ptr<DcFlowApplication> >::iterator it;
	for (it = m_flowList.begin(); it != m_flowList.end(); it++) {
		if ((*it)->m_flowID == flow->m_flowID) {
			m_flowList.erase(it);
			return;
		}
	}
	std::cout << "Can not find Flow in current container" << std::endl;
}

void DcFlowContainer::Create(uint32_t n) {
	//uint16_t id = 1;
	for (uint32_t i = 0; i < n; i++) {
		Ptr<DcFlowApplication> flow = CreateObject<DcFlowApplication>();
		flow->m_flowID = i;		//id++;
		m_flowList.push_back(flow);
	}
}

void DcFlowContainer::print(std::ostream& os) {
	std::cout << "Total: " << GetN() << " Flows" << std::endl;
	for (DcFlowContainer::Iterator it = m_flowList.begin();
			it != m_flowList.end(); it++) {
		Ptr<DcFlowApplication> flow = *it;
		flow->printFlow(os);
	}
}

}		// Namespace ns3

