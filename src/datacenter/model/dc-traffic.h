/*
 * dc-traffic.h
 *
 *  Created on: Mar 16, 2012
 *      Author: poscotso
 */


#ifndef DC_TRAFFIC_H_
#define DC_TRAFFIC_H_

#include "ns3/bulk-send-application.h"
#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <queue>

#include "ns3/dc-vm-hypervisor.h"


namespace ns3 {

class Address;
class RandomVariable;
class Socket;

typedef struct SocketList{
	Ptr<Socket>		socket;
	bool			connected;
	uint32_t		maxBytes;
	uint32_t		totalBytes;
	int64_t		createTime;
}SocketList;

typedef std::vector<SocketList>::iterator socketListI;

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

class DCTrafficApplication : public Application
{
public:
  static TypeId GetTypeId (void);

  DCTrafficApplication ();

  virtual ~DCTrafficApplication ();

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
  void SetMaxBytes (uint32_t maxBytes);

  /**
   * \return pointer to associated socket
   */
  Ptr<Socket> GetSocket (void) const;

protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  void SendData (Ptr<Socket> socket);

  //Ptr<Socket>     m_socket;       // Associated socket
  Address         m_peer;         // Peer address
  //bool            m_connected;    // True if connected
  uint32_t        m_sendSize;     // Size of data to send each time
  uint32_t        m_maxBytes;     // Limit total number of bytes sent
  //uint32_t        m_totBytes;     // Total bytes sent so far
  TypeId          m_tid;
  TracedCallback<Ptr<const Packet> > m_txTrace;

private:
  void ConnectionSucceeded (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);
  void DataSend (Ptr<Socket>, uint32_t); // for socket's SetSendCallback
  void Ignore (Ptr<Socket> socket);


  /** 80% in the same rack, 20% out of the rack*/
  uint32_t GetDestination();

  /** 99%, <100MB, 1%, 100-1000MB*/
  uint32_t GetFlowSize();

  /** 60%, 10; 5%, 80-100; 35%, 0-9*/
  uint32_t GetConcurrentFlow();

  int ProbabilityGenerator (int probability);
  void ConstructNodeList();
  void GenFlowTime();
  void ScheduleAFlow();

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
  uint32_t m_timeScale;
  uint32_t m_uniFlowSize;

  uint32_t m_bigFlowCount;

  std::queue<double> m_nextFlowTime;

  FILE * m_logFlowTime;

public:
  static unsigned long m_flowsGenerated;
  static FILE * m_flowCompletion;
  //static unsigned long m_bytesGenerated;

	//add by cuilin
	Ptr<DcVmHypervisorApplication> getHyperVisor(uint32_t i);
	Ptr<DcVmHypervisorApplication> getHyperVisor();
	//Ptr<DcVmHypervisorApplication> hyperVisor;

	uint32_t srcVM;
	uint32_t dstVM;
	
};

} // namespace ns3

#endif /* DC_TRAFFIC_H_ */
