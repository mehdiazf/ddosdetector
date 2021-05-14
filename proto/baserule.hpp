#ifndef BASERULE_HPP
#define BASERULE_HPP

#include <iostream>
#include <vector>
#include <chrono>
#include <map>

#include <boost/program_options.hpp>

#include "../action.hpp"
#include "../parser.hpp"

class Counter
{
 public:
    Counter();
    Counter(const Counter& other);
    Counter& operator=(const Counter& other);
    uint64_t count_packets;
    uint64_t count_bytes;
    uint64_t pps;
    uint64_t bps;
    std::time_t pps_not_trig;
    std::time_t bps_not_trig;
    std::chrono::high_resolution_clock::time_point last_update_;
    
};
namespace Comperator{
    
    bool cmp_byte(const std::pair<uint32_t, Counter>& a, const std::pair<uint32_t, Counter>& b);    
    bool cmp_packet(const std::pair<uint32_t, Counter>& a, const std::pair<uint32_t, Counter>& b);    
}
template<class Key, class Val>
class CountersList
{
public:
    CountersList();    
    CountersList& operator+=(CountersList& other);
    // List output (debug)
    void print() const;
    // List size
    unsigned int size() const;
    // Sheet cleaning
    void clear();
    // Increment the counter of element k by 1 or v
    void increase(const Key& k);
    void _increase(const Key & k, const unsigned int len);
    //void increase(const Key& k, const Val& v);
    // Returns the name of the largest counter
    std::string get_max() const;
    void calc_delta(const CountersList& old);
    std::vector<std::string> get_ip_list(uint32_t pps_trig,
    unsigned int pps_period, uint32_t bps_trig, unsigned int bps_period);
    
private:
    std::map<Key,Counter> map_;
};
/*
 Numeric period class. Used to store some kind of restriction
 Having the lowest and highest numbers. For example, the presentation of ip addresses:
 0.0.0.0/0 0-4294967295 as period: 0-4294967295 allows for further
 Make a quick check that the ip address belongs to the subnet using
 In_this () functions.
*/
template<class T>
class NumRange
{
public:
    NumRange();
    explicit NumRange(const std::pair<T, T>& p);
    bool operator==(NumRange const & other) const;
    NumRange& operator=(const std::pair<T, T>& p);
    // Checking that there is chilso in the period
    bool in_this(T num) const;
    // Parameter status (enabled or disabled)
    bool stat() const;
    // Returns the period in CIDR format, converting numbers to ip addresses
    std::string to_cidr() const;
    // Returns the period in the format of a string, notation separated by a '-' sign
    std::string to_range() const;
private:
    // The first number of the interval
    T start_;
    // Last number of the interval
    T end_;
    // Status
    bool enable_;
};
/*
 The class of the numeric parameter to compare. Allows you to store the number and set the type
 Comparisons with this number (=,> or <). Further, the call to the in_this () function
 Will compare the passed number with the parameter, and the comparison will be of type type_
*/
template<class T>
class NumComparable
{
public:
    NumComparable();
    explicit NumComparable(const std::pair<T, unsigned short int>& p);
    bool operator==(NumComparable const & other) const;
    NumComparable& operator=(const std::pair<T, unsigned short int>& p);
    // Compare num to parameter
    bool in_this(T num) const;
    // string representation of a number with an indication of the comparison type
    bool stat() const;
    std::string to_str() const;
private:
    // Parameter number
    T num_;
    // status
    bool enable_;
    // Comparison type, can be:
    // 0 is =
    // 1 is>
    // 2 is <
    unsigned short int type_;
};


class BaseRule
{
public:
    BaseRule();
    explicit BaseRule(const std::vector<std::string>& tkn_rule);
    void BaseRule_parse(const boost::program_options::variables_map& vm);
    // Checking the triggers of the rule, if the trigger did not work, then the
    // update time in variables: pps_last_not_triggered and
    // bps_last_not_triggered.
    bool is_triggered();
    // Formation of text for setting a trigger
    std::string get_job_info(std::string txt) const;
    // Generating a request to the InfluxDB database for a triggered trigger
    std::string get_trigger_influx() const;
    //calc data per ip
    void calc_delta(const BaseRule& old);
    void get_ip_list(std::vector<std::string> & vct, const std::string& description);
    
    // Basic rule parameters
    std::string rule_type;                             // Textual representation of the rule type (tcp, udp, etc.)
    std::string text_rule;                        // Text of the rule
    action::Action act;                           // Trigger action
    std::string comment;                          // Comment on the rule
    uint64_t count_packets;                       // Packet counter
    uint64_t count_bytes;                         // Byte counter
    bool next_rule;                               // Go to next rule
    uint64_t pps;                                 // Packets per second counter
    uint64_t bps;                                 // Bytes per second counter
    uint32_t pps_trigger;                         // Trigger trigger for pps (command: --pps-trigger)
    uint32_t bps_trigger;                         // Trigger trigger for bps (command: --bps-trigger)
    std::time_t pps_last_not_triggered;           // Time of the last pps trigger
    std::time_t bps_last_not_triggered;           // Time of the last bps trigger
    unsigned int pps_trigger_period;              // The period that the pps trigger should be active
    unsigned int bps_trigger_period;              // Period that the bps trigger should be active
    CountersList<uint32_t, unsigned int> dst_top; // Dst_ip list of addresses with counters, if a subnet is used
protected:
    // The text of the rule divided into components (by space or sign =)
    std::vector<std::string> tokenize_rule;
};

#endif // end BASERULE_HPP
