/*
 * MecPairCommunication.cc
 *
 *  Created on: 30 Jun 2017
 *      Author: thiagogenez
 */

#include "ns3/assert.h"

#include "mec-pair-communication.h"

namespace ns3 {

//Id initialisation
uint32_t MecPairCommunication::m_counter = 0;

MecPairCommunication::MecPairCommunication() {
	m_id = MecPairCommunication::m_counter;
	MecPairCommunication::m_counter++;
}

MecPairCommunication::~MecPairCommunication() {
}

uint32_t MecPairCommunication::GetId() const {
	return m_id;
}

void MecPairCommunication::AddVm(Ptr<MecVm> vm) {
	m_Vm = vm;
}
void MecPairCommunication::AddUe(Ptr<MecVm> ue) {
	m_Ue = ue;
}

void MecPairCommunication::AddUploadPolicyFlow(Ptr<MecFlow> flow,
		Ptr<MecPolicy> policy) {
	m_uploadFlows.Add(flow);
	m_uploadPolicies.Add(policy);
	m_uploadPolicyFlowMap.insert(
			std::pair<Ptr<MecFlow>, Ptr<MecPolicy>>(flow, policy));

}
void MecPairCommunication::AddDownloadPolicyFlow(Ptr<MecFlow> flow,
		Ptr<MecPolicy> policy) {
	m_downloadFlows.Add(flow);
	m_downloadPolicies.Add(policy);

	std::pair<std::map<Ptr<MecFlow>, Ptr<MecPolicy>>::iterator, bool> ret;

	ret = m_downloadPolicyFlowMap.insert(
			std::pair<Ptr<MecFlow>, Ptr<MecPolicy>>(flow, policy));

	NS_ASSERT_MSG(ret.second,
			"MecPairCommunication::AddDownloadPolicyFlow  --> element already existed flowId=" << flow->GetId() << " policyId=" << policy->GetId());
}

void MecPairCommunication::Print(std::ostream& os) const {
	os << "Pair id=" << GetId() << std::endl;
	m_Vm->Print(os);
	m_Ue->Print(os);

	os << "Upload flows containers UE -> VM: " << std::endl;
	m_uploadFlows.Print(os);

	os << "Download flows containers VM -> UE: " << std::endl;
	m_downloadFlows.Print(os);

	os << "m_downloadPolicyFlowMap.size=" << m_downloadPolicyFlowMap.size()
			<< std::endl;
	os << "m_uploadPolicyFlowMap.size=" << m_uploadPolicyFlowMap.size()
			<< std::endl;
	os << std::endl;
}

MecFlowContainer& MecPairCommunication::GetDownloadFlows() {
	return m_downloadFlows;
}

Ptr<MecVm>& MecPairCommunication::GetUe() {
	return m_Ue;
}

MecFlowContainer& MecPairCommunication::GetUploadFlows() {
	return m_uploadFlows;
}

Ptr<MecVm>& MecPairCommunication::GetVm() {
	return m_Vm;
}

MecPolicyContainer& MecPairCommunication::GetUploadPolicies() {
	return m_uploadPolicies;
}

MecPolicyContainer& MecPairCommunication::GetDownloadPolicies() {
	return m_downloadPolicies;
}

std::map<Ptr<MecFlow>, Ptr<MecPolicy> >& MecPairCommunication::GetDownloadPolicyFlowMap() {
	return m_downloadPolicyFlowMap;
}

std::map<Ptr<MecFlow>, Ptr<MecPolicy> >& MecPairCommunication::GetUploadPolicyFlowMap() {
	return m_uploadPolicyFlowMap;
}

} /* namespace ns3 */
