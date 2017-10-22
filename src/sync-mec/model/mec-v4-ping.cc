/*
 * MecLatencyMonitor.cc
 *
 *  Created on: 9 Jul 2017
 *      Author: thiagogenez
 */

#include "mec-v4-ping.h"

#include "ns3/log.h"
#include "ns3/object-base.h"

#include "ns3/icmpv4.h"
#include "ns3/assert.h"
#include "ns3/ipv4-address.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "mec-v4-ping.h"

#include "ns3/pointer.h"
#include "ns3/callback.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MecV4Ping");

NS_OBJECT_ENSURE_REGISTERED(MecV4Ping);

TypeId MecV4Ping::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::MecV4Ping").SetParent<Application>().SetGroupName(
					"Internet-Apps").AddConstructor<MecV4Ping>().AddAttribute(
					"Remote", "The address of the machine we want to ping.",
					Ipv4AddressValue(),
					MakeIpv4AddressAccessor(&MecV4Ping::m_remote),
					MakeIpv4AddressChecker()).AddAttribute("Verbose",
					"Produce usual output.", BooleanValue(false),
					MakeBooleanAccessor(&MecV4Ping::m_verbose),
					MakeBooleanChecker()).AddAttribute("Interval",
					"Wait  interval  seconds between sending each packet.",
					TimeValue(Seconds(0.5)),
					MakeTimeAccessor(&MecV4Ping::m_interval), MakeTimeChecker()).AddAttribute(
					"Size",
					"The number of data bytes to be sent, real packet will be 8 (ICMP) + 20 (IP) bytes longer.",
					UintegerValue(56), MakeUintegerAccessor(&MecV4Ping::m_size),
					MakeUintegerChecker<uint32_t>(16)).AddTraceSource("Rtt",
					"The rtt calculated by the ping.",
					MakeTraceSourceAccessor(&MecV4Ping::m_traceRtt),
					"ns3::Time::TracedCallback")
				.AddAttribute("Pong",
					"The callback for pong.",
					CallbackValue(),
					MakeCallbackAccessor(&MecV4Ping::m_pongCallback),
					MakeCallbackChecker ())
				.AddAttribute("E2EHelper",
					"E2E helper for pong answers",
					PointerValue(CreateObject<MecE2ECheckerHelper>()),
					MakePointerAccessor(&MecV4Ping::m_e2eHelper),
					MakePointerChecker<MecE2ECheckerHelper>());
	return tid;
}

MecV4Ping::MecV4Ping() :
		m_interval(Seconds(0.5)), m_size(56), m_socket(0), m_seq(0), m_verbose(
				false), m_recv(0) {
	NS_LOG_FUNCTION(this);
}
MecV4Ping::~MecV4Ping() {
	NS_LOG_FUNCTION(this);
}

void MecV4Ping::DoDispose(void) {
	NS_LOG_FUNCTION(this);

	if (m_next.IsRunning()) {
		StopApplication();
	}

	m_socket = 0;
	Application::DoDispose();
}

uint32_t MecV4Ping::GetApplicationId(void) const {
	NS_LOG_FUNCTION(this);
	Ptr<Node> node = GetNode();
	for (uint32_t i = 0; i < node->GetNApplications(); ++i) {
		if (node->GetApplication(i) == this) {
			return i;
		}
	}
	NS_ASSERT_MSG(false, "forgot to add application to node");
	return 0; // quiet compiler
}

void MecV4Ping::Receive(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	while (m_socket->GetRxAvailable() > 0) {
		Address from;
		Ptr<Packet> p = m_socket->RecvFrom(0xffffffff, 0, from);
		NS_LOG_DEBUG("recv " << p->GetSize () << " bytes");
		NS_ASSERT(InetSocketAddress::IsMatchingType(from));
		InetSocketAddress realFrom = InetSocketAddress::ConvertFrom(from);
		NS_ASSERT(realFrom.GetPort() == 1); // protocol should be icmp.
		Ipv4Header ipv4;
		p->RemoveHeader(ipv4);
		uint32_t recvSize = p->GetSize();
		NS_ASSERT(ipv4.GetProtocol() == 1); // protocol should be icmp.
		Icmpv4Header icmp;
		p->RemoveHeader(icmp);
		if (icmp.GetType() == Icmpv4Header::ECHO_REPLY) {
			Icmpv4Echo echo;
			p->RemoveHeader(echo);
			std::map<uint16_t, Time>::iterator i = m_sent.find(
					echo.GetSequenceNumber());

			if (i != m_sent.end() && echo.GetIdentifier() == 0) {
				uint32_t * buf = new uint32_t[m_size];
				uint32_t dataSize = echo.GetDataSize();
				uint32_t nodeId;
				uint32_t appId;
				if (dataSize == m_size) {
					echo.GetData((uint8_t *) buf);
					Read32((const uint8_t *) &buf[0], nodeId);
					Read32((const uint8_t *) &buf[1], appId);

					if (nodeId == GetNode()->GetId()
							&& appId == GetApplicationId()) {
						Time sendTime = i->second;
						NS_ASSERT(Simulator::Now() >= sendTime);
						Time delta = Simulator::Now() - sendTime;

						m_sent.erase(i);
						m_avgRtt.Update(delta.GetMilliSeconds());
						m_recv++;
						m_traceRtt(delta);
						m_pongCallback( GetNode()->GetId(),
								delta.GetMilliSeconds());

						if (m_verbose) {
							std::cout << recvSize << " bytes from "
									<< realFrom.GetIpv4() << ":" << " icmp_seq="
									<< echo.GetSequenceNumber() << " ttl="
									<< (unsigned) ipv4.GetTtl() << " time="
									<< delta.GetMilliSeconds() << " ms\n";
						}
					}
				}
				delete[] buf;
			}
		}
	}
}

// Writes data to buffer in little-endian format; least significant byte
// of data is at lowest buffer address
void MecV4Ping::Write32(uint8_t *buffer, const uint32_t data) {
	NS_LOG_FUNCTION(this << buffer << data);
	buffer[0] = (data >> 0) & 0xff;
	buffer[1] = (data >> 8) & 0xff;
	buffer[2] = (data >> 16) & 0xff;
	buffer[3] = (data >> 24) & 0xff;
}

// Writes data from a little-endian formatted buffer to data
void MecV4Ping::Read32(const uint8_t *buffer, uint32_t &data) {
	NS_LOG_FUNCTION(this << buffer << data);
	data = (buffer[3] << 24) + (buffer[2] << 16) + (buffer[1] << 8) + buffer[0];
}

void MecV4Ping::Send() {
	NS_LOG_FUNCTION(this);

	NS_LOG_INFO("m_seq=" << m_seq);
	Ptr<Packet> p = Create<Packet>();
	Icmpv4Echo echo;
	echo.SetSequenceNumber(m_seq);
	m_seq++;
	echo.SetIdentifier(0);

	//
	// We must write quantities out in some form of network order.  Since there
	// isn't an htonl to work with we just follow the convention in pcap traces
	// (where any difference would show up anyway) and borrow that code.  Don't
	// be too surprised when you see that this is a little endian convention.
	//
	uint8_t* data = new uint8_t[m_size];
	for (uint32_t i = 0; i < m_size; ++i)
		data[i] = 0;
	NS_ASSERT(m_size >= 16);

	uint32_t tmp = GetNode()->GetId();
	Write32(&data[0 * sizeof(uint32_t)], tmp);

	tmp = GetApplicationId();
	Write32(&data[1 * sizeof(uint32_t)], tmp);

	Ptr<Packet> dataPacket = Create<Packet>((uint8_t *) data, m_size);
	echo.SetData(dataPacket);
	p->AddHeader(echo);
	Icmpv4Header header;
	header.SetType(Icmpv4Header::ECHO);
	header.SetCode(0);
	if (Node::ChecksumEnabled()) {
		header.EnableChecksum();
	}
	p->AddHeader(header);
	m_sent.insert(std::make_pair(m_seq - 1, Simulator::Now()));
	m_socket->Send(p, 0);
	m_next = Simulator::Schedule(m_interval, &MecV4Ping::Send, this);
	delete[] data;
}

void MecV4Ping::StartApplication(void) {
	NS_LOG_FUNCTION(this);

	m_started = Simulator::Now();
	if (m_verbose) {
		std::cout << "PING  " << m_remote << " 56(84) bytes of data.\n";
	}

	m_socket = Socket::CreateSocket(GetNode(),
			TypeId::LookupByName("ns3::Ipv4RawSocketFactory"));
	NS_ASSERT(m_socket != 0);
	m_socket->SetAttribute("Protocol", UintegerValue(1)); // icmp
	m_socket->SetRecvCallback(MakeCallback(&MecV4Ping::Receive, this));
	InetSocketAddress src = InetSocketAddress(Ipv4Address::GetAny(), 0);
	int status;
	status = m_socket->Bind(src);
	NS_ASSERT(status != -1);
	InetSocketAddress dst = InetSocketAddress(m_remote, 0);
	status = m_socket->Connect(dst);
	NS_ASSERT(status != -1);

	Send();
}
void MecV4Ping::StopApplication(void) {
	NS_LOG_FUNCTION(this);

	if (m_next.IsRunning()) {
		m_next.Cancel();
	}
	if (m_socket) {
		m_socket->Close();
	}

	//std::cout << "PING STOPED at " << Simulator::Now().GetSeconds()<< " s"<<  std::endl;

	if (m_verbose) {
		std::ostringstream os;
		os.precision(4);
		os << "--- " << m_remote << " ping statistics ---\n" << m_seq
				<< " packets transmitted, " << m_recv << " received, "
				<< ((m_seq - m_recv) * 100 / m_seq) << "% packet loss, "
				<< "time " << (Simulator::Now() - m_started).GetMilliSeconds()
				<< "ms\n";

		if (m_avgRtt.Count() > 0)
			os << "rtt min/avg/max/mdev = " << m_avgRtt.Min() << "/"
					<< m_avgRtt.Avg() << "/" << m_avgRtt.Max() << "/"
					<< m_avgRtt.Stddev() << " ms\n";
		std::cout << os.str();
	}

	if (!m_pongCallback.IsNull()) {
		m_pongCallback(GetNode()->GetId(), m_avgRtt.Avg());
	}
}

} /* namespace ns3 */
