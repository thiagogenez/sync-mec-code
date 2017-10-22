/*
 * mec-flow.h
 *
 *  Created on: 9 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_FLOW_H_
#define MEC_FLOW_H_

#include "ns3/socket.h"
#include "ns3/application.h"
//#include "ns3/mec-flow-stats.h"

#include "ns3/data-rate.h"
#include <vector>

namespace ns3 {

class MecVm;

class MecFlow: public Application {
public:

	static TypeId GetTypeId(void);

	MecFlow();
	virtual ~MecFlow();

	void Print(std::ostream& os);

	//getters
	uint32_t GetId() const;

	uint32_t GetSrcEntityId() const;
	uint32_t GetDstEntityId() const;

	uint32_t GetSrcNodeId() const;
	uint32_t GetDstNodeId() const;

	uint16_t GetSrcPort() const;
	uint16_t GetDstPort() const;

	bool GetUseTCP() const;
	uint64_t GetDataBitRate() const;
	std::string GetFlowStats() const;

	//setters
	void SetSrcEntity(Ptr<MecVm> entity);
	void SetDstEntity(Ptr<MecVm> entity);

	void SetSrcNodeId(uint32_t srcNodeId);
	void SetDstNodeId(uint32_t dstNodeId);

	void SetSrcPort(uint16_t srcPort);
	void SetDstPort(uint16_t dstPort);

	void SetUseTCP(bool useTCP);
	void SetDataRate(std::string dataRate);

	void SetPacketSize(uint32_t size);

	// Inherited from Application base class.
	// Called at time specified by Start
	virtual void StartApplication();
	// Called at time specified by Stop
	virtual void StopApplication();

private:

	// For TCP
	void SendTCPData();
	void ScheduleTxTCP();
	void ConnectionSucceeded(Ptr<Socket> socket);
	void ConnectionFailed(Ptr<Socket> socket);
	Ptr<Socket> m_socketTCP;
	uint32_t m_nPackets = 0;
	uint32_t m_packetsSent = 0;
	bool m_useTCP = false;

	//For UDP
	void SendUDP(void);
	void ScheduleTxUDP();
	Ptr<Socket> m_socketUDP;

	// general uses
	static uint32_t m_counter;
	uint32_t m_id;
	Ptr<Socket> m_socket;	// Associated socket
	DataRate m_dataRate;
	EventId m_sendEvent;

	Ptr<MecVm> m_srcEntity;
	Ptr<MecVm> m_dstEntity;

	uint32_t m_srcNodeId = 0;
	uint32_t m_dstNodeId = 0;

	uint16_t m_srcPort = 0;
	uint16_t m_dstPort = 0;

	uint32_t m_totBytes = 0;    // Total bytes sent so far
	uint32_t m_packetSize = 0;     // Size of data to send each time

	bool m_running = false;
	bool m_connected = false;       // True if connected

//	MecFlowStats m_stats;

	std::vector<EventId> m_allPacketsEvents;

protected:

	virtual void DoDispose(void);

};

} /* namespace ns3 */

#endif /* MEC_FLOW_H_ */
