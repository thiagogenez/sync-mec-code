/*
 * mec-logging.h
 *
 *  Created on: 20 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_LOGGER_H_
#define MEC_LOGGER_H_

#include "ns3/string.h"
#include "ns3/mec-logger-helper.h"

namespace ns3 {

class MecLogger {

private:

	MecLogger();
	virtual ~MecLogger();

public:

	static MecLoggerHelper VERBOSE;
	static MecLoggerHelper TIMER;

	static MecLoggerHelper UPLOAD;
	static MecLoggerHelper DOWNLOAD;

//	MecLogger(std::string filename);

// loggin purposes
	static void Logging(const std::string &msg);
	static void LoggingError(std::string);
	static void LoggingRealTime(uint32_t step, double realTime);

	static void LoggingFlowMonitor( std::string msg, bool isDownload);

	static void VerboseToFile( const std::string& filename,
			 const std::string& ex);

	static void RealTimeToFile(const std::string& filename,
			const std::string& ex);

	static void UploadFlowMonitorToFile( const std::string& filename,
			const std::string& ex);

	static void DownloadFlowMonitorToFile( const std::string& filename,
			const std::string& ex);

};

} /* namespace ns3 */

#endif /* MEC_LOGGER_H_ */
