/*
 * mec-utils.h
 *
 *  Created on: 14 Jun 2017
 *      Author: thiagogenez
 */

#ifndef MEC_UTILS_H_
#define MEC_UTILS_H_

#include <vector>

#include <algorithm>    // std::random_shuffle
#include <iostream>
#include <random>
#include <sstream>

#include "ns3/object.h"
#include "ns3/string.h"
#include "ns3/node.h"
#include "ns3/ipv4-address.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"

#include "ns3/mec-vm-hypervisor.h"
#include "ns3/mec-flow.h"

namespace ns3 {

//class MecVmHypervisor;
//class MecFlow;

class MecUtilsHelper {

public:

	static void ExtractFlowNetworkNodeInformation(Ptr<MecFlow> flow,
			uint32_t *srcAddr, uint32_t *dstAddr, uint8_t* protocol,
			uint16_t *srcPort, uint16_t *dstPort);

	static Ptr<MecVmHypervisor> GetHyperVisor(uint32_t nodeId);

	static Ptr<Node> GetNode(uint32_t nodeId);

	static Ipv4Address GetIpv4Address(Ptr<Node> node, uint32_t interface,
			uint32_t addressIndex);

	static Ipv4Address GetIpv4Address(uint32_t nodeId, uint32_t interface,
			uint32_t addressIndex);

	static uint32_t GetRandomIntegerNumber(uint32_t min, uint32_t max);

	// I don't know why but template static methods can be linked when
	// implemented here (in the .h file) and not in the .cc file

	template<typename T>
	static std::string ToString(const T& t) {
		std::ostringstream ss;
		ss << t;
		return ss.str();
	}

	template<typename T>
	static void Suffle(T begin, T end) {

//		std::random_device rd;
//		std::mt19937 g(rd());
//		std::shuffle(begin, end, g);

		std::shuffle(begin, end, std::default_random_engine((rand() % 100)));
	}

	template<typename T>
	static std::string VectorToString(T begin, T end) {
		std::ostringstream ss;
		bool first = true;
		for (; begin != end; begin++) {
			if (!first) {
				ss << ", ";
			}
			ss << *begin;
			first = false;
		}
		return ss.str();
	}

	template<typename T>
	static void GenerateRandomVectorOfUniqueNumbers(std::vector<T>& dst, T from,
			T to, uint32_t size) {

		dst.clear();
		for (T i = from; i <= to; i++) {
			dst.push_back(i);
		}

		Suffle(dst.begin(), dst.end());
		size = dst.size() - size;
		if (size > 0) {
			while (size > 0) {
				dst.pop_back();
				size--;
			}
		}
	}

	template<typename T>
	static void GenerateRandomVectorOfUniqueNumbers(std::vector<T>& source,
			uint32_t size) {

		Suffle(source.begin(), source.end());
		size = source.size() - size;
		if (size > 0) {
			while (size > 0) {
				source.pop_back();
				size--;
			}
		}
	}

private:
	MecUtilsHelper();
	//virtual ~MecUtilsHelper();

};

} /* namespace ns3 */

#endif /* MEC_UTILS_H_ */
