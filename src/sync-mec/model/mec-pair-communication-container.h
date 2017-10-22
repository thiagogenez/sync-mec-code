/*
 * mec-pair-communication-container.h
 *
 *  Created on: 30 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_PAIR_COMMUNICATION_CONTAINER_H_
#define MEC_PAIR_COMMUNICATION_CONTAINER_H_

#include <vector>

#include "ns3/ptr.h"
#include "ns3/mec-pair-communication.h"

namespace ns3 {

class MecPairCommunicationContainer {
public:

	typedef std::vector<Ptr<MecPairCommunication> >::const_iterator const_iterator;

	MecPairCommunicationContainer();
	virtual ~MecPairCommunicationContainer();

	const_iterator CBegin() const;
	const_iterator CEnd() const;
	uint32_t GetSize() const;
	Ptr<MecPairCommunication> GetPairByIndex(uint32_t i) const;

	void Add(Ptr<MecPairCommunication> pair);
	void Print(std::ostream& os) const;

	bool Empty() const;

private:

	std::vector<Ptr<MecPairCommunication>> m_pairList;
};

} /* namespace ns3 */

#endif /* MEC_PAIR_COMMUNICATION_CONTAINER_H_ */
