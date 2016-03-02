

#pragma once

#include"base_stream.h"
#include<fstream>
#include<vector>
#include<list>
#include<unordered_map>
using namespace std::chrono;
template<class T>
class file_aggr_stream : public sensor_stream<T>{
public:
	file_aggr_stream(const char * name,int size=100) : _file(name),_size(size){

	}

	virtual void dispatch(std::vector<T> & records){
		std::string json;
		json.reserve(20 * records.size());
		for (auto iter = records.begin(); iter != records.end(); iter++){
			auto it = traces.find(iter->_pid);
			if (it != traces.end()) {
				it->second.push_back(iter->_syscall);
			}
			else{
				std::vector<syscall_t> sys_traces;
				sys_traces.reserve(100);
				sys_traces.push_back(iter->_syscall);
				traces.emplace(iter->_pid,std::move(sys_traces));
			}

			// add process name
			auto it2 = pnames.find(iter->_pid);
			if (it2 == pnames.end())
				pnames.emplace(iter->_pid,iter->_name);
		}
		if(traces.size() >= _size)
			print_all();
	}

	void print_all(){
		milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
		_file<<"processes = "<<traces.size()<<" timestamp = "<<ms.count()<<"\n";
		for (auto iter = traces.begin();iter != traces.end();iter++) {
			auto it2 = pnames.find(iter->first);
			_file << it2->second << " == " << iter->first << " ==> ";
			for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++)
				_file << *iter2 << ",";
			_file << "\n";
		}
		traces.clear();
		pnames.clear();
		_file.flush();
	}

	~file_aggr_stream(){
		_file.close();
	}

private:
	std::ofstream _file;
	std::unordered_map<pid_t,std::vector<syscall_t>> traces;
	std::unordered_map<pid_t,std::string> pnames;
	int _size ;
};