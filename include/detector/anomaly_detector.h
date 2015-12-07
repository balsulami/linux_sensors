//
// Created by root on 9/21/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_ANOMALYDETECTOR_H
#define HIGHPERFORMANCELINUXSENSORS_ANOMALYDETECTOR_H

#include "stream/base_stream.h"
#include "sensor/parallel_worker.h"
#include <algorithm>
#include "utils/types.h"
#include "pipeline_estimator.h"
#include "utils/sys_record.h"
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>

enum EVENT_TYPE {
    Normal, Anomalous
};

template<typename T>
class AnomalyDetector
        : public sensor_stream<T> {
    typedef std::unordered_map<pid_t, std::vector<syscall_t>> PIDTraces;

public:
    AnomalyDetector(int predict_interval = 30, int train_interval = 1, int trace_len = 10)
            : _predict_interval(predict_interval), _train_interval(train_interval), _trace_len(trace_len) {

    }

    virtual void dispatch(std::vector<T> &records) {
        cout << "checking lock in dispatch\n";
        std::lock_guard<std::mutex> lock(mutex);
        for (auto iter = records.begin(); iter != records.end(); iter++) {
            auto it = _current_traces.find(iter->_pid);

            if (it != _current_traces.end()) {
                it->second.push_back(iter->_syscall);
            }
            else {
                std::vector<syscall_t> sys_traces;
                sys_traces.reserve(200);
                sys_traces.push_back(iter->_syscall);
                _current_traces.emplace(iter->_pid, std::move(sys_traces));
            }
        }
    }

    void start() {
        cout << "The detector is listening for " << _train_interval << " mins ... \n";
        //std::this_thread::sleep_for(std::chrono::seconds(20)); //minutes(_train_interval));
//        auto traces = _extract_traces(_current_traces, _trace_len);
        TraceList train_traces;
        std::vector<pid_t > train_pids;
        cout << "Extracting traces for training ... \n";
        _extract_traces(_current_traces,_trace_len,train_pids,train_traces);
        // debug message
        cout << "training the detector ...." << "\n";
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        _detector.train(train_traces);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(t2 - t1).count();
        cout << "****** " << duration << " seconds to train the detector\n";
        _worker.start(AnomalyDetector::_start_detect, this);
//        cout << "The detector is filling up\n";
//        fillup_traces(this);
        cout << "The detector is running\n";
//        _start_detect(this);
////        _worker.start(AnomalyDetector::_start_train, this);
    }

private:
    int _predict_interval, _train_interval;
    int _trace_len;
    pipeline_estimator _detector;
    PIDTraces _current_traces;
    ParallelWorker _worker;
    std::mutex mutex;

    static void _start_detect(AnomalyDetector *ptr) {
        while (true) {
            cout << "The detector is sleeping for 1 sec\n";
            std::this_thread::sleep_for(std::chrono::seconds(ptr->_predict_interval));

            cout << "Extracting traces for predictions ... \n";
            TraceList pred_traces;
            std::vector<pid_t> pred_pids;
            {
                cout << "checking lock in start_detect\n";
                std::lock_guard<std::mutex> lock(ptr->mutex);
                _extract_traces(ptr->_current_traces, ptr->_trace_len, pred_pids, pred_traces);
            }
            PredictLists predictions = ptr->_detector.predict(pred_traces);
            cout << "The detection results are :\n";
            long normal_pids = std::count_if(predictions.begin(),predictions.end(),[](double pred){return pred>0.0;});
            cout<<"\tnormal pids="<<normal_pids<<"\n";
            cout<<"\tanomlous pids="<< (predictions.size()-normal_pids) <<"\n";
        }
    }

    static void _extract_traces(PIDTraces &traces, int min_len, std::vector<pid_t> & extract_pids, std::vector<std::vector<syscall_t>> & extract_traces) {
        size_t min = std::numeric_limits<size_t>::max(), max = std::numeric_limits<size_t>::min(), sum = 0, count = 0;
        for (auto &trace: traces) {
            if (trace.second.size() > min_len) {
                if (trace.second.size() < min)
                    min = trace.second.size();
                if (trace.second.size() > max)
                    max = trace.second.size();
                sum += trace.second.size();
                extract_pids.push_back(trace.first);
                extract_traces.push_back(std::move(trace.second));
            }
        }
        cout << "\t# " << extract_pids.size() << " traces is used" << "\n";
        if(extract_pids.size() > 0) {
            cout << "\tMaximum trace length = " << max << "\n";
            cout << "\tMinimum trace length = " << min << "\n";
            cout << "\tAverage trace length = " << sum / extract_pids.size() << "\n";
            for (auto &pid: extract_pids)
                traces.erase(pid);
        }
        cout << "\t# " << traces.size() << " traces is left" << "\n";
    }

    static void fillup_traces(AnomalyDetector *ptr){
        std::vector<SysRecord> records;
        int random_processes = 50 +rand() % 50;
        for (int i = 0; i < random_processes; i++) {
            int random_trace = 7+ rand()% 200;
            for (int j = 0; j< random_trace ; j++) {
                records.emplace_back(i, rand()% 250);
            }
        }
        ptr->dispatch(records);
    }
};

#endif
