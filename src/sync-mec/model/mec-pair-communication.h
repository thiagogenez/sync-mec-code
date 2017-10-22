/*
 * mec-pair-communication.h
 *
 *  Created on: 30 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_PAIR_COMMUNICATION_H_
#define MEC_PAIR_COMMUNICATION_H_

#include <map>

#include "ns3/object.h"

#include "ns3/mec-policy-container.h"
#include "ns3/mec-flow-container.h"
#include "ns3/mec-vm.h"

namespace ns3 {

class MecPairCommunication: public Object {
public:
	MecPairCommunication();
	virtual ~MecPairCommunication();

	uint32_t GetId() const;

	void AddVm(Ptr<MecVm> vm);
	void AddUe(Ptr<MecVm> ue);

	void AddUploadPolicyFlow(Ptr<MecFlow> flow, Ptr<MecPolicy> policy);
	void AddDownloadPolicyFlow(Ptr<MecFlow> flow, Ptr<MecPolicy> policy);

	void Print(std::ostream& os) const;

	//Getters
	MecFlowContainer& GetDownloadFlows();
	MecFlowContainer& GetUploadFlows();

	Ptr<MecVm>& GetVm();
	Ptr<MecVm>& GetUe();

	MecPolicyContainer& GetUploadPolicies();
	MecPolicyContainer& GetDownloadPolicies();

	std::map<Ptr<MecFlow>, Ptr<MecPolicy> >& GetDownloadPolicyFlowMap();
	std::map<Ptr<MecFlow>, Ptr<MecPolicy> >& GetUploadPolicyFlowMap();

private:

	// general uses
	static uint32_t m_counter;
	uint32_t m_id;

	Ptr<MecVm> m_Ue;
	Ptr<MecVm> m_Vm;

	MecFlowContainer m_uploadFlows;
	MecFlowContainer m_downloadFlows;

	MecPolicyContainer m_uploadPolicies;
	MecPolicyContainer m_downloadPolicies;

	std::map<Ptr<MecFlow>, Ptr<MecPolicy>> m_uploadPolicyFlowMap;
	std::map<Ptr<MecFlow>, Ptr<MecPolicy>> m_downloadPolicyFlowMap;

};

} /* namespace ns3 */

#endif /* MEC_PAIR_COMMUNICATION_H_ */
