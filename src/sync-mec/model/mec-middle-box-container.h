/*
 * mec-middle-box-container.h
 *
 *  Created on: 7 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_MIDDLE_BOX_CONTAINER_H_
#define MEC_MIDDLE_BOX_CONTAINER_H_

#include <vector>
#include "ns3/object.h"

#include "ns3/mec-middle-box.h"

namespace ns3 {

class MecMiddleBoxContainer {
public:

	typedef std::vector<Ptr<MecMiddleBox> >::const_iterator const_iterator;

	MecMiddleBoxContainer();
	virtual ~MecMiddleBoxContainer();

	const_iterator CBegin() const;
	const_iterator CEnd() const;
	uint32_t GetSize() const;
	Ptr<MecMiddleBox> GetMiddleboxByIndex(uint32_t i) const;
	MecMiddleBoxContainer GetMiddleBoxByType(uint16_t type);
	std::vector<uint16_t> GetTypes() const;

	bool Empty() const;
	bool Contains(Ptr<MecMiddleBox> middlebox);

	void SetMiddleBoxAt(uint32_t index, Ptr<MecMiddleBox> newMB);

	void Add(Ptr<MecMiddleBox> mb);

	void CreateMiddleBoxContainer(uint32_t n, std::string dataRate,
			std::vector<uint16_t> types);
	void Remove(Ptr<MecMiddleBox> mb);
	void Clear();
	void Print(std::ostream& os);

private:

	typedef std::vector<Ptr<MecMiddleBox> >::iterator iterator;
	std::vector<Ptr<MecMiddleBox> > m_mbList;

};

} /* namespace ns3 */

#endif /* MEC_MIDDLE_BOX_CONTAINER_H_ */
