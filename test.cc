#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include "RigolScope.hh"

template <class T>
inline std::string convertToString(const T& t) {

	std::stringstream ss;
	ss << t;
	return ss.str();

}

int main() {

	std::vector<float> temp;
	std::string temp2 = "";	

	std::cout << "Creating RigolScope object..." << std::endl;
	// RigolScope scope("/dev/ttyUSB0", Baud_38400);
	RigolScope scope("/dev/ttyUSB0", Baud_9600);

	try {
		std::cout << "Getting info..." << std::endl;
		temp2 = scope.getInfo();
	}
	catch(...) {
		std::cout << "vituiks man" << std::endl;
		return -1;
	}

	std::cout << temp2 << std::endl;

	std::cout << "Changing timeout..." << std::endl;
	scope.setSerialTimeout(boost::posix_time::seconds(10));
	//std::cout << "Changing serial port speed..." << std::endl;
	//scope.setSerialSpeed(Baud_38400);

	// scope.setAuto();
	std::cout << std::endl << "Timescale: " << convertToString(scope.getTimescale()) << " s/div" << std::endl;
	std::cout << "Time offset: " << convertToString(scope.getTimeOffset()) << "s" << std::endl;
	std::cout << "Frequency counter enabled: " << convertToString(scope.getFreqCounterEnable()) << std::endl;
	//std::cout << "Frequency: " << convertToString(scope.getFreqCounterValue()) << "Hz" << std::endl;
	std::cout << "Trigger mode: " << scope.getEnumString(scope.getTriggerMode()) << std::endl;
	std::cout << "Trigger source: " << scope.getEnumString(scope.getTriggerSource(scope.getTriggerMode())) << std::endl;
	std::cout << "Trigger level: " << convertToString(scope.getTriggerLevel(scope.getTriggerMode())) << std::endl;
	std::cout << "Trigger sweep: " << scope.getEnumString(scope.getTriggerSweep(scope.getTriggerMode())) << std::endl;
	std::cout << "Trigger coupling: " << scope.getEnumString(scope.getTriggerCoupling(scope.getTriggerMode())) << std::endl;
	std::cout << "Trigger hold off: " << convertToString(scope.getTriggerHoldoff()) << "s" << std::endl;
	std::cout << "Trigger status: " << scope.getEnumString(scope.getTriggerStatus()) << std::endl;
	std::cout << "Trigger edge: " << convertToString(scope.getEdgeTriggerSlope()) << std::endl << std::endl;

	Channel chan = CH1;	

	while(true) {
		std::cout << "Channel " << convertToString(chan) << " enabled: " << convertToString(scope.getChannelEnable(chan)) << std::endl;
		std::cout << "Channel " << convertToString(chan) << " memory depth: " << convertToString(scope.getMemDepth(chan)) << " points" << std::endl;
		std::cout << "Channel " << convertToString(chan) << " v/div: " << convertToString(scope.getVoltScale(chan)) << " V/div" << std::endl;
		std::cout << "Channel " << convertToString(chan) << " voltage offset: " << convertToString(scope.getVoltOffset(chan)) << "V" << std::endl;
		std::cout << "Channel " << convertToString(chan) << " Coupling: " << scope.getCoupling(chan) << std::endl << std::endl;
		if(chan == CH2) break;
		chan = CH2;
	}

	/*scope.setSerialSpeed(Baud_38400);
	temp = scope.getLongData(CH1);

	for(size_t i = 0; i != temp.size(); i++)
		std::cout << temp[i] << std::endl;*/

	scope.setKeyLock(false);
	return 0;

}
