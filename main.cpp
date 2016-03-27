

#include "utils/types.h"
#include <iostream>
#include <fstream>
#include"sensor/single_sensor.h"
#include"sensor/multithreaded_sensor.h"
#include "stream/dumy_stream.h"
#include "stream/file_stream.h"
#include "stream/socket_stream.h"
#include "sensor/trace/sys_params_record.h"

typedef SysParamsRecord Record ;
#include "utils/linux_daemon.h"

template<typename T>
sensor_stream<T> * setup_stream(std::vector<std::string> & args){
    auto iter = std::find(args.begin(), args.end(), "-h");
	if (iter != args.end()){
		std::string value = *(iter + 1);
		auto split = value.find(':');

		std::string hostname = value.substr(0, split);
		std::string port = value.substr(split + 1);
		return new socket_stream<T>("sensor1", hostname.c_str(), std::atoi(port.c_str()));
	}
    iter = std::find(args.begin(), args.end(), "-o");
    if (iter != args.end()){
        return new file_stream<T>((iter + 1)->c_str());
    }
	iter = std::find(args.begin(), args.end(), "-a");
	if (iter != args.end()){
		// only SysRecord is supported
		return new file_aggr_stream<T>((iter + 1)->c_str());
	}
    return new dumy_stream<T>();
}

int main(int argc, char ** argv) {
	std::string sensor_name = "syscall_sensor";
	std::string run_path = "/var/run";

	std::vector<std::string> args;
	std::transform(argv + 1, argv + argc, std::back_inserter(args), [](char *arg) { return std::string(arg); });

	// Reteriving the pid of the last running sensor
	int pid = get_sensor_pid(sensor_name, run_path);
	// check if the sensor is already running
	if (kill(pid, 0) == ESRCH)
		// the pid exists but the process is not running
		// remove the pid file
		remove_sensor_pid(sensor_name, run_path);

	std::vector<std::string>::iterator iter;
	if ((iter = std::find(args.begin(), args.end(), "stop")) != args.end()) {
		if (pid != -1) {
			// pass the command stop
			std::cout << "Stopping the daemon process pid= " << pid << "\n";
			//write_to(current_dir+"/.cmd","stop",false);
			remove_sensor_pid(sensor_name, run_path);
			kill_sensor(pid);
			return 0;
		} else {
			std::cout << "The sensor is NOT running.\n";
			return 0;
		}
	}
	else if ((iter = std::find(args.begin(), args.end(), "status")) != args.end()) {
		if (pid != -1) {
			// pass the command stop
			std::cout << "Gathering info about the sensor pid= " << pid << "\n";
				iter = std::find(args.begin(), args.end(), "-h");
				if (iter == args.end()) {
					// debug mode
					// measure how many events the Linux generates
					std::cout << "You must specify the address and port to connect to the sensor\n";
					return 0;
				}
				else {
					std::string value = *(iter + 1);
					auto split = value.find(':');

					std::string hostname = value.substr(0, split);
					std::string port = value.substr(split + 1);

					linux_socket socket(hostname.c_str(), std::atoi(port.c_str()));
					if (!socket.connect()) {
						std::cout << "The sensor is not running in logging mode.\n";
						return 0;
					}
					std::string reciv;
					do {
						reciv = socket.receive();
						if (reciv.empty())
							break;
						std::cout << reciv << "\n";
					} while (true);

				}

		} else {
			std::cout << "The sensor is NOT running.\n";
			return 0;
		}
	}
	else if ((iter = std::find(args.begin(), args.end(), "start")) != args.end()) {
		sensor_stream<Record> *stream = setup_stream<Record>(args);
		if (pid ==-1) {
			setup_daemon(sensor_name, run_path);
			iter = std::find(args.begin(), args.end(), "-r");
			if (iter != args.end()) {
				// debug mode
				// measure how many events the Linux generates
			}
			else {
				multithreaded_sensor<Record> linux_sensor(stream);
				linux_sensor.config_events(true, false);
				iter = std::find(args.begin(), args.end(), "-l");
				if (iter != args.end()) {
					linux_sensor.run(true);
				}else{
					linux_sensor.run(false);
				}
			}
		}
		else{
			std::cout << "The sensor is running.\n";
			return 0;
		}
	}
}
