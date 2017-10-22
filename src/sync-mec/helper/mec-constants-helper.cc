/*
 * mec-constants.cpp
 *
 *  Created on: 9 Jun 2017
 *      Author: thiagogenez
 */

#include "mec-constants-helper.h"

namespace ns3 {

MecConstantsHelper::MecConstantsHelper() {

}

MecConstantsHelper::~MecConstantsHelper() {
}

const StringValue MecConstantsHelper::BACKHAUL_DATA_RATE = StringValue("5Gbps"); // 1Gbps
const StringValue MecConstantsHelper::DELAY = StringValue("1ms");	// 0.001 ms

const std::string MecConstantsHelper::MIDDLEBOX_CAPACITY = "10Gbps";

const bool MecConstantsHelper::FLOW_USE_TCP = false;

const Time MecConstantsHelper::SDN_CONTROLLER_NEXT_POLICY_CHECK_TIME = Seconds(
		3);
const Time MecConstantsHelper::FLOW_MONITOR_NEXT_CHECK_TIME = MilliSeconds(250);

const uint32_t MecConstantsHelper::MEC_UDP_DST_PORT = 999;
const uint32_t MecConstantsHelper::MEC_TCP_DST_PORT = 999;

const uint8_t MecConstantsHelper::POLICY_UDP_PROTOCOL = 17;
const uint8_t MecConstantsHelper::POLICY_TCP_PROTOCOL = 6;

const uint32_t MecConstantsHelper::MIN_FLOW_PER_VM = 1;
const uint32_t MecConstantsHelper::MAX_FLOW_PER_VM = 5;

uint32_t MecConstantsHelper::MIN_UPLOAD_FLOW_DATA_RATE = 3;	//kbps
uint32_t MecConstantsHelper::MAX_UPLOAD_FLOW_DATA_RATE = 6;	//kbps

uint32_t MecConstantsHelper::MIN_DOWNLOAD_FLOW_DATA_RATE = 3;	//kbps
uint32_t MecConstantsHelper::MAX_DOWNLOAD_FLOW_DATA_RATE = 6;	//kbps

std::string MecConstantsHelper::FLOW_DATE_RATE_UNITS = "kbps"; //"Mbps";
uint32_t MecConstantsHelper::MIN_NUMBER_VMS_ON_HOST_SERVER = 2;
uint32_t MecConstantsHelper::MAX_NUMBER_VMS_ON_HOST_SERVER = 4;
const uint16_t MecConstantsHelper::AMOUNT_OF_MIDDLEBOX_TYPE = 3; // 1: firewall, ..., 5: NAT
const uint16_t MecConstantsHelper::MIN_POLICY_LEN = 1;
const uint16_t MecConstantsHelper::MAX_POLICY_LEN = 3;
const uint32_t MecConstantsHelper::PERCENTAGE_POLICY_FREE = 20;
Time MecConstantsHelper::HYPERVISOR_START_TIME = Seconds(0.1);
Time MecConstantsHelper::SIMULATION_STOP_TIME = Seconds(10.5); //(5 + 0.0001);
Time MecConstantsHelper::E2E_CHECKER_START_TIME = Seconds(0.01); //(5 + 0.0001);
Time MecConstantsHelper::E2E_CHECKER_STOP_TIME = Seconds(30); //(5 + 0.0001);
Time MecConstantsHelper::E2E_CHECKER_PING_INTERVAL_TIME = MilliSeconds(500);

const std::string MecConstantsHelper::FLOW_DATA_RATE_DEFAULT = "500kbps";
const uint32_t MecConstantsHelper::FLOW_SEND_SIZE_DEFAULT = 1468;
const uint32_t MecConstantsHelper::MAX_FLOW_NPACKETS_DEFAULT = 1000000;

const uint32_t MecConstantsHelper::TCP_PACKET_SIZE = 1456; //  = 1500 - 20 - 20 - 4
const uint32_t MecConstantsHelper::UDP_PACKET_SIZE = 1468; // = 1500 - 20 (ip) - 8 (udp) - 4 (policy header)
uint16_t MecConstantsHelper::TCP_PORT_SEED = 1000;
uint16_t MecConstantsHelper::UDP_PORT_SEED = 1000;
const bool MecConstantsHelper::USE_TCP = false;

const uint32_t MecConstantsHelper::MIN_TOTAL_VM_CPU = 5;
const uint32_t MecConstantsHelper::MAX_TOTAL_VM_CPU = 10;
const uint32_t MecConstantsHelper::MIN_TOTAL_VM_MEMORY = 5;
const uint32_t MecConstantsHelper::MAX_TOTAL_VM_MEMORY = 10;

double MecConstantsHelper::MIN_LIVE_MIGRATION_VM_SIZE = 50.0;
double MecConstantsHelper::MAX_LIVE_MIGRATION_VM_SIZE = 150.0;
double MecConstantsHelper::AVERAGE_VM_SIZE = (50.0 + 150.0) / 2.0;

double MecConstantsHelper::MIN_LIVE_MIGRATION_DIRTY_RATE_PAGE = 0.0;
double MecConstantsHelper::MAX_LIVE_MIGRATION_DIRTY_RATE_PAGE = 3.0;

double MecConstantsHelper::LIVE_MIGRATION_L = 10.0;

double MecConstantsHelper::LIVE_MIGRATION_X =
		MecConstantsHelper::AVERAGE_VM_SIZE / 10.0;

double MecConstantsHelper::LIVE_MIGRATION_T = (AVERAGE_VM_SIZE
		/ LIVE_MIGRATION_L) / 10.0;

const uint32_t MecConstantsHelper::TOTAL_HOST_SERVER_CPU = 500;
const uint32_t MecConstantsHelper::TOTAL_HOST_SERVER_MEMORY = 500;

uint32_t MecConstantsHelper::UE_MOBILITY_MIN_PERCENTAGEM = 10;
uint32_t MecConstantsHelper::UE_MOBILITY_MAX_PERCENTAGEM = 60;
Time MecConstantsHelper::UE_NEXT_MOBILITY_TIME = Seconds(101) + MicroSeconds(1234);

}
/* namespace ns3 */
