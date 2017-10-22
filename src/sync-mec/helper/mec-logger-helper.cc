/*
 * mec-file-helper.cc
 *
 *  Created on: 27 Jul 2017
 *      Author: thiagogenez
 */

#include "ns3/nstime.h"
#include "ns3/simulator.h"

#include "mec-logger-helper.h"

namespace ns3 {

MecLoggerHelper::MecLoggerHelper() {

	m_cout = &std::cout;
	m_file = NULL;
}

MecLoggerHelper::MecLoggerHelper(const std::string& filename,
		const std::string& ext) {

	m_file = new std::ofstream(filename + ext, std::ios::out);

	if (!m_file->is_open()) {
	}

	m_cout = m_file;

}

MecLoggerHelper::~MecLoggerHelper() {

	if (m_cout->rdbuf() != std::cout.rdbuf()) {
		if (m_file) {
			m_file->close();
//			delete m_cout;
//			delete m_file;
		}
	}
}

void MecLoggerHelper::Write(std::string msg) {

	(*m_cout) << Simulator::Now() << " : " << msg << "\n";
	m_cout->flush();
}

void MecLoggerHelper::WriteTimeless(std::string msg) {

	(*m_cout) << msg << "\n";
	m_cout->flush();
}

} /* namespace ns3 */
