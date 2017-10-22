/*
 * mec-policy.h
 *
 *  Created on: 13 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_POLICY_CONTAINER_H_
#define MEC_POLICY_CONTAINER_H_

#include <vector>
#include "ns3/object.h"

#include "ns3/mec-policy.h"
#include "ns3/mec-middle-box.h"
#include "ns3/mec-flow-container.h"

namespace ns3 {

class MecPolicyContainer {
public:

	typedef std::vector<Ptr<MecPolicy>>::const_iterator const_iterator;
	typedef std::vector<Ptr<MecPolicy>>::iterator iterator;

	MecPolicyContainer();
	virtual ~MecPolicyContainer();

	iterator Begin();
	iterator End();
	const_iterator CBegin() const;
	const_iterator CEnd() const;
	uint32_t GetSize() const;

	Ptr<MecPolicy> GetPolicyById(uint32_t i);
	Ptr<MecPolicy> GetPolicyByIndex(uint32_t i) const;
	Ptr<MecPolicy> CreatePolicyEntry(Ptr<MecFlow> flow);
	Ptr<MecPolicy> GetPolicyByFlow(Ptr<MecFlow> flow);
	Ptr<MecPolicy> Search(uint32_t srcAddr, uint32_t dstAddr, uint8_t protocol,
			uint16_t srcPort, uint16_t dstPort);

	bool IsValidPolicy(Ptr<MecPolicy> policy);
	bool Contains(Ptr<MecPolicy> policy);

	void SetPolicyStatus(uint32_t percentate, bool status);
	void Add(Ptr<MecPolicy> policy);
	void Add(MecPolicyContainer container);

	void Print(std::ostream& os) const;

private:
	std::vector<Ptr<MecPolicy>> m_policyList;

	Ptr<MecPolicy> CreatePolicyEntry(uint32_t srcAddr, uint32_t dstAddr,
			uint8_t protocol, uint16_t srcPort, uint16_t dstPort);

};

} /* namespace ns3 */

#endif /* MEC_POLICY_CONTAINER_H_ */

