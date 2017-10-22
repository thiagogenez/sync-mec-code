/*
 * mec-middle-box.cc
 *
 *  Created on: 7 Jun 2017
 *      Author: thiagogenez
 */

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/object-base.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"
#include "ns3/data-rate.h"
#include "ns3/string.h"

#include "mec-middle-box.h"
#include "ns3/mec-utils-helper.h"

namespace ns3 {

//Id initialisation
uint32_t MecMiddleBox::m_counter = 0;

NS_LOG_COMPONENT_DEFINE("MecMiddleBox");
NS_OBJECT_ENSURE_REGISTERED(MecMiddleBox);

MecMiddleBox::MecMiddleBox() {
	NS_LOG_FUNCTION(this);
	m_id = m_counter;
	MecMiddleBox::m_counter++;
}

MecMiddleBox::~MecMiddleBox() {
	NS_LOG_FUNCTION(this);
}

TypeId MecMiddleBox::GetTypeId() {
	static TypeId tid =
			TypeId("ns3::MecMiddleBox").SetParent<Object>().AddConstructor<
					MecMiddleBox>();
	return tid;
}

void MecMiddleBox::AttachMiddleBox(Ptr<Node> node) {
	m_nodeIdAttached = node->GetId();

	int num = node->GetNDevices();

	NS_ASSERT_MSG(num > 0,
			"Attaching MB ERROR, MB id:" << m_id << ", NO device on node:" << m_nodeIdAttached);

	Ipv4Address Addr = MecUtilsHelper::GetIpv4Address(node, 1, 0);
	m_address = Addr;

}

//assign MB to flow, reduce the capacity
void MecMiddleBox::FlowAssign(Ptr<MecFlow> flow) {
	m_capacityRemain -= flow->GetDataBitRate();
}

//release MB from a flow, resume the capacity
void MecMiddleBox::FlowRelease(Ptr<MecFlow> flow) {
	m_capacityRemain += flow->GetDataBitRate();
}

//check whether the MB has enough capacity to acceptable a flow
bool MecMiddleBox::IsFlowAcceptable(Ptr<MecFlow> flow) {
	if (m_capacityRemain - flow->GetDataBitRate() >= 0)
		return true;
	return false;
}

void MecMiddleBox::SetCapacity(std::string dataRate) {
	DataRate rate = DataRate(dataRate);
	m_capacity = rate.GetBitRate() / 1000;		//kbps
	m_capacityRemain = rate.GetBitRate() / 1000;

}

void MecMiddleBox::SetType(uint16_t i) {
	m_middleboxType = i;
}

uint32_t MecMiddleBox::GetId() const {
	return m_id;
}

uint32_t MecMiddleBox::GetAttachedNodeId() const {
	return m_nodeIdAttached;
}

uint16_t MecMiddleBox::GetMiddleBoxType() const {
	return m_middleboxType;
}

Ipv4Address MecMiddleBox::GetAddress() const {
	return m_address;
}

int64_t MecMiddleBox::GetCapacityRemain() const {
	return m_capacityRemain;
}

void MecMiddleBox::Print(std::ostream& os) const {
	os << "MB id=" << GetId() << " : type=" << GetMiddleBoxType() << ", Address="
			<< GetAddress() << " (Node's ID attached=" << GetAttachedNodeId()
			<< ")" << "\n";
}

} /* namespace ns3 */
