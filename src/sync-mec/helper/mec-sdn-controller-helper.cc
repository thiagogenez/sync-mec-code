/*
 * mec-sdn-controller-helper.cc
 *
 *  Created on: 12 Jul 2017
 *      Author: thiagogenez
 */

#include "ns3/log.h"
#include "ns3/assert.h"

#include "mec-sdn-controller-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MecSdnControllerHelper");
NS_OBJECT_ENSURE_REGISTERED(MecSdnControllerHelper);


//Id initialisation
uint64_t MecSdnControllerHelper::m_counter = 0;

TypeId MecSdnControllerHelper::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::MecSdnControllerHelper").SetParent<Object>().SetGroupName(
					"sync-mec");
	return tid;
}

MecSdnControllerHelper::MecSdnControllerHelper() {
	m_id = MecSdnControllerHelper::m_counter;
	MecSdnControllerHelper::m_counter++;
}

MecSdnControllerHelper::~MecSdnControllerHelper() {
}

void MecSdnControllerHelper::AddMecPolicyMigrationHelper(
		Ptr<MecPolicyMigrationHelper> policyMigrationHelper) {

	m_policyInAnalyse.insert(
			std::pair<uint32_t, Ptr<MecPolicyMigrationHelper> >(
					policyMigrationHelper->GetId(), policyMigrationHelper));

}

Ptr<Node> MecSdnControllerHelper::GetPreferibleHostServer() {
	std::map<uint32_t, Ptr<MecPolicyMigrationHelper>>::iterator it;

	// check the choosen of each policy and count
	for (it = m_policyInAnalyse.begin(); it != m_policyInAnalyse.end(); it++) {
		Ptr<MecPolicyMigrationHelper> policyMigrationHelper = it->second;

		Ptr<Node> preferable =
				policyMigrationHelper->GetPreferableHostServerForVm();

		std::map<uint32_t, uint32_t>::iterator itNode;
		itNode = m_HostServerPreferibleToVm.find(preferable->GetId());

		NS_ASSERT_MSG(itNode != m_HostServerPreferibleToVm.end(),
				"MecSdnControllerHelper::GetPreferibleHostServer --> ERROR, Host id="<<preferable->GetId() << "not found!!!");

		m_HostServerPreferibleToVm[preferable->GetId()]++;
	}

	uint32_t max = 0;
	uint32_t preferibleId = 0;
	uint32_t sum = 0;
	// select the winner
	for (std::map<uint32_t, uint32_t>::iterator itNode =
			m_HostServerPreferibleToVm.begin();
			itNode != m_HostServerPreferibleToVm.end(); itNode++) {

		uint32_t currentNodeId = itNode->first;
		uint32_t votes = itNode->second;

		if (votes > max) {
			max = votes;
			preferibleId = currentNodeId;
		}

		sum += votes;
	}

	NS_ASSERT_MSG(sum == m_policyInAnalyse.size(),
			"MecSdnControllerHelper::GetPreferibleHostServer() -> some decision is missing  votes-sum="<<sum<< ";number_policies="<<m_policyInAnalyse.size());

	// return the winner
	Ptr<Node> preferible = m_HostServerCandidatesToVm[preferibleId];

	return preferible;
}

void MecSdnControllerHelper::AddtHostServersCandidatesToVm(
		NodeContainer &nodeContainer) {

	for (uint32_t i = 0; i < nodeContainer.GetN(); i++) {
		Ptr<Node> node = nodeContainer.Get(i);

		m_HostServerCandidatesToVm.insert(
				std::pair<uint32_t, Ptr<Node>>(node->GetId(), node));

		m_HostServerPreferibleToVm.insert(
				std::pair<uint32_t, uint32_t>(node->GetId(), 0));
	}

}

bool MecSdnControllerHelper::AllPolicyMigrationDone() {
	std::map<uint32_t, Ptr<MecPolicyMigrationHelper>>::iterator it;

	for (it = m_policyInAnalyse.begin(); it != m_policyInAnalyse.end(); it++) {
		Ptr<MecPolicyMigrationHelper> policyMigrationHelper = it->second;

		if (!policyMigrationHelper->IsPolicyMigrationDone()) {
			return false;
		}
	}

	return true;
}


void MecSdnControllerHelper::ResetTimer() {
	m_realTimer.Reset();
}
double MecSdnControllerHelper::ElapseTimer() const {
	return m_realTimer.Elapsed();
}

Ptr<MecPolicyMigrationHelper> MecSdnControllerHelper::GetPolicyMigrationHelper(
		uint32_t policyMigrationHelperId) {
	return m_policyInAnalyse[policyMigrationHelperId];
}

uint64_t MecSdnControllerHelper::GetId() const {
	return m_id;
}

bool MecSdnControllerHelper::IsPhase_II_InitiationTrigged() {
	return m_phase_II_trigged;
}

void MecSdnControllerHelper::TriggerPhase_II() {
	m_phase_II_trigged = true;
}

void MecSdnControllerHelper::SetPair(Ptr<MecPairCommunication> pair) {
	m_pair = pair;
}

Ptr<MecPairCommunication> MecSdnControllerHelper::GetPair() const {
	return m_pair;
}

uint32_t MecSdnControllerHelper::GetPairId() const{
	return m_pair->GetId();
}

} /* namespace ns3 */
