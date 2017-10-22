/*
 * mec-vm-container.h
 *
 *  Created on: 12 Jun 2017
 *      Author: thiagogenez
 */

#ifndef VM_CONTAINER_H_
#define VM_CONTAINER_H_

#include <vector>
#include "ns3/object.h"

#include "ns3/mec-vm.h"

namespace ns3 {

class MecVmContainer {
public:

	typedef std::vector<Ptr<MecVm> >::const_iterator const_iterator;
	typedef std::vector<Ptr<MecVm> >::iterator iterator;

	MecVmContainer();
	virtual ~MecVmContainer();

	const_iterator CBegin() const;
	const_iterator CEnd() const;
	iterator Begin();
	iterator End();
	uint32_t GetSize() const;
	Ptr<MecVm> GetVmByIndex(uint32_t i) const;
	MecVmContainer::const_iterator GetVmById(uint32_t vmId) const;
	bool Contain(uint32_t vmId) const;
	bool Empty() const;

	void Add(MecVmContainer other);
	void Add(Ptr<MecVm> vm);
	void Remove(Ptr<MecVm> vm);
	void CreateVmContainer(uint32_t n);
	void CreateVmContainer(uint32_t n, bool isUe);
	void Print(std::ostream& os) const;

	bool isIsUe();
	bool isIsVm();

private:
	std::vector<Ptr<MecVm> > m_VmList;

	bool isUE = false;
	bool isVM = false;
};

} /* namespace ns3 */

#endif /* VM_CONTAINER_H_ */
