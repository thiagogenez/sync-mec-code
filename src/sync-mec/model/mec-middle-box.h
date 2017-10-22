/*
 * mec-middle-box.h
 *
 *  Created on: 7 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_MIDDLE_BOX_H_
#define MEC_MIDDLE_BOX_H_

#include "ns3/object.h"

#include "ns3/mec-flow.h"

namespace ns3 {

class MecMiddleBox: public Object {
public:

	static TypeId GetTypeId();

	MecMiddleBox();
	virtual ~MecMiddleBox();

	//methods
	void AttachMiddleBox(Ptr<Node> node);
	void FlowAssign(Ptr<MecFlow> flow);
	void FlowRelease(Ptr<MecFlow> flow);
	bool IsFlowAcceptable(Ptr<MecFlow> flow);

	//setters
	void SetType(uint16_t i);
	void SetCapacity(std::string dataRate);

	//getters
	uint32_t GetAttachedNodeId() const;
	uint32_t GetId() const;
	uint16_t GetMiddleBoxType() const;
	Ipv4Address GetAddress() const;
	int64_t GetCapacityRemain() const;

	void Print(std::ostream& os) const;

private:
	static uint32_t m_counter;
	uint32_t m_id = 0;	// the ID of this MB
	uint32_t m_nodeIdAttached = 0;	// the ID of the node attached to this MB
	Ipv4Address m_address;		// the IP address that this MB is attached to

	uint64_t m_capacity = 0;	//kbps, currently use the sum of flows datarate
	int64_t m_capacityRemain = 0; 		//available capacity

	uint16_t m_middleboxType = 0;

};

} /* namespace ns3 */

#endif /* MEC_MIDDLE_BOX_H_ */
