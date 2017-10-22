/*
 * mec-sdn-controller-helper.h
 *
 *  Created on: 12 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_SDN_CONTROLLER_HELPER_H_
#define MEC_SDN_CONTROLLER_HELPER_H_

#include "ns3/object.h"

#include "ns3/mec-policy-migration-helper.h"
#include "ns3/mec-pair-communication.h"
#include "ns3/mec-real-timer.h"

//#include <map>
//#include <vector>

namespace ns3 {

class MecSdnControllerHelper: public Object {
public:
	MecSdnControllerHelper();
	virtual ~MecSdnControllerHelper();
	static TypeId GetTypeId();

	void AddMecPolicyMigrationHelper(
			Ptr<MecPolicyMigrationHelper> policyMigrationHelper);
	void AddtHostServersCandidatesToVm(NodeContainer &nodeContainer);

	bool AllPolicyMigrationDone();
	uint64_t GetId() const;
	bool IsPhase_II_InitiationTrigged();
	void TriggerPhase_II();
	Ptr<Node> GetPreferibleHostServer();
	Ptr<MecPolicyMigrationHelper> GetPolicyMigrationHelper(
			uint32_t policyMigrationHelperId);

	void SetPair(Ptr<MecPairCommunication> pair);
	Ptr<MecPairCommunication> GetPair() const;
	uint32_t GetPairId() const;

	void ResetTimer();
	double ElapseTimer() const;

private:


	static uint64_t m_counter;
	uint64_t m_id;
	uint32_t m_pairId = 0;

	std::map<uint32_t, Ptr<MecPolicyMigrationHelper> > m_policyInAnalyse;
	std::map<uint32_t, Ptr<Node>> m_HostServerCandidatesToVm;
	std::map<uint32_t, uint32_t> m_HostServerPreferibleToVm;

	Ptr<MecPairCommunication> m_pair;

	bool m_phase_II_trigged = false;

	MecRealTimer m_realTimer;

};

} /* namespace ns3 */

#endif /* MEC_SDN_CONTROLLER_HELPER_H_ */
