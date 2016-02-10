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

                //extract process name
                StringRef name(it->begin(), 16);

                // extract pid
                auto begin = it->begin();
                StringRef pid(begin + 17, 5);

                //extract system call id
                StringRef syscall(begin + 62,3);

                // extract system call parameters (rest of the line)
                auto param_it = begin+63;
                StringRef args(param_it, it->end() - param_it);

                data.append(name, pid, syscall, args);
            }
        }
    }
};
#endif //HIGHPERFORMANCELINUXSENSORS_VERBOSE_COLLECTOR_H
