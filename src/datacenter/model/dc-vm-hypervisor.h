#ifndef DC_VM_HYPERVISOR_H_
#define DC_VM_HYPERVISOR_H_

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/data-rate.h"

//#include <map>

#include <climits>

#include <sys/time.h>

#include "ns3/dc-fat-tree-helper.h"
#include "ns3/dc-policy.h"
#include "ns3/dc-middlebox.h"
#include "ns3/dc-vm.h"
#include "ns3/dc-flow.h"

#include "ns3/policy_log.h"

#include "ns3/flow-monitor-module.h"

namespace ns3 {

//class PolicyLogAll;
class DcVm;
class DcVmContainer;
struct Graph;

//server has preference on VM
typedef struct PrefrerenceListItem {
	uint32_t id; //vmID
	int64_t value; //preference
} PrefrerenceListItem;

//typedef std::list<PrefrerenceListItem> preferenceList;
typedef std::list<PrefrerenceListItem>::iterator PreferenceListItemIterator;
typedef std::list<PrefrerenceListItem>::reverse_iterator PreferenceListItemReverseIterator;

class PreferenceList {
private:
	std::list<PrefrerenceListItem> m_list;

public:

	PreferenceListItemIterator Begin(void) {
		return m_list.begin();
	}
	;

	PreferenceListItemIterator End(void) {
		return m_list.end();
	}
	;
	PreferenceListItemReverseIterator rBegin(void) {
		return m_list.rbegin();
	}
	;

	PreferenceListItemReverseIterator rEnd(void) {
		return m_list.rend();
	}
	;

	uint32_t Size() {
		return m_list.size();
	}
	;

	bool IsExist(uint32_t id) {
		PreferenceListItemIterator it;
		for (it = m_list.begin(); it != m_list.end(); ++it) {
			if (it->id == id) {
				return true;;
			}
		}

		return false;
	}
	;

	PrefrerenceListItem PopItem() {
		PrefrerenceListItem item;
		if (!m_list.empty()) {
			item = m_list.front();
			m_list.pop_front();
		}
		return item;
	}
	;

	PreferenceListItemIterator Get(uint32_t id) {
		PreferenceListItemIterator it;
		for (it = m_list.begin(); it != m_list.end(); ++it) {
			if (it->id == id) {
				return it;
			}
		}
		return it;
	}
	;

	//get the item, if don't exist, create a new one
	PreferenceListItemIterator GetItem(uint32_t id) {
		PreferenceListItemIterator it;
		for (it = m_list.begin(); it != m_list.end(); ++it) {
			if (it->id == id) {
				return it;
			}
		}

		PrefrerenceListItem item;
		item.id = id;
		item.value = 0;
		//m_list.push_back(item);
		return m_list.insert(m_list.end(), item);
	}
	;

	void AddAtEnd(uint32_t id, int64_t value) {
		PrefrerenceListItem item;
		item.id = id;
		item.value = value;
		m_list.push_back(item);
		//return m_list.insert(m_list.end(), item);
		return;
	}
	;

	//remove item with id
	void Erase(uint32_t id) {
		if (IsExist(id)) {
			PreferenceListItemIterator it = Get(id);
			m_list.erase(it);
		}
	}
	;

	//remve a range of items: [first, last)
	void Erase(PreferenceListItemIterator first,
			PreferenceListItemIterator last) {
		m_list.erase(first, last);
	}
	;

	void Sort(bool IsDecending) {
		if (IsDecending)
			m_list.sort(compare_Decending);
		else
			m_list.sort(compare_Ascending);
	}
	;

	static bool compare_Decending(const PrefrerenceListItem& first,
			const PrefrerenceListItem& second) {
		//return first.value >= second.value;

		if (first.value > second.value)
			return true;
		else
			return false;
	}

	static bool compare_Ascending(const PrefrerenceListItem& first,
			const PrefrerenceListItem& second) {
		//return first.value >= second.value;

		if (first.value < second.value)
			return true;
		else
			return false;
	}

	void Print() {
		std::cout << "Total " << m_list.size() << " items: { ";
		for (PreferenceListItemIterator it = m_list.begin(); it != m_list.end();
				++it) {
			std::cout << "[id=" << it->id << ", value=" << it->value << "],";
		}
		std::cout << "}" << std::endl;
	}
//private:

};

class PreferenceMatrix {
private:
	std::map<uint32_t, PreferenceList> m_matrix;

public:
	typedef std::map<uint32_t, PreferenceList>::iterator PreferenceMatrixIterator;
	uint32_t Size() {
		return m_matrix.size();
	}
	;

	PreferenceMatrixIterator Begin(void) {
		return m_matrix.begin();
	}
	;

	PreferenceMatrixIterator End(void) {
		return m_matrix.end();
	}
	;

	PreferenceList * GetPrefereceList(uint32_t id) {
		//return ((PreferenceList) m_matrix[id]);
		return &(m_matrix[id]);
	}
	;

	//void UpdatePreferenceList(uint32_t id, PreferenceList list)
	//{
	//	m_matrix[id] = list;
	//};

	void SortEachList(bool IsDecending) {
		for (PreferenceMatrixIterator it = Begin(); it != End(); ++it) {
			PreferenceList list = (PreferenceList) it->second;
			list.Sort(IsDecending);
			it->second = list;
		}
	}
	;

	void Print() {
		for (PreferenceMatrixIterator it = Begin(); it != End(); ++it) {
			std::cout << "Preference list for id=" << it->first << " : ";
			PreferenceList list = (PreferenceList) (it->second);
			list.Print();
		}
	}
	;
};

#define MAX_NODE_ID UINT_MAX

typedef std::map<uint32_t, uint32_t>::iterator MatchingIterator;

class VmServerMatching {
private:
	std::map<uint32_t, uint32_t> m_matching;  //<vmID, serverID>, 1:n

public:

	uint32_t Size() {
		return m_matching.size();
	}
	;

	MatchingIterator Begin(void) {
		return m_matching.begin();
	}
	;
	MatchingIterator End(void) {
		return m_matching.end();
	}
	;

	void Add(uint32_t vmID, uint32_t serverID) {
		MatchingIterator it = m_matching.find(vmID);
		if (it == m_matching.end()) //doesn't exist
				{
			m_matching.insert(std::pair<uint32_t, uint32_t>(vmID, serverID));
		} else {
			it->second = serverID;
		}
	}
	;

	std::vector<uint32_t> GetMatchedVmOnServer(uint32_t serverID) {
		std::vector<uint32_t> listVMs;
		for (MatchingIterator it = m_matching.begin(); it != m_matching.end();
				it++) {
			if (it->second == serverID)
				listVMs.push_back(it->first);
		}
		return listVMs;
	}
	;

	uint32_t GetMatchedServerForVm(uint32_t vmID) {
		//uint32_t serverID;
		for (MatchingIterator it = m_matching.begin(); it != m_matching.end();
				it++) {
			if (it->first == vmID)
				return it->second;
		}
		return MAX_NODE_ID;
	}
	;

	void Erase(uint32_t vmID) {
		m_matching.erase(vmID);
	}
	;
};

class DcVmHypervisorApplication: public Application {
public:
	static TypeId GetTypeId(void);
	DcVmHypervisorApplication();
	virtual ~DcVmHypervisorApplication();

	void FlowRxTrace(std::string context, Ptr<const Packet> p,
			const Address &address);

private:
	virtual void StartApplication(void);    // Called at time specified by Start
	virtual void StopApplication(void);     // Called at time specified by Stop
	Ptr<UniformRandomVariable> m_uniformRand;

public:
	static FILE * m_logVmCount;
	//static FILE * m_logConnCount;
	static FILE * m_logVmMoveCount;
	static FILE * m_logfile;

public:
	//For loging, add by cuilin
	uint32_t m_linkcost[4];
	uint64_t m_optimalCost;
	static uint32_t hostIdOwnTheToken;
	static uint32_t vmIdOwnTheToken;
	static uint32_t numOfIterations;
	static uint32_t vmLogCount;
	static std::vector<uint32_t> allVmId; //unique ID for each VM, will be map to allVmContainer
	static std::map<uint32_t, uint32_t> allVmIdMap; //unique ID for each VM, will be map to allVmContainer
	//static std::list<CostItem> allCostList;
	//static std::list<SpaceItem> allSpaceList;
	static uint64_t m_globalTatalCost;
	static uint64_t m_prevGlobalCost;
	static bool needCostRegistration;
	static bool m_enableLog;
	static bool m_centralVmLimit;
	//static std::map<uint32_t, std::vector<NeighborVmItem> > allVmNeighborMap;
	static std::vector<uint32_t> m_gaVmIdList;

	EventId m_calcTotalCostEvent;
	pthread_mutex_t mutex;
	pthread_mutex_t mutexLog;

//================================================
	//For policy consolidation, on a SDN Controller
	EventId m_policyCheckEvent;
	void ScheduleNextPolicyCheck(Time nextTime);
	void policyCheck();

	DcVmContainer getNextIsolatedGroup();

	void policyPhase_I(DcVmContainer groupVMs,
			PreferenceMatrix &preferenceMatrixServer);
	void policyPhase_II(DcVmContainer groupVMs,
			PreferenceMatrix &preferenceMatrixServer);

	Graph * ConstructCostMatrix(Ptr<DcFlowApplication> flow,
			std::vector<uint32_t> & mappingGraph);

	void CheckVmMigration(VmServerMatching & matching);
	void ConstructingPreferenceList_Server2Vm(
			PreferenceMatrix & preferenceMatrixServer, DcVmContainer groupVMs);
	PreferenceMatrix ConstructingPreferenceList_Vm2Server(
			DcVmContainer groupVMs);
	bool checkServerCapacity(VmServerMatching & matching, uint32_t serverID,
			DcVmContainer groupVMs);
	uint32_t getLastMatchedVmForServer(VmServerMatching & matching,
			uint32_t serverID, PreferenceMatrix & preferenceMatrixServer);
	bool IsOkVmOnServer(Ptr<DcVm> vm, uint32_t serverID);
	int64_t CalcUtilityOfMigrationToServer(Ptr<DcVm> vm, uint32_t serverID);
	int64_t CalcUtilityOfPolicyMigration(PolicyItemIterator itPolicy,
			uint8_t seqIndex, uint32_t newMbID, Ptr<DcFlowApplication> flow);
	uint64_t CalcCostofVmOnServer(Ptr<DcVm> vm, uint32_t serverID);
	uint64_t CalcCostofFlow(Ptr<DcFlowApplication> flow);

	NodeContainer getAvailableServer(Ptr<DcVm> vm);
	MiddleBoxContainer getAvailableMB(Ptr<DcMiddleBox> mb,
			Ptr<DcFlowApplication> flow);
	uint32_t CalcPathWeight_Node2Node(uint32_t nodeID1, uint32_t nodeID2);
	uint64_t CalcPathCost_Node2Node(DataRate rate, uint32_t nodeID1,
			uint32_t nodeID2);

	//For SCORE
	EventId m_scoreScheduleEvent;
	void ScheduleSCORE(Time nextTime, uint32_t vmID);
	void ScoreCheck(uint32_t vmID);
	uint64_t CalcCostofVmOnServer_SCORE(Ptr<DcVm> vm, uint32_t serverID);

	void SchedulePeriodLog();
	void LogPeriodAllFlowCost();
	void LogPeriodStats();
	void LogPeriodStatsOLD();
	void LogVmMigration(uint32_t migratedVmID, uint32_t srcNodeID,
			uint32_t dstNodeID, int64_t utility);
	void LogPolicyMigration(uint32_t policyID, uint32_t flowID,
			uint32_t seqIndex, uint32_t srcMbID, uint32_t dstMbID,
			int64_t utility);
	void LogPolicyViolation();

	void LogAlgPerformance(struct timeval start, struct timeval phase1,
			struct timeval phase2);

//================================================

	void globalChangePolicy(uint32_t num, bool status);
	void globalChangeFlow(uint32_t num, bool status);

	//void startFlow(Ptr<DcFlowApplication> flow);
	void CountNVM();
	void ScheduleVmCount();
	//void ScheduleNextCalc (uint32_t interval);
	void ScheduleNextCalc(Time nextTime);
	void CalcTotalCost();
	void RemedySchedule();
	void LogAllLinkUtilization(bool isPeriod);
	void ScheduleSnapshot();
	void LogSnapshotAllVmCost();

	void logSnapshotLinkUtilization();

	uint32_t CalcLinkUtilization_Node2Node(uint32_t nodeID1, uint32_t nodeID2,
			uint64_t lamda, uint64_t* linkFlowsUp, uint64_t* linkFlowsDown);
	//uint64_t CalcCost_VmOnNode(uint32_t hostID, uint32_t vmID);
	//uint64_t CalcCost_VmOnNode_ForInitialPlacement(uint32_t hostID, uint32_t vmID);
	//uint64_t CalcCost_VmOnNode_ForDecisionMaking(uint32_t hostID, uint32_t vmID);
	//uint64_t CalcCost_VmOnNode_NoPolicyScheme(uint32_t hostID, uint32_t vmID);
	//uint64_t CalcAllVmCost();
	//uint32_t CalcLinkCost_Host2MB(uint32_t hostID, uint8_t mid);
	//uint32_t CalcLinkCost_MB2MB(uint8_t mid1, uint8_t mid2);
	//uint32_t CalcLinkCost_Host2Host(uint32_t nodeID_1, uint32_t nodeID_2);

	//uint64_t CalcMigrationTraffic(uint32_t vmID);
	//uint64_t CalcMigrationCost(uint32_t vmID, uint32_t srcHostID, uint32_t dstHostID);

	Ptr<DcVmHypervisorApplication> getHyperVisor(uint32_t NodeID);

	bool isStarted;

	// Global info	
	Ptr<DcFatTreeHelper> fattree;
	MiddleBoxContainer * mbc;
	DcPolicy * policies;
	DcVmContainer * vmc;
	DcFlowContainer * fc;

	Ptr<FlowMonitor> monitor;
	Ptr<Ipv4FlowClassifier> classifier;
	//std::map<FlowId, FlowMonitor::FlowStats> statsLast;
	//int64_t lastStatTime; //in milli sec
	//bool m_isFirstMonitorStatReady;

	PolicyLogAll * policyLog;

	EventId m_vmCountEvent;

	//Local server info
	//std::vector<DcVmNewItemI> m_hostedVMs;
	DcVmContainer m_hostedVMs;	//VMs hosted by current server

	//DcVmContainer m_checkedVMs; //VMs that have been processed, may be unused
	DcVmContainer m_toBeCheckedVMs; //VMs that need to been processed in future policyCheck

	uint32_t m_totalCPU;
	uint32_t m_totalMemory;
	uint32_t m_useCPU;
	uint32_t m_useMemory;

	bool m_overloaded;

	bool m_usePolicy;  //whether policy-aware
	bool m_changePolicy;
	bool m_changeFlow;
};

} /* namespace ns3 */
#endif /* DC_VM_HYPERVISOR_H_ */
