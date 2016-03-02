
#pragma once

#include"utils/format.h"
#include "utils/string_ref.h"
#include<string>
#include "sys_record.h"

class SysParamsRecord : public SysRecord {
public:
	SysParamsRecord() :SysRecord(){}

	SysParamsRecord(StringRef & name, StringRef & pid, StringRef & syscall, StringRef & arg) :
			SysRecord(pid,syscall)
	{
		set_name(name);
//		set_arg(arg);
	}

	void set_name(StringRef & name){
		auto begin = name.begin();
		while(*begin == ' ') ++begin;

		auto end = name.end();
		while(*end == ' ') --end;
		_name = std::string(begin,end);
	}

	void set_arg(StringRef & arg){
		auto begin = arg.begin();
		while(*begin != '(') ++begin;

		auto end = arg.end();
		while(*end != ')') --end;
		_arg = std::string(begin,end+1);
	}

	std::string to_string(){
		std::string json = fmt::format("'{0}','{1}','{2}','{3}'", _name,_pid,_syscall,_arg);
		return json;
	}

	std::string _name;
	std::string _arg;
};