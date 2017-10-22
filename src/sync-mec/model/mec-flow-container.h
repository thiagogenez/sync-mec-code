/*
 * mec-flow-container.h
 *
 *  Created on: 9 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_FLOW_CONTAINER_H_
#define MEC_FLOW_CONTAINER_H_

#include <vector>
#include "ns3/object.h"

#include "ns3/mec-flow.h"

namespace ns3 {

class MecFlowContainer {
public:

	typedef std::vector<Ptr<MecFlow> >::const_iterator const_iterator;

	MecFlowContainer();
	virtual ~MecFlowContainer();

	const_iterator CBegin() const;
	const_iterator CEnd() const;
	uint32_t GetSize() const;
	Ptr<MecFlow> GetFlowByIndex(uint32_t i) const;

	void Add(Ptr<MecFlow> flow);
	void Add(MecFlowContainer other);
	void Remove(Ptr<MecFlow> vm);
	void CreateFlowContainer(uint32_t n);

	bool Empty() const;
	void Print(std::ostream& os) const;

	bool IsFlowExist(uint32_t flowID) const;

private:

	typedef std::vector<Ptr<MecFlow> >::iterator iterator;

	const_iterator GetFlowById(uint32_t flowID) const;


	std::vector<Ptr<MecFlow> > m_flowList;
};

} /* namespace ns3 */

#endif /* MEC_FLOW_CONTAINER_H_ */
