/*
 * mec-policy.h
 *
 *  Created on: 13 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_POLICY_H_
#define MEC_POLICY_H_

#include "ns3/object.h"
#include "ns3/mec-middle-box.h"
#include "ns3/mec-middle-box-container.h"

namespace ns3 {

class MiddleBoxContainer;
class MecMiddleBox;

class MecPolicy: public Object {
public:

	static TypeId GetTypeId();

	MecPolicy();
	virtual ~MecPolicy();

	bool IsIsOk() const;
	void SetIsOk(bool isOk);

	uint32_t GetDstAddr() const;
	void SetDstAddr(uint32_t dstAddr);

	uint16_t GetDstPort() const;
	void SetDstPort(uint16_t dstPort);

	uint32_t GetId() const;

	uint8_t GetProtocol() const;
	void SetProtocol(uint8_t protocol);

	uint32_t GetSrcAddr() const;
	void SetSrcAddr(uint32_t srcAddr);

	uint16_t GetSrcPort() const;
	void SetSrcPort(uint16_t srcPort);

	uint8_t GetLen() const;
	void SetLen(uint8_t len);

	uint16_t GetMiddleBoxType(uint32_t index) const;
	void AddMiddleBoxType(uint16_t type);

	Ptr<MecMiddleBox> GetMiddleBoxByIndex(uint32_t index) const;

	// used for reallocation
	void SetMiddleBox(uint32_t index, Ptr<MecMiddleBox> middlebox);

	//used to create policy
	void AddMiddlebox(Ptr<MecMiddleBox> middlebox);

	bool Match(uint32_t srcAddr, uint32_t dstAddr, uint8_t protocol,
			uint16_t srcPort, uint16_t dstPort);

	bool Match(Ptr<MecPolicy> otherPolicy);
	bool ContainsMiddlebox(Ptr<MecMiddleBox> middlebox);
	void Print(std::ostream& os);

	void Set(MiddleBoxContainer container);
	void Clear();

	uint32_t GetMiddleBoxListSize() const;

private:
	static uint32_t m_counter;
	uint32_t m_id;
	uint32_t m_srcAddr = 0;
	uint32_t m_dstAddr = 0;
	uint8_t m_protocol = 0;
	uint16_t m_srcPort = 0;
	uint16_t m_dstPort = 0;

	uint8_t m_len = 0;
	std::vector<uint16_t> m_seq;
	MecMiddleBoxContainer m_mbList;

	bool m_isOK = false;
};

} /* namespace ns3 */

#endif /* MEC_POLICY_H_ */
