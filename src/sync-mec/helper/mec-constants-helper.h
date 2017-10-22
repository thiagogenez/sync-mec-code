/*
 * mec-constants.h
 *
 *  Created on: 9 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_CONSTANTS_H_
#define MEC_CONSTANTS_H_

#include "ns3/nstime.h"

#include "ns3/string.h"

namespace ns3 {

class MecConstantsHelper {

public:
	static const StringValue BACKHAUL_DATA_RATE;

	static const StringValue DELAY;

	// used in scenario-generator.cc
	static const std::string MIDDLEBOX_CAPACITY;

	// Most used by mec-flow.cc
	static const std::string FLOW_DATA_RATE_DEFAULT;
	static const bool FLOW_USE_TCP;
	static const uint32_t FLOW_SEND_SIZE_DEFAULT;

	// Most used by sdn controller
	static const Time SDN_CONTROLLER_NEXT_POLICY_CHECK_TIME;
	static const Time FLOW_MONITOR_NEXT_CHECK_TIME;

	// Most used by scenario-generator.cc
	static Time HYPERVISOR_START_TIME;
	static Time SIMULATION_STOP_TIME;
	static const uint32_t MEC_UDP_DST_PORT;
	static const uint32_t MEC_TCP_DST_PORT;

	// E2E checker
	static Time E2E_CHECKER_START_TIME;
	static Time E2E_CHECKER_STOP_TIME;
	static Time E2E_CHECKER_PING_INTERVAL_TIME;

	//Used in mec-policy.h
	static const uint8_t POLICY_UDP_PROTOCOL;
	static const uint8_t POLICY_TCP_PROTOCOL;

	// Most used in scenario-generator
	static uint32_t MIN_UPLOAD_FLOW_DATA_RATE;	//kbps
	static uint32_t MAX_UPLOAD_FLOW_DATA_RATE;	//kbps
	static uint32_t MIN_DOWNLOAD_FLOW_DATA_RATE;	//kbps
	static uint32_t MAX_DOWNLOAD_FLOW_DATA_RATE;	//kbps
	static const uint32_t MAX_FLOW_PER_VM;
	static const uint32_t MIN_FLOW_PER_VM;

	static const uint32_t MAX_FLOW_NPACKETS_DEFAULT;
	static std::string FLOW_DATE_RATE_UNITS;

	static const uint32_t TCP_PACKET_SIZE;
	static const uint32_t UDP_PACKET_SIZE;
	static const bool USE_TCP;
	static uint16_t TCP_PORT_SEED;
	static uint16_t UDP_PORT_SEED;
	static const uint16_t AMOUNT_OF_MIDDLEBOX_TYPE; // 0: firewall, ..., n-1: NAT
	static const uint16_t MAX_POLICY_LEN;
	static const uint16_t MIN_POLICY_LEN;
	static const uint32_t PERCENTAGE_POLICY_FREE;

	// Most used to configure VMs
	static const uint32_t MIN_TOTAL_VM_CPU;
	static const uint32_t MAX_TOTAL_VM_CPU;

	static const uint32_t MIN_TOTAL_VM_MEMORY;
	static const uint32_t MAX_TOTAL_VM_MEMORY;

	static double MIN_LIVE_MIGRATION_VM_SIZE;
	static double MAX_LIVE_MIGRATION_VM_SIZE;
	static double AVERAGE_VM_SIZE;

	static double MIN_LIVE_MIGRATION_DIRTY_RATE_PAGE;
	static double MAX_LIVE_MIGRATION_DIRTY_RATE_PAGE;

	static double LIVE_MIGRATION_L;
	static double LIVE_MIGRATION_X;
	static double LIVE_MIGRATION_T;

	//Most used to configure Hosts
	static const uint32_t TOTAL_HOST_SERVER_CPU;
	static const uint32_t TOTAL_HOST_SERVER_MEMORY;

	static uint32_t MIN_NUMBER_VMS_ON_HOST_SERVER;
	static uint32_t MAX_NUMBER_VMS_ON_HOST_SERVER;

	static uint32_t UE_MOBILITY_MIN_PERCENTAGEM;
	static uint32_t UE_MOBILITY_MAX_PERCENTAGEM;
	static Time UE_NEXT_MOBILITY_TIME;

private:
	MecConstantsHelper();
	virtual ~MecConstantsHelper();
};

} /* namespace ns3 */

#endif /* MEC_CONSTANTS_H_ */
