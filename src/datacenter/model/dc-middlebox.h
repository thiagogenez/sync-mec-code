/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DC_MIDDLEBOX_H
#define DC_MIDDLEBOX_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/socket.h"

#include "ns3/dc-flow.h"


namespace ns3 {

enum MiddleboxType {
	MB_FIREWALL,
	MB_IDS,
	MB_LB,
	MB_MAX
};


class DcMiddleBox : public Object//public Application
{
public:
  static TypeId GetTypeId (void);

  DcMiddleBox ();//(uint16_t id, MiddleboxType type);

  virtual ~DcMiddleBox ();

  
  /**
   * \return pointer to associated socket
   */
  Ptr<Socket> GetSocket (void) const;

  // inherited from Application base class.
  //virtual void StartApplication (void);    // Called at time specified by Start
  //virtual void StopApplication (void);     // Called at time specified by Stop

  void attachMiddleBox(Ptr<Node> node);

  
	//bool getLocalNodeAddress();

	void flowAssign(Ptr<DcFlowApplication> flow);
	void flowRelease(Ptr<DcFlowApplication> flow);
	bool isFlowAcceptable(Ptr<DcFlowApplication> flow);
	
	uint32_t m_id;
	MiddleboxType m_type;
	uint32_t m_nodeID; // the node ID that this MB is attached to
	Ipv4Address m_address;

	uint64_t m_capacity;//kbps, currently use the sum of flows datarate
	int64_t m_capacityRemain; //available capacity
};

class MiddleBoxContainer
{
public:
  /// MiddleBox container iterator
  typedef std::vector< Ptr<DcMiddleBox> >::const_iterator Iterator;

  
  MiddleBoxContainer ();
  Iterator Begin (void) const;
  Iterator End (void) const;
  uint32_t GetN (void) const;
  Ptr<DcMiddleBox> Get (uint32_t i) const;
  MiddleBoxContainer getMiddleBoxByType(MiddleboxType type);

  void Add (Ptr<DcMiddleBox> mb); //do not allow to add, use create for centrallized management
  void setMB (uint32_t index, Ptr<DcMiddleBox> newMB);
  void Create (uint32_t n);

  void Remove(Ptr<DcMiddleBox> mb);
  void ClearAll();
  
  void print();
private:
  std::vector<Ptr<DcMiddleBox> > m_mbList;
};


}

#endif /* DC_MIDDLEBOX_H */

