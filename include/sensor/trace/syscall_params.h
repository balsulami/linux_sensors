//
// Created by root on 9/15/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_VERBOSE_COLLECTOR_H
#define HIGHPERFORMANCELINUXSENSORS_VERBOSE_COLLECTOR_H


#include "utils/string_ref.h"
#include "sensor/data_aggregator.h"

template<typename T>
class SyscallsParams {
public:
    void collect(std::vector<StringRef> & lines,DataAggregator<T> & data) {
        for (auto it = lines.begin(); it != lines.end(); it++) {
            if (it->size() > 60) {

                auto begin = it->begin();
                StringRef pid(begin + 17, 5);

                auto name_it = it->begin();

                while (*name_it == ' ')++name_it;
                StringRef name(name_it, 16 - (name_it - begin));

                StringRef time(begin + 34, 13);

                auto syscall_it = begin + 63;
                while (*syscall_it++ != ' ');
                StringRef syscall(begin + 63, syscall_it - (begin + 63));

                StringRef args(syscall_it, it->end() - syscall_it);

                data.append(name, pid, syscall, args);
            }
        }
    }
};
#endif //HIGHPERFORMANCELINUXSENSORS_VERBOSE_COLLECTOR_H
