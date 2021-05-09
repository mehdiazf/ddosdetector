#ifndef RULES_HPP
#define RULES_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/bind/placeholders.hpp>

// Logging
#include "log4cpp/Category.hh"
#include "log4cpp/Priority.hh"

#include "exceptions.hpp"
#include "parser.hpp"
#include "lib/queue.hpp"
#include "action.hpp"
#include "functions.hpp"
#include "influxdb.hpp"

// protocols
#include "proto/ip.hpp"
#include "proto/tcp.hpp"
#include "proto/udp.hpp"
#include "proto/icmp.hpp"

// Get log4cpp logger from main programm
extern log4cpp::Category& logger;

/*
 A class for storing packet analysis rules for one L4 protocol (TCP,
 UDP, ICMP, etc.).
*/
template<class T>
class RulesList
{
public:
    explicit RulesList(boost::program_options::options_description opt);
    bool operator==(const RulesList& other)  const;
    /*
     Comparison of the list. Only rules_ and last_update_ are checked against the comparison.
    */
    RulesList& operator=(const RulesList& other);
    /*
     Addition of rule counters. Used by the watcher thread to collect
     Statistics from different copies of sheets in different streams. After addition
     Counters, in other counters are cleared.
    */
    RulesList& operator+=(RulesList& other);
    /*
     Calculation of delta values: how many packets / bytes received per unit
     Time (for 1 second).
     @param rules_old: rule sheet with old data (data captured in
     Past iteration);
    */
    void calc_delta(const RulesList& rules_old);
    /*
     Checking rule triggers, setting jobs to the job processor.
     @param task_list: reference to the task queue of the handler;
    */
    void check_triggers(ts_queue<action::TriggerJob>& task_list,
        InfluxClient& influx);
    
    void _check_triggers(ts_queue<action::TriggerJob>& task_list,  //###############################33
        InfluxClient& influx);
    /*
     Checking rule triggers, setting jobs to the job processor.
     @param task_list: reference to the task queue of the handler;
    */
    void add_rule(T rule);
    /*
    Delete rule by number
     @param num: the number of the deleted rule in the list
    */
    void del_rule(const unsigned int num);
    /*
     Clearing the list of rules
    */
    void clear();
    /*
     Inserting a new rule into the list at position num. If the position is already taken,
     All elements starting from this position are shifted to the end of the list.
     @param num: the position to set the rule
     @param rule: added rule
    */
    void insert_rule(const unsigned int num, T rule);
    /*
     Checking the package according to the rules of the sheet. The function is called for each
     Received packet of type T.
     @param l4header: packet with stripped ip and ethernet headers
     @param s_addr: source ip address
     @param d_addr: destination ip address 
     @param len: whole packet size (with ip and ethernet headers)
    */
    template<typename H>
    bool check_list(const H& l4header, const uint32_t s_addr,
                    const uint32_t d_addr, const unsigned int len)
    {
        boost::lock_guard<boost::shared_mutex> guard(m_);
        for(auto& r: rules_)
        {            
            if(r.check_packet(l4header, s_addr, d_addr))               
            {                     
                r.count_packets++;
                r.count_bytes += len;
                //r.dst_top.increase(d_addr);
                r.dst_top._increase(d_addr,len);
                if(!r.next_rule)
                {
                    return true;
                }
            }
        }
        return false;
    }
    /*
     Displaying a textual representation of rules and statistics.
    */
    std::string get_rules();
    /*
    Generating queries for statistics in InfluxDB
    */
    std::string get_influx_querys();
    /*
     Returns parameters for parsing rules (variable parse_opt_).
    */
    boost::program_options::options_description get_params() const;
private:
    mutable boost::shared_mutex m_;
    // Vector for storing rules
    std::vector<T> rules_;
    // Rule parsing options
    boost::program_options::options_description parse_opt_;
    // Time of the last change of data in the sheet (change of counters)
    std::chrono::high_resolution_clock::time_point last_update_; 
};

/*
 Class for storing rule sheets for different protocols. Contains methods
 To work with all sheets at once. A specific sheet is accessed
 Directly, meaning that the class does not need additional synchronization between
 Streams (data protection from invariance occurs on a more granular
 Level - in the RulesList classes).
*/
class RulesCollection
{
public:
    RulesCollection(boost::program_options::options_description& help_opt,
                // TCP rules options
                boost::program_options::options_description& tcp_opt,
                // UDP rules options
                boost::program_options::options_description& udp_opt,
                // ICMP rules options
                boost::program_options::options_description& icmp_opt);
    /*
     Copy constructor.
     @param clear: if true, then rule lists are cleared
    */
    RulesCollection(const RulesCollection& parent, bool clear = false);
    bool operator!=(const RulesCollection& other) const;
    RulesCollection& operator=(const RulesCollection& other);
    RulesCollection& operator+=(RulesCollection& other);
    /*
     Generates help for all parameters of all types of rules (variable
     Help_opt).
    */
    std::string get_help() const;
    /*
     Forms a textual representation of all rule sheets (functions are called
     RulesList <T> .get_rules ())
    */
    std::string get_rules();
    /*
    Generates a set of queries to the InfluxDB database to add statistics
     (functions RulesList <T> .get_influx_querys () are called)
    */
    std::string get_influx_querys();
    /*
     Checks if the type of the list of rules is valid.
     @param type: type name
    */
    bool is_type(const std::string& type) const;
    /*
   Counting delta data in all lists
     @param old: old version of the data
    */
    void calc_delta(const RulesCollection& old);
    /*
     Checking triggers in all rule lists
     @param task_list
    */
    void check_triggers(ts_queue<action::TriggerJob>& task_list,
        InfluxClient& influx);    
private:
    std::vector<std::string> types_;
    boost::program_options::options_description help_;
public:
    RulesList<TcpRule> tcp; // Rule sheet for TCP
    RulesList<UdpRule> udp; // Rule sheet for UDP
    RulesList<IcmpRule> icmp; // Rule sheet for ICMP
    std::chrono::high_resolution_clock::time_point last_change;
};

/*
 Class of loading / saving file with rules. Set signal_hook
 To reload the configuration when a SIGHUP is received.
*/
class RulesFileLoader
{
public:
    RulesFileLoader(boost::asio::io_context& service, const std::string& file,
        std::shared_ptr<RulesCollection>& c);
    /*
    Loading data from a rules file, setting signal_hook
    */
    void start();
private:
    boost::asio::signal_set sig_set_;
    std::string rules_config_file_;
    std::shared_ptr<RulesCollection>& collect_;
    /*
     Function for reading data from the file rules_config_file_.
     Reads data and enters rules into the collect_ collection.
    */
    void reload_config();
    /*
     Asynchronous SIGHUP signal handler, calls the data read function
     From config reload_config ()
    */
    void sig_hook(boost::asio::signal_set& this_set_,
        boost::system::error_code error, int signal_number);
};

#endif // end RULES_HPP
