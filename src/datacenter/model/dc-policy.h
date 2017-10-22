/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DC_POLICY_H
#define DC_POLICY_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/socket.h"

#include "ns3/dc-middlebox.h"


#include "ns3/dc-fat-tree-helper.h"


namespace ns3 {

#define POLICY_UDP_PROTO	17
#define POLICY_TCP_PROTO	6

typedef struct PolicyItem{

	uint16_t id;
		
	// each flow identifier
	//flowid	 fid;
	uint32_t flowID;
	uint32_t saddr;
	uint32_t daddr;
	uint8_t  proto;
	uint16_t sport;
	uint16_t dport;

	//middlebox list
	uint8_t len;
	MiddleboxType Seq[4]; //at most 4 MBs  
	MiddleBoxContainer mbList;

	bool isOK;
}PolicyItem;

typedef std::vector<PolicyItem>::iterator PolicyItemIterator;

class DcPolicy //: public Object//public Application
{
public:
	//static TypeId GetTypeId (void);

	DcPolicy ();
	virtual ~DcPolicy ();

	PolicyItemIterator Begin (void);// const;
	PolicyItemIterator End (void);// const;
	uint32_t GetN (void) const;
	PolicyItemIterator  Get (uint32_t i);// const;
	PolicyItemIterator createPolicyEntry(uint32_t saddr, uint32_t daddr, uint8_t proto, uint16_t sport, uint16_t dport);
	uint32_t generateRandomPolicy(PolicyItemIterator it, MiddleBoxContainer mbc, Ptr<DcFatTreeHelper> fattree, DcFlowContainer fc);
	bool checkPolicy(PolicyItemIterator itP);
	void SetMiddleBoxList (PolicyItemIterator item, MiddleBoxContainer mbc);
	bool isOK(PolicyItemIterator it);	
	//void Add (PolicyItem p);
	PolicyItemIterator search(uint32_t saddr, uint32_t daddr, uint8_t proto, uint16_t sport, uint16_t dport);
	PolicyItemIterator getPolicyByFlow(Ptr<DcFlowApplication> flow);
	void migratePolicy(PolicyItemIterator itPolicy, uint32_t seqIndex, Ptr<DcMiddleBox> newMB, DcFlowContainer fc);
	DcFlowContainer getMatchedFlow(PolicyItemIterator itPolicy, DcFlowContainer fc);
	void initialMiddleboxCapacity(DcFlowContainer fc);
	void printPolicy(PolicyItemIterator it);
	bool isValid(PolicyItemIterator it);

	void setPolicyStatus(uint32_t percentate, bool status);

private:
	uint16_t m_maxID; //current max ID+1 of policies, maximum 65535
  	std::vector<PolicyItem> m_policyList;

};


}

#endif /* DC_POLICY_H */

