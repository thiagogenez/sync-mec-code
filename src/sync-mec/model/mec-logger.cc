/*
 * mec-logging.cc
 *
 *  Created on: 20 Jul 2017
 *      Author: thiagogenez
 */

#include "mec-logger.h"

#include "ns3/nstime.h"
#include "ns3/simulator.h"

namespace ns3 {

MecLoggerHelper MecLogger::VERBOSE;
MecLoggerHelper MecLogger::TIMER;

MecLoggerHelper MecLogger::DOWNLOAD;
MecLoggerHelper MecLogger::UPLOAD;

MecLogger::MecLogger() {

}

MecLogger::~MecLogger() {
}

void MecLogger::Logging(const std::string& msg) {
	VERBOSE.Write(msg);
}

void MecLogger::LoggingError(std::string msg) {
	Time time = Simulator::Now();
	std::cerr << time << " : " << msg << std::endl;
}

void MecLogger::LoggingRealTime(uint32_t step, double realTime) {
	Time time = Simulator::Now();
	std::ostringstream msg;

	msg << time.GetSeconds() << ";" << step << ";" << realTime;


	TIMER.WriteTimeless(msg.str());

}

void MecLogger::LoggingFlowMonitor(std::string txt, bool isDownload) {
	Time time = Simulator::Now();
	std::ostringstream msg;

	msg << time.GetSeconds() << ";" << txt;

	if (isDownload) {
		DOWNLOAD.WriteTimeless(msg.str());
	}

	else {
		UPLOAD.WriteTimeless(msg.str());
	}
}

void MecLogger::VerboseToFile(const std::string& filename,
		const std::string& ex) {

	static MecLoggerHelper newVerbose(filename, ex);
	VERBOSE = newVerbose;
}
void MecLogger::RealTimeToFile(const std::string& filename,
		const std::string& ex) {

	static MecLoggerHelper newTimer(filename, ex);
	TIMER = newTimer;

	std::string header = "simtime;step;realtime";

	TIMER.WriteTimeless(header);
}

void MecLogger::UploadFlowMonitorToFile(const std::string& filename,
		const std::string& ex) {

	static MecLoggerHelper newUpload(filename, ex);
	UPLOAD = newUpload;
	UPLOAD.WriteTimeless("time;id;latency;throughput;hops;loss;jitter");
}

void MecLogger::DownloadFlowMonitorToFile(const std::string& filename,
		const std::string& ex) {

	static MecLoggerHelper newDownload(filename, ex);

	DOWNLOAD = newDownload;
	DOWNLOAD.WriteTimeless("time;id;latency;throughput;hops;loss;jitter");

}

} /* namespace ns3 */
