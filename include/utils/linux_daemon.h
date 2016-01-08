//
// Created by root on 1/8/16.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_LINUX_DAEMON_H
#define HIGHPERFORMANCELINUXSENSORS_LINUX_DAEMON_H

#include <sys/stat.h>
#include <string>
void setup_daemon(const std::string & deamon_name,const std::string & run_path) {
    //------------
    //Daemon Setup
    //------------
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
           we can exit the parent process. */
    if (pid > 0) {
        std::cout << "Creating a daemon process pid= " << pid << "\n";
        std::cout << "You can stop the process by the following command '" << deamon_name << " stop\n";
        exit(EXIT_SUCCESS);
    }

    write_to(run_path + "/" + deamon_name + ".pid", std::to_string(getpid()));

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}


int get_sensor_pid(const std::string & deamon_name, const std::string & run_path){
    std::ifstream pid_file(run_path + "/" + deamon_name + ".pid");
    int pid = -1;
    if (pid_file.good())
        pid_file >> pid;
    pid_file.close();
    return pid;
}

int remove_sensor_pid(const std::string & deamon_name, const std::string & run_path){
    std::remove((run_path + "/" + deamon_name + ".pid").c_str());
}

int kill_sensor(int pid){
    write_to(SYSCALLS_PATH + "/filter", "0");
    write_to(SYSCALLS_PATH + "/" + SYS_ENTRY + "/" + "filter", "0");
    write_to(SYSCALLS_PATH + "/" + SYS_EXIT + "/filter", "0");
    sleep_sec(5);
    auto curr_pid = get_pid();
    auto pids = get_pids(pid);
    for (auto itr = pids.begin();itr < pids.end(); itr++){
        if(curr_pid != *itr)
            kill(*itr, SIGKILL);
    }
    write_to(FTRACE_PATH + "/tracing_on", "0");
    kill(pid, SIGKILL);
}

#endif //HIGHPERFORMANCELINUXSENSORS_LINUX_DAEMON_H
