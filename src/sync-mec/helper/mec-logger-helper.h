/*
 * mec-file-helper.h
 *
 *  Created on: 27 Jul 2017
 *      Author: thiagogenez
 */

#ifndef MEC_LOGGER_HELPER_H_
#define MEC_LOGGER_HELPER_H_

#include <iostream>
#include <fstream>

namespace ns3 {

class MecLoggerHelper {
public:
	MecLoggerHelper();
	MecLoggerHelper( const std::string& filename, const  std::string& ext);
	virtual ~MecLoggerHelper();

	void Write(std::string msg);
	void WriteTimeless(std::string msg);

private:

	std::ostream *m_cout;
	std::ofstream *m_file;
};

} /* namespace ns3 */

#endif /* MEC_LOGGER_HELPER_H_ */
