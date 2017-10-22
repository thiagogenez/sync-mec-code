#ifndef DC_FLOW_H_
#define DC_FLOW_H_

//#include "ns3/bulk-send-application.h"
#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
//#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <queue>

#include "ns3/data-rate.h"

//#include "ns3/dc-vm-hypervisor.h"

#include "ns3/random-variable-stream.h"

namespace ns3 {

//class Address;
//class RandomVariable;
//class Socket;

/*
 typedef struct SocketList{
 Ptr<Socket>		socket;
 bool			connected;
 uint32_t		maxBytes;
 uint32_t		totalBytes;
 int64_t		createTime;
 }SocketList;

 typedef std::vector<SocketList>::iterator socketListI;
 */

/**
 * \ingroup applications
 * \defgroup bulksend DCTrafficApplication
 *
 * This traffic generator simply sends data
 * as fast as possible up to MaxBytes or until
 * the appplication is stopped if MaxBytes is
 * zero. Once the lower layer send buffer is
 * filled, it waits until space is free to
 * send more data, essentially keeping a
 * constant flow of data. Only SOCK_STREAM
 * and SOCK_SEQPACKET sockets are supported.
 * For example, TCP sockets can be used, but
 * UDP sockets can not be used.
 */

#define POLICY_UDP_PROTO	17
#define POLICY_TCP_PROTO	6

class DcFlowApplication: public Application {
public:
	static TypeId GetTypeId(void);

	DcFlowApplication();

	virtual ~DcFlowApplication();

	/**
	 * \param maxBytes the upper bound of bytes to send
	 *
	 * Set the upper bound for the total number of bytes to send. Once
	 * this bound is reached, no more application bytes are sent. If the
	 * application is stopped during the simulation and restarted, the
	 * total number of bytes sent is not reset; however, the maxBytes
	 * bound is still effective and the application will continue sending
	 * up to maxBytes. The value zero for maxBytes means that
	 * there is no upper bound; i.e. data is sent until the application
	 * or simulation is stopped.
	 */
	void SetMaxBytes(uint32_t maxBytes);

	/**
	 * \return pointer to associated socket
	 */
	Ptr<Socket> GetSocket(void) const;

protected:
	virtual void DoDispose(void);
private:
	// inherited from Application base class.
public:
	virtual void StartApplication(void);    // Called at time specified by Start
	virtual void StopApplication(void);     // Called at time specified by Stop

	Ptr<UniformRandomVariable> m_uniformRand;

private:

	void SendTCPData();

	Ptr<Socket> m_socket;     //       // Associated socket
	Address m_peer;         // Peer address
	bool m_connected;         //    // True if connected
	uint32_t m_sendSize;     // Size of data to send each time
	uint32_t m_maxBytes;     // Limit total number of bytes sent
	uint32_t m_totBytes;     //     // Total bytes sent so far
	TypeId m_tid;
	TracedCallback<Ptr<const Packet> > m_txTrace;

private:
	void ConnectionSucceeded(Ptr<Socket> socket);
	void ConnectionFailed(Ptr<Socket> socket);
	void DataSend(Ptr<Socket>, uint32_t); // for socket's SetSendCallback
	void Ignore(Ptr<Socket> socket);

	/** 80% in the same rack, 20% out of the rack*/
	//uint32_t GetDestination();
	/** 99%, <100MB, 1%, 100-1000MB*/
	uint32_t GetFlowSize();
	uint32_t GetUDPFlowSize();

	/** 60%, 10; 5%, 80-100; 35%, 0-9*/
	uint32_t GetConcurrentFlow();

	int ProbabilityGenerator(int probability);

	//void GenFlowTime();
	void ScheduleAFlow();

	/*
	 int m_noOfRackMachine;
	 std::vector<uint32_t> m_inRackHost;
	 std::vector<uint32_t> m_outRackHost;
	 std::vector<SocketList> m_socketList;


	 bool m_isStart;
	 bool m_isNodeListReady;
	 bool m_isLogFlowCreated;
	 uint32_t m_activeFlows;
	 uint32_t m_targetFlows;

	 uint32_t m_totalFlows;
	 */
	uint32_t m_timeScale;
	uint32_t m_uniFlowSize;

	uint32_t m_bigFlowCount;

	std::queue<double> m_nextFlowTime;

	FILE * m_logFlowTime;

public:
	static unsigned long m_flowsGenerated;
	static FILE * m_flowCompletion;

//add by cuilin
	uint32_t m_flowID;
	uint32_t m_srcVmID;
	uint32_t m_dstVmID;
	uint32_t m_srcNodeID; //might be unused
	uint32_t m_dstNodeID;
	uint16_t m_srcPort;
	uint16_t m_dstPort;

	//Structure that represents the measured metrics of an individual packet flow
	struct DcFlowStats {
		/// Contains the sum of all end-to-end delays for all received
		/// packets of the flow.
		Time delaySum; // delayCount == rxPackets

		/// Contains the sum of all end-to-end delay jitter (delay
		/// variation) values for all received packets of the flow.  Here
		/// we define _jitter_ of a packet as the delay variation
		/// relatively to the last packet of the stream,
		/// i.e. \f$Jitter\left\{P_N\right\} = \left|Delay\left\{P_N\right\} - Delay\left\{P_{N-1}\right\}\right|\f$.
		/// This definition is in accordance with the Type-P-One-way-ipdv
		/// as defined in IETF \RFC{3393}.
		Time jitterSum; // jitterCount == rxPackets - 1

		/// Contains the last measured delay of a packet
		/// It is stored to measure the packet's Jitter
		Time lastDelay;

		/// Total number of transmitted bytes for the flow
		uint64_t txBytes;
		/// Total number of received bytes for the flow
		uint64_t rxBytes;
		/// Total number of transmitted packets for the flow
		uint32_t txPackets;
		/// Total number of received packets for the flow
		uint32_t rxPackets;

		/// Total number of packets that are assumed to be lost,
		/// i.e. those that were transmitted but have not been reportedly
		/// received or forwarded for a long time.  By default, packets
		/// missing for a period of over 10 seconds are assumed to be
		/// lost, although this value can be easily configured in runtime
		uint32_t lostPackets;

		/// Contains the number of times a packet has been reportedly
		/// forwarded, summed for all received packets in the flow
		uint32_t timesForwarded;
	};

	DcFlowStats m_stats;

	DataRate m_dataRate;
	bool m_running;
	EventId m_sendEvent;

	bool useTCP;

	bool isOK;
	//Ptr<DcVmHypervisorApplication> hyperVisor;

	void printFlow(std::ostream& os);

	//For TCP
	void SendTCPDataNew();
	void ScheduleTxTCP();
	void ConnectionSucceededNew(Ptr<Socket> socket);
	Ptr<Socket> m_socketTCP;
	//Address         m_peer;
	//uint32_t        m_packetSize;
	uint32_t m_nPackets;

	//EventId         m_sendEvent;

	uint32_t m_packetsSent;

//FOR	UDP

	void SetUdpPacketSize(uint32_t size);
	//void ScheduleTransmit (Time dt);
	void SendUDP(void);
	void ScheduleTxUDP();
	//uint32_t m_count;
	//Time m_interval;
	//uint32_t m_size;
	uint32_t m_sent;  //num of packets sent
	//Address m_peerAddress;
	//uint16_t m_peerPort;	
	Ptr<Socket> m_socketUDP;
	//DataRate  m_cbrRate;      // Rate that data is generated

};

class DcFlowContainer {
public:
	/// MiddleBox container iterator
	typedef std::vector<Ptr<DcFlowApplication> >::const_iterator Iterator;

	DcFlowContainer();
	Iterator Begin(void) const;
	Iterator End(void) const;
	uint32_t GetN(void) const;
	Ptr<DcFlowApplication> Get(uint32_t i) const;
	Iterator GetFlowByID(uint32_t flowID) const;
	Iterator GetFlow(uint32_t saddr, uint32_t daddr, uint8_t proto,
			uint16_t sport, uint16_t dport) const;
	bool IsFlowExist(uint32_t flowID) const;

	void Add(Ptr<DcFlowApplication> flow);
	void Add(DcFlowContainer other);
	void Remove(Ptr<DcFlowApplication> vm);

	void Create(uint32_t n);
	void print(std::ostream& os);
private:
	std::vector<Ptr<DcFlowApplication> > m_flowList;
};

} // namespace ns3

#endif /* DC_TRAFFIC_H_ */
