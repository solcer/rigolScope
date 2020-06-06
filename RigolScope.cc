#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <map>
#include <stdio.h>
#include <pthread.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "RigolScope.hh"

template <class T>
inline std::string convertToString(const T& t) {

	std::stringstream ss;
	ss << t;
	return ss.str();

}

inline float convertToFloat(std::string const& s) {

	std::istringstream i(s);
	float x;
	if (!(i >> x))
		throw std::out_of_range("");
	return x;

}

inline size_t convertToSizeT(std::string const& s) {

	std::istringstream i(s);
	size_t x;
	if (!(i >> x))
		throw std::out_of_range("");
	return x;

}

inline double convertToDouble(std::string const& s) {

	std::istringstream i(s);
	double x;
	if (!(i >> x))
		throw std::out_of_range("");
	return x;

}

RigolScope::RigolScope(std::string device, Baud_rate rate) : io_(), port_(io_), timer_(io_), 
				timeout_(boost::posix_time::seconds(2)), address_(device) {

	channel_string_[CH1] = "CHAN1";
	channel_string_[CH2] = "CHAN2";

	trigger_mode_string_[Edge] = "EDGE";
	trigger_mode_string_[Pulse] = "PULSE";
	trigger_mode_string_[Video] = "VIDEO";
	trigger_mode_string_[Slope] = "SLOPE";
	trigger_mode_string_[Pattern] = "PATTERN";
	trigger_mode_string_[Duration] = "DURATION";
	trigger_mode_string_[Alternation] = "ALTERNATION";

	trigger_source_string_[Source_CH1] = "CH1";
	trigger_source_string_[Source_CH2] = "CH2";
	trigger_source_string_[Source_Ext] = "EXT";
	trigger_source_string_[Source_Acline] = "ACLINE";

	trigger_sweep_string_[Sweep_auto] = "AUTO";
	trigger_sweep_string_[Sweep_normal] = "NORMAL";
	trigger_sweep_string_[Sweep_single] = "SINGLE";

	trigger_coupling_string_[Trig_DC] = "DC";
	trigger_coupling_string_[Trig_AC] = "AC";
	trigger_coupling_string_[Trig_HF] = "HF";
	trigger_coupling_string_[Trig_LF] = "LF";

	trigger_status_string_[Run] = "RUN";
	trigger_status_string_[Stop] = "STOP";
	trigger_status_string_[Triggered] = "T'D";
	trigger_status_string_[Wait] = "WAIT";
	trigger_status_string_[Auto] = "AUTO";

	port_.open(address_);
	configureSerial(rate);

	if(port_.is_open()) {
		info_ = getInfo();
	}
	else
		throw std::invalid_argument("Device does not exist");

}

RigolScope::~RigolScope() {

	port_.close();

}

std::string RigolScope::getInfo() {

	write("*IDN?");
	return read();

}

void RigolScope::reset() {

	write("*RST");

}

void RigolScope::setRun(bool val) {

	if(val)
		write(":RUN");
	else
		write(":STOP");

}

std::vector<float> RigolScope::getData(Channel chan) {

	write(":WAV:POIN:MODE NOR");
	write((":WAV:DATA? CHAN" + convertToString(chan)));
	std::string raw_data = read();
	return formatData(raw_data, getVoltOffset(chan), getVoltScale(chan));

}

float RigolScope::getVoltScale(Channel chan) {

	write(":CHAN" + convertToString(chan) + ":SCAL?");
	return convertToFloat(read());

}

void RigolScope::setVoltScale(Channel chan, float scale) {

	if(scale >= 0.002 && scale <= 9000.0)
		write(":CHAN" + convertToString(chan) + ":SCAL " + convertToString(scale));
	else
		throw std::out_of_range("Value out of range");

}

float RigolScope::getVoltOffset(Channel chan) {

	write(":CHAN" + convertToString(chan) + ":OFFS?");
	return convertToFloat(read());

}

void RigolScope::setVoltOffset(Channel chan, float scale) {

	if(scale >= -40000.0 && scale <= 40000.0)
		write(":CHAN" + convertToString(chan) + ":OFFS " + convertToString(scale));
	else
		throw std::out_of_range("Value out of range");

}

float RigolScope::getTimescale() {

	write(":TIM:SCAL?");
	return convertToFloat(read());

}

void RigolScope::setTimescale(float timescale) {

	if(timescale >= 0.000000002 && timescale <= 50.0)
		write(":TIM:SCAL " + convertToString(timescale));
	else
		throw std::out_of_range("Value out of range");

}

float RigolScope::getTimeOffset() {

	write(":TIM:OFFS?");
	return convertToFloat(read());

}

void RigolScope::setTimeOffset(float time_offset) {

	if(time_offset >= -300.0 && time_offset <= 300.0)
		write(":TIM:OFFS " + convertToString(time_offset));
	else
		throw std::out_of_range("Value out of range");

}

size_t RigolScope::getMemDepth(Channel chan) {

	write(":CHAN" + convertToString(chan) + ":MEMD?");
	return convertToSizeT(read());

}

int RigolScope::getAttenuation(Channel chan) {

	write(":CHAN" + convertToString(chan) + ":PROBE?");
	return convertExponent(read(), 0);
	
}

void RigolScope::setAttenuation(Channel chan, int attenuation) {

	if(attenuation >= 1 && attenuation <= 1000)
		write(":CHAN" + convertToString(chan) + ":PROBE " + convertToString(attenuation));
	else
		throw std::out_of_range("Value out of range");

}

std::string RigolScope::getCoupling(Channel chan) {

	write(":CHAN" + convertToString(chan) + ":COUPLING?");
	return read();
	
}

void RigolScope::setCoupling(Channel chan, std::string coupling) {

	if(coupling == "AC" || coupling == "DC" || coupling == "GND")
		write(":CHAN" + convertToString(chan) + ":PROBE " + coupling);
	else
		throw std::out_of_range("Value out of range");

}

void RigolScope::setKeyLock(bool val) {

	if(val)
		write(":KEY:LOCK ENABLE");
	else
		write(":KEY:LOCK DISABLE");

}

void RigolScope::setTriggerForce() {

	write(":FORCE");

}

void RigolScope::setAuto() {

	write(":AUTO");

}

bool RigolScope::getChannelEnable(Channel chan) {

	write(":CHAN" + convertToString(chan) + ":DISP?");
	if(read() == "ON")
		return true;
	else
		return false;

}

void RigolScope::setChannelEnable(Channel chan, bool val) {

	if(val == true)
		write(":CHAN" + convertToString(chan) + ":DISP ON");
	else
		write(":CHAN" + convertToString(chan) + ":DISP OFF");	

}

void RigolScope::setFreqCounter(bool val) {

	if(val)
			write(":COUNter:ENABle ON");
	else
		write(":COUNter:ENABle OFF");

}

bool RigolScope::getFreqCounterEnable() {

	write(":COUNter:ENABle?");
	
	if(read() == "ON")
		return true;
	else
		return false;

}

float RigolScope::getFreqCounterValue() {

	write(":COUNter:VALue?");
	return convertExponent(read(), 5);

}

Trigger_mode RigolScope::getTriggerMode() {

	write(":TRIG:MODE?");

	try {
		return (Trigger_mode)trigger_mode_string_[read()];
	}
	catch(std::out_of_range) {
		throw std::out_of_range("Scope returned something unexpected");
	}

}

void RigolScope::setTriggerMode(Trigger_mode mode) {
	
	switch(mode) {
		case Edge:
		case Pulse:
		case Video:
		case Slope:
		case Pattern:
		case Duration:
		case Alternation:
			write(":TRIG:MODE " + trigger_mode_string_[mode]);
			break;
		default:
			throw std::out_of_range("Value out of range");
	}
}

Trigger_source RigolScope::getTriggerSource(Trigger_mode mode) {
	
	switch(mode) {
		case Edge:
		case Pulse:
		case Video:
		case Slope:
		case Pattern:
		case Duration:
		case Alternation:
			write(":TRIG:" + trigger_mode_string_[mode] + ":SOUR?");
			return (Trigger_source)trigger_source_string_[read()];
		default:
			throw std::out_of_range("Value out of range");
	}

}

void RigolScope::setTriggerSource(Trigger_mode mode, Trigger_source source) {

	switch(mode) {
		case Edge:
		case Pulse:
			if(source >= Source_Acline)
				break;
		case Video:
			if(source >= Source_Ext)
				break;
		case Slope:
			if(source >= Source_Ext)
				break;
			write(":TRIG:" + trigger_mode_string_[mode] + ":SOUR " + trigger_source_string_[source]);
			return;
		default:
			throw std::out_of_range("Value out of range");
	}
	throw std::out_of_range("Value out of range");

}

float RigolScope::getTriggerLevel(Trigger_mode mode) {

	switch(mode) {
		case Edge:
		case Pulse:
		case Video:
			write(":TRIG:" + trigger_mode_string_[mode] + ":LEV?");
			return convertToFloat(read());
		default:
			throw std::out_of_range("Incorrect mode");
	}

}

void RigolScope::setTriggerLevel(Trigger_mode mode, float level) {

	float scale = 0;

	if(getTriggerSource(mode) == Source_CH1)
		scale = getVoltScale(CH1);
	else if(getTriggerSource(mode) == Source_CH2)
		scale = getVoltScale(CH2);
	else if(getTriggerSource(mode) == Source_Ext)
		scale = 0.2;

	if(scale == 0)
			throw std::out_of_range("Value out of range");

	if(level >= -6.0*scale && level <= 6.0*scale)
		write(":TRIG:" + trigger_mode_string_[mode] + ":LEV " + convertToString(level));
	else
		throw std::out_of_range("Value out of range");

}

Trigger_sweep RigolScope::getTriggerSweep(Trigger_mode mode) {

	switch(mode) {
		case Edge:
		case Pulse:
		case Slope:
		case Pattern:
		case Duration:
			write(":TRIG:" + trigger_mode_string_[mode] + ":SWE?");
			return (Trigger_sweep)trigger_sweep_string_[read()];
		default:
			throw std::out_of_range("Value out of range");
	}

}

void RigolScope::setTriggerSweep(Trigger_mode mode, Trigger_sweep sweep) {

	switch(mode) {
		case Edge:
		case Pulse:
		case Slope:
		case Pattern:
		case Duration:
			write(":TRIG:" + trigger_mode_string_[mode] + ":SWE " + trigger_sweep_string_[sweep]);
			return;
		default:
			throw std::out_of_range("Value out of range");
	}

}

Trigger_coupling RigolScope::getTriggerCoupling(Trigger_mode mode) {

	write(":TRIG:" + trigger_mode_string_[mode] + ":COUP?");
	return (Trigger_coupling)trigger_coupling_string_[read()];

}

void RigolScope::setTriggerCoupling(Trigger_mode mode, Trigger_coupling coupling) {

	switch(coupling) {
		case Trig_DC:
		case Trig_AC:
		case Trig_HF:
			write(":TRIG:" + trigger_mode_string_[mode] + ":COUP " + trigger_coupling_string_[coupling]);
			return;
		case Trig_LF:
			if(mode == Edge || mode == Pulse || mode == Slope) {
				write(":TRIG:" + trigger_mode_string_[mode] + ":COUP " + trigger_coupling_string_[coupling]);
				return;
			}
			else
				throw std::out_of_range("Value out of range");
	}

}

float RigolScope::getTriggerHoldoff() {

	write(":TRIG:HOLD?");
	return convertExponent(read(), 3);

}

void RigolScope::setTriggerHoldoff(float hold_off) {

	if(hold_off <= 1.5 && hold_off >= 0.0000005)
		write(":TRIG:HOLD " + convertToString(hold_off));
	else
		throw std::out_of_range("Value out of range");

}

Trigger_status RigolScope::getTriggerStatus() {

	write(":TRIG:STATUS?");
	return (Trigger_status)trigger_status_string_[read()];

}

void RigolScope::setTriggerHalf() {

	write(":TRIG%50");
	return;

}

bool RigolScope::getEdgeTriggerSlope() {

	write(":TRIG:EDGE:SLOP?");
	if(read() == "POSITIVE")
		return true;
	else
		return false;

}

void RigolScope::setEdgeTriggerSlope(bool slope) {

	if(slope)
		write(":TRIG:EDGE:SLOPE POSITIVE");
	else
		write(":TRIG:EDGE:SLOPE NEGATIVE");

}

std::vector<float> RigolScope::getLongData(Channel chan) {

	setRun(false);
	sleep(1);
	write(":WAVEFORM:POINTS:MODE MAXIMUM");

	write(":WAVEFORM:DATA? " + convertToString(chan));
	std::string raw_data = read();
	return formatData(raw_data, getVoltOffset(chan), getVoltScale(chan));
	
}

void RigolScope::setSerialSpeed(Baud_rate rate) {

	std::cout << ":RS232:BAUD " << convertToString(rate) << std::endl;
	write(":RS232:BAUD " + convertToString(rate));
	write(":RS232:BAUD " + convertToString(rate));
	//port_.open(address_);
	//std::cout << address_ << std::endl;
	
	//configureSerial(rate);
	//port_.close();
	//sleep(1);
	//port_.open(address_);

	configureSerial(rate);

	while(true) {
		try{
			//write(":RS232:BAUD " + convertToString(rate));
			write("*IDN?");
			std::cout << read() << std::endl;
		}
		catch(timeout_exception) {
			std::cout << "No answer" << std::endl;
		}
	}
	/*std::string temp = "";
	while(temp == "") {
		temp = read();
		std::cout << "vittuuu" << std::endl;
		std::cout << temp << std::endl;
		write("*IDN?");
		sleep(2);
	}*/
	
}

std::string RigolScope::getEnumString(Channel chan) {

	return channel_string_[chan];

}

std::string RigolScope::getEnumString(Trigger_mode mode) {

	return trigger_mode_string_[mode];

}

std::string RigolScope::getEnumString(Trigger_source source) {

	return trigger_source_string_[source];

}

std::string RigolScope::getEnumString(Trigger_sweep sweep) {

	return trigger_sweep_string_[sweep];

}

std::string RigolScope::getEnumString(Trigger_coupling coupling) {

	return trigger_coupling_string_[coupling];

}

std::string RigolScope::getEnumString(Trigger_status status) {

	return trigger_status_string_[status];

}

void RigolScope::write(std::string command) {

	command += "\n";
	boost::asio::write(port_, boost::asio::buffer(command.c_str(), command.size()));

}

std::string RigolScope::read() {

	boost::asio::async_read_until(port_, streambuffer_, "\n", boost::bind(&RigolScope::readCompleted, 
			this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

	if(timeout_ != boost::posix_time::seconds(0)) {
		timer_.expires_from_now(timeout_);
		timer_.async_wait(boost::bind(&RigolScope::timeoutExpired, this, boost::asio::placeholders::error));
	}

	result_ = resultInProgress;
	bytes_transferred_ = 0;

	for(;;) {

		io_.run_one();
		switch(result_) {
			case resultSuccess: {
				timer_.cancel();
				bytes_transferred_ -= 1;
				std::istream is(&streambuffer_);
				std::string result(bytes_transferred_,'\0');
				is.read(&result[0], bytes_transferred_);
				is.ignore(1);
				return result;
			}
			case resultTimeoutExpired:
				port_.cancel();
				throw(timeout_exception("Timeout expired"));
			case resultError:
				timer_.cancel();
				port_.cancel();
				throw(boost::system::system_error(boost::system::error_code(), "Error while reading"));
			case resultInProgress:;
		}
	}
}

void RigolScope::readCompleted(const boost::system::error_code& error, const size_t bytes_transferred_) {

	if(error) {
		if(error != boost::asio::error::operation_aborted) {
			result_ = resultError;
		}
	} 
	else {
		if(result_ != resultInProgress) return;
		result_ = resultSuccess;
		this->bytes_transferred_ = bytes_transferred_;
	}
}

void RigolScope::timeoutExpired(const boost::system::error_code& error) {

	if(result_ != resultInProgress) return;

	if(error != boost::asio::error::operation_aborted) {
		result_ = resultTimeoutExpired;
	}

}

void RigolScope::setSerialTimeout(const boost::posix_time::time_duration& duration) {

	timeout_ = duration;

}

std::vector<float> RigolScope::formatData(std::string raw_data, float volt_offset, float volt_scale) {

	std::vector<float> data;

	for(size_t i = 0; i != raw_data.length(); i++) {
		int temp = raw_data[i];
		data.push_back((((temp*(-1)+255)-130.0-(volt_offset/volt_scale*25))/25*volt_scale));   // do some data scaling magic
	}
	return data;
}

float RigolScope::convertExponent(std::string number, size_t decimals) {

		float base = 0.0;	

		if(decimals == 0)
			base = convertToFloat(number.substr(0, (decimals + 1)));
		else
			base = convertToFloat(number.substr(0, (decimals + 2)));

		float exp = convertToDouble(number.substr((decimals + 4), 2));

		if(number.substr((decimals+3), 1) == "-")
			exp = -1.0*exp;

		return base*pow(10, exp);

}
void RigolScope::configureSerial(Baud_rate rate) {

	port_.set_option(boost::asio::serial_port_base::baud_rate((int)rate));
	port_.set_option(boost::asio::serial_port_base::character_size(8));
	port_.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
	port_.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
	port_.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

}
