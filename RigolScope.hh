#ifndef RIGOLSCOPE_HH
#define RIGOLSCOPE_HH

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <map>
#include <boost/asio.hpp>

enum Channel {CH1 = 1, CH2};
enum Trigger_mode {Edge, Pulse, Video, Slope, Pattern, Duration, Alternation};
enum Trigger_source {Source_CH1, Source_CH2, Source_Ext, Source_Acline};
enum Trigger_sweep {Sweep_auto, Sweep_normal, Sweep_single};
enum Trigger_coupling {Trig_DC, Trig_AC, Trig_HF, Trig_LF};
enum Trigger_status {Run, Stop, Triggered, Wait, Auto};
enum Baud_rate {Baud_300 = 300, Baud_2400 = 2400, Baud_4800 = 4800, Baud_9600 = 9600, Baud_19200 = 19200, Baud_38400 = 38400};

//! \todo{Doxygen spec on exceptions}
//! \todo{Binary read?}
//! \todo{USB support}
//! \todo{Add const everywhere}
//! \todo{check setTriggerMode() on how to make the functions with strings}
//! \todo{typedef for trigger modes!!}
//! \todo{Fix convertexponent for negative numbers!}
//! \todo{think on this trigger setup}

//! Little helper struct, for mapping strings coming from the scope to ints used inside the code, and vice versa
template <class T>
struct Bidirectional_map {

	std::vector<std::string> strings;

	std::string& operator[](const T& t) {
		try {
			return strings.at(t);
		}
		catch(std::out_of_range) {
			strings.push_back("");
			return strings.back();
		}
	}

	int operator[](std::string string) {
		for(size_t i = 0; i != strings.size(); ++i) {
			if(strings[i] == string) return i;
		}
		throw std::out_of_range("Value does not exist");
	}

};

class timeout_exception: public std::runtime_error {

public:
	timeout_exception(const std::string& arg): runtime_error(arg) {}

};

//! \note{This class uses enums internally (for sanitizing input and making sure you can't input anything incorrect), 
//! use getEnumString() functions if you want the enums as strings (for printing in software etc.)}
class RigolScope {
public:

//! Constructor for a RigolScope object
//! @param device Address of the device (ie. "/dev/ttyUSB0" for example)
	RigolScope(std::string device, Baud_rate rate);

	~RigolScope();

//! Gets the manufacturer, model number, serial number and the version of the firmware from the scope ("*IDN?" command)
//! @return Scope information
	std::string getInfo();

//! Reset the scope ("*RST" command)
	void reset();

//! Put the scope in RUN or STOP mode (":RUN" and ":STOP" commands)
//! @param val true for RUN and false for STOP
	void setRun(bool val);

//! Gets raw data from the scope, returns 600 points of data in mode ":WAVEFORM:POINTS:MODE NORMAL"
//! @param chan Number of channel (values CH1 or CH2)
//! @return Scaled data points as volts
	std::vector<float> getData(Channel chan);

//! Gets v/div
//! @param chan Number of channel (values CH1 or CH2)
//! @return v/div as volts
	float getVoltScale(Channel chan);

//! Set the v/div
//! @param chan Number of channel (values CH1 or CH2)
//! @param scale v/div value as volts, has to be between 2mV (value 0.002) and 10kV (value 10000.0)
	void setVoltScale(Channel chan, float scale);

//! Get the voltage offset
//! @param chan Number of channel (values CH1 or CH2)
//! @return Voltage offset as volts
	float getVoltOffset(Channel chan);

//! Function to set the voltage offset on channel chan
//! @param chan Number of channel (values CH1 or CH2)
//! @param offset Voltage offset as a float value in volts, has to be between -40kV (value -40000.0) and 40kV (value 40000.0)
	void setVoltOffset(Channel chan, float offset);

//! Get the t/div
//! @return t/div as seconds
	float getTimescale();

//! Function to set the timescale
//! @param timescale Timescale as a float value in seconds, has to be between 2ns (value 0.000000002) and 50s (value 50.0)
	void setTimescale(float timescale);

//! Get the time offset
//! @return Time offset as seconds
	float getTimeOffset();

//! Function to set the time offset
//! @param time_offset Time offset as a float value in seconds, has to be between -300s (value -300.0) and 300s (value 300.0)
	void setTimeOffset(float time_offset);

//! Get the memory depth
//! @param chan Number of channel (values CH1 or CH2)
//! @return Memory depth as bytes (8bit values)
	size_t getMemDepth(Channel chan);

//! Get the probe attenuation
//! @param chan Number of channel (values CH1 or CH2)
//! @return Attenuation (value "10" for 10x probe etc.)
	int getAttenuation(Channel chan);

//! Function to set the attenuation on channel chan
//! @param chan Number of channel (values CH1 or CH2)
//! @param attenuation Attenuation as an int value, has to be between 1 and 1000
	void setAttenuation(Channel chan, int attenuation);

//! Get the coupling type
//! @param chan Number of channel (values CH1 or CH2)
//! @return Coupling type ("AC", "DC" or "GND")
	std::string getCoupling(Channel chan);

//! Function to set the coupling mode in channel chan
//! @param chan Number of channel (values CH1 or CH2)
//! @param coupling Coupling mode as a string value, has to be "AC", "DC" or "GND"
	void setCoupling(Channel chan, std::string coupling);

//! Lock the keys on the scope. Keys will get locked every time you send a command to the scope
//! @param val true for locked keys, false for unlocked
	void setKeyLock(bool val);

//! Force trigger
	void setTriggerForce();

//! Autoconfigure scope (":AUTO" command)
	void setAuto();

//! Check if channel is enabled (=visible, ":CHAN<number>:DISP?" command)
//! @param chan Number of channel (values CH1 or CH2)
//! @return true if channel is enabled, false if disabled
	bool getChannelEnable(Channel chan);

//! Set if channel is enabled (=visible, ":CHAN<number>:DISP ON/OFF" command)
//! @param chan Number of channel (values CH1 or CH2)
//! @param val true if channel is enabled, false if disabled
	void setChannelEnable(Channel chan, bool val);

//! Enable or disable the frequency counter on the scope (":COUN:ENAB ON/OFF" command)
//! @param val true for enabled, false for disabled
	void setFreqCounter(bool val);

//! Check if frequency counter is enabled or disabled (":COUN:ENAB?" command)
//! @return true for enabled, false for disabled
	bool getFreqCounterEnable();

//! Get the frequency counter value (":COUN:VALUE?" command)
//! @return Frequency in herz
	float getFreqCounterValue();

//! Get the trigger mode (":TRIG:MODE?" command)
//! @return Trigger mode, values: Edge, Pulse, Video, Slope, Pattern, Duration, Alternation
	Trigger_mode getTriggerMode();
	
//! Set the trigger mode (":TRIG:MODE" command)
//! @param mode Trigger mode, values: Edge, Pulse, Video, Slope, Pattern, Duration, Alternation
	void setTriggerMode(Trigger_mode mode);

//! Get the trigger source (":TRIG:<mode>:SOUR?" command)
//! @param mode Trigger mode, values: Edge, Pulse, Video, Slope, Pattern, Duration, Alternation
//! @return Trigger source
	Trigger_source getTriggerSource(Trigger_mode mode);

//! Set the trigger source (":TRIG:<mode>:SOUR CHAN1/CHAN2/EXT/ACLINE" command)
//! @param mode Trigger mode, values: Edge, Pulse, Video, Slope, Pattern, Duration, Alternation
//! @param source Trigger source
	void setTriggerSource(Trigger_mode mode, Trigger_source source);

//! Get current trigger level (":TRIG:<mode>:LEV?" command)
//! @param mode Trigger mode, values: Edge, Pulse, Video
//! @return Trigger level
	float getTriggerLevel(Trigger_mode mode);

//! Set the trigger level (":TRIG:<mode>:LEV" command)
//! @param mode Trigger mode, values: Edge, Pulse, Video
//! @param level Trigger level, value has to be between -6*voltage scale 
//! and 6*voltage scale if trigger source is channel 1 or 2, and between -1.2 and 
//!	1.2 if trigger source is external. Exception will be thrown if the trigger source is Acline.
	void setTriggerLevel(Trigger_mode mode, float level);

//! Get the trigger sweep type (":TRIG:<mode>:SWE?" command)
//! @param mode Trigger mode, values: Edge, Pulse, Slope, Pattern, Duration
//! @return Trigger sweep type, values Auto, Normal and Single
	Trigger_sweep getTriggerSweep(Trigger_mode mode);

//! Set the trigger sweep type (":TRIG:<mode>:SWE" command)
//! @param mode Trigger mode, values: Edge, Pulse, Slope, Pattern, Duration
	void setTriggerSweep(Trigger_mode mode, Trigger_sweep sweep);

//! Get the trigger coupling type (":TRIG:<mode>:COUP?" command)
//! @param mode Trigger mode, values: Edge, Pulse, Video, Slope, Pattern, Duration, Alternation
//! @return Trigger coupling
	Trigger_coupling getTriggerCoupling(Trigger_mode mode);

//! Set the trigger coupling type (":TRIG:<mode>:COUP" command)
//! @param mode Trigger mode, values: Edge, Pulse, Video, Slope, Pattern, Duration, Alternation
//! @param coupling Trigger coupling type, values: Trig_DC, Trig_AC, Trig_HF, Trig_LF
	void setTriggerCoupling(Trigger_mode mode, Trigger_coupling coupling);

//! Get the trigger hold off value (":TRIG:<mode>:HOLD?" command)
//! @param mode Trigger mode, values: Edge, Pulse, Video, Slope, Pattern, Duration, Alternation
//! @return Holdoff value
	float getTriggerHoldoff();

//! Set the trigger hold off value (":TRIG:<mode>:HOLD" command)
//! @param mode Trigger mode, values: Edge, Pulse, Video, Slope, Pattern, Duration, Alternation
//! @param hold_off Holdoff value, has to be between 1.5s and 500ns, values: 1.5 and 0.0000005
	void setTriggerHoldoff(float hold_off);

//! Get the trigger status (":TRIG:STATUS?" command)
//! @return Trigger status, values: Run, Stop, Triggered, Wait or Auto
	Trigger_status getTriggerStatus();

//! Set the trigger level to the vertical midpoint of amplitude (":TRIG%50" command)
	void setTriggerHalf();

//! Get the slope setting on the Edge trigger mode (":TRIG:EDGE:SLOP?" command)
//! @return true if the scope triggers on rising edge, and false if the scope triggers on the falling edge
	bool getEdgeTriggerSlope();

//! Set the slope setting on the Edge trigger mode (":TRIG:EDGE:SLOPE" command)
//! @param slope true if the scope triggers on rising edge, and false if the scope triggers on the falling edge
	void setEdgeTriggerSlope(bool slope);

//! Helper function for converting enums to a string you can print, overloaded for all the enum types used.
//! \todo{These can be made in some neater way I'm sure, maybe with a template and traits...?}
//! @param chan Number of channel, values: CH1 or CH2
//! @return Channel as a string, values: "CH1" or "CH2"
	std::string getEnumString(Channel chan);
//! @param mode Trigger mode, values: Edge, Pulse, Video, Slope, Pattern, Duration, Alternation
//! @return Trigger mode as a string, values: "EDGE", "PULSE", "VIDEO", "SLOPE", "PATTERN", "DURATION", "ALTERNATION"
	std::string getEnumString(Trigger_mode mode);
//! @param source Trigger source, values: Chan1, Chan2, Ext, Acline
//! @return Trigger source, values: "CH1", "CH2", "EXT", "ACLINE"
	std::string getEnumString(Trigger_source source);
//! @param sweep Sweep type, values: Auto, Normal and Single
//! @return Sweep type, values "AUTO", "NORMAL", "SINGLE"
	std::string getEnumString(Trigger_sweep sweep);
//! @param coupling Coupling type, values: Trig_DC, Trig_AC, Trig_HF, Trig_LF
//! @return Coupling type, values: "DC", "AC", "HF", "LF"
	std::string getEnumString(Trigger_coupling coupling);
//! @param status Trigger status
//! @return Trigger status, values: "RUN", "STOP", "T'D", "WAIT", "AUTO"
	std::string getEnumString(Trigger_status status);

//! Get long data from the channel, 2048 points of data. First acquisition is stopped, then one second wait,
//! then ":WAVEFORM:POINTS:MODE MAXIMUM" is configured, then acquisition gets started, 
//! then the function waits for the scope to be triggered and then it retrieves data.
//! \note{Long mem does NOT work, I have no idea why, it just hangs and does not give a single value}
//! \note{Trigger has to be in single sweep mode, otherwise you will just get 600 points of data!}
//! @param chan Number of channel (values CH1 or CH2)
//! @return Scaled data points as volts
	std::vector<float> getLongData(Channel chan);

//! Function to set the serial speed connection rate
//! @param rate Baud rate, accepted values listed in the enum list in the beginning of the class
//! \todo{Does not work, baud rate change command is not received before set_option modifies serial port buffer contents}
	void setSerialSpeed(Baud_rate rate);

//! Function to set the serial port timeout
//! @param t Timeout as a time_duration object, create with "boost::posix_time::seconds(1)" for example
//! \note{Does not work with timeout as zero, it will hang!}
	void setSerialTimeout(const boost::posix_time::time_duration& duration);

private:

	Bidirectional_map<Channel> channel_string_;
	Bidirectional_map<Trigger_mode> trigger_mode_string_;
	Bidirectional_map<Trigger_source> trigger_source_string_;
	Bidirectional_map<Trigger_sweep> trigger_sweep_string_;
	Bidirectional_map<Trigger_coupling> trigger_coupling_string_;
	Bidirectional_map<Trigger_status> trigger_status_string_;

	std::string info_;

//! Private variables related to serial port communication
	enum ReadResult {resultInProgress, resultSuccess, resultError, resultTimeoutExpired };
	enum ReadResult result_;
	boost::asio::io_service io_;
	boost::asio::serial_port port_;
    boost::asio::deadline_timer timer_;
	boost::posix_time::time_duration timeout_;
	boost::asio::streambuf streambuffer_;
	size_t bytes_transferred_;
	std::string address_;

//! Function for writing to the scope
//! \note{Appends line end ("\n") to the command automatically}
//! @param command Command to be sent to the scope
	void write(std::string command);

//! Function for reading from the scope
	std::string read();

//! Internal function for handling asynchronous read timeouts
//! @param error Error object
	void timeoutExpired(const boost::system::error_code& error);

//! Internal function for handling a completed read
//! @param error Error object
//! @param bytesTransferred Amount of bytes transferred
	void readCompleted(const boost::system::error_code& error, const size_t bytesTransferred);

//! Function for configuring the (computers) serial interface
//! @param rate Serial port baud rate
	void configureSerial(Baud_rate rate);

//! Internal function for scaling/formatting the raw data from the scope
//! @param raw_data Raw data read from the scope as big std::string clump
//! \todo{Change to a binary data format + binary read}
//! @param volt_offset Voltage offset of the channel where the data was read from
//! @param volt_scale v/div of the channel where the data was read from
//! @return Formatted and scaled data
	std::vector<float> formatData(std::string raw_data, float volt_offset, float volt_scale);

//! Internal function for converting data from "1.00000e+03" format to a float value
//! @param decimals Number of decimals in the string format
//! @return Converted value
	float convertExponent(std::string number, size_t decimals);

//! Disable copying and assignment
	RigolScope(const RigolScope&);
	void operator=(const RigolScope&);

};
#endif
