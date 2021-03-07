#ifndef Action_HPP
#define Action_HPP

#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include <algorithm>
#include <syslog.h>
#include <stdlib.h>
#include <fstream>
#include <chrono>
#include <ctime>

// Logging
#include "log4cpp/Category.hh"
#include "log4cpp/Priority.hh"

#include "exceptions.hpp"
#include "functions.hpp"

// Get log4cpp logger from main programm
extern log4cpp::Category& logger;

namespace action
{
    /*
     Functions-operations. This function is performed by the trigger action when it
     Works.
    */
    // Logging function
    void job_log(const std::string& to, const std::string& data);
    // Third-party script call function
    void job_script(const std::string& to, const std::string& data);
    // Packet dump fuction
    //void job_dump(const std::string& to, const std::string& data); // FUTURE: create dump traffic and store to .pcap file
    // Function to write messages to syslog
    void job_syslog(const std::string& to, const std::string& data);

    // Allowed job types
    typedef std::function<void(const std::string&, const std::string&)> j_funct;
    typedef std::map<std::string, j_funct> types_map_t;
    struct type_list
    {
        static types_map_t jobs;
        static types_map_t::iterator find(const std::string& v);
        static types_map_t::iterator end();
    };

    /*
     Trigger event class. Contains data to call a job when a trigger
     Worked.
    */
    class Action
    {
    public:
        Action();
        Action(const Action& other);
        explicit Action(const std::string& type);
        Action(const std::string& type, const std::string& file);
        Action& operator=(const Action& other);
    private:
        // Changing the type of task
        std::string check_type(const std::string& type) const;
    protected:
        // Job type, maybe: log, script, dump
        std::string type_;
        // Imph file, path to script, path to log, etc.
        std::string file_;
    };

    /*
     Trigger assignment class. Contains an instance of a trigger event, and
     Provides an interface for executing the task inherent in the trigger
    */
    class TriggerJob : public Action
    {
    public:
        TriggerJob();
        TriggerJob(const Action& a, const std::string& d);
        // Starting a trigger job
        void run();
    private:
        // Data for setting the trigger
        std::string data_;
    };
}

#endif // end Action_HPP