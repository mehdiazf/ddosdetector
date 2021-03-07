#ifndef TCP_HPP
#define TCP_HPP

#include <iostream>
#include <vector>
#include <bitset>

#include <boost/program_options.hpp>

#include <netinet/tcp.h>

#include "baserule.hpp"
#include "ip.hpp"

namespace tcprule
{
    // Textual representation of TCP flags in the correct order, for
    // parsing rules
    const std::vector<char> accept_tcp_flags = { 'U', 'A', 'P', 'R', 'S', 'F' };
}

/*
 TCP flags class, allows you to compare all flags (bits_) at once with
 Using mask (mask_)
*/
class tcp_flags
{
public:
    tcp_flags();
    tcp_flags(const std::pair<std::bitset<6>, std::bitset<6>>& flags);
    bool operator==(const tcp_flags& other) const;
        // Comparison of bits in flags with parameter bits_ by mask mask_
    bool in_this(const std::bitset<6>& flags) const;
    std::string get_flags();

    bool enable;
private:
    // Flag bits
    std::bitset<6> bits_;
    // Comparison mask
    std::bitset<6> mask_;
};

/*
 Class of TCP rules. Contains the verifiable parameters of the package and also the standard
 A set of methods for the proto class.
*/
class TcpRule : public Ipv4Rule, public BaseRule
{
public:
    TcpRule();
    explicit TcpRule(const std::vector<std::string>& tkn_rule);
    bool operator==(const TcpRule& other) const;
    std::string get_description();    
    TcpRule& operator+=(TcpRule& other);
    // Parsing the text representation of the rule according to the opt rules
    void parse(const boost::program_options::options_description& opt);
    // Checking the L4 packet header for a rule match
    bool check_packet(const struct tcphdr *tcp_hdr,
                      const uint32_t s_addr,
                      const uint32_t d_addr) const;

    // Source port 
    NumRange<uint16_t> src_port;
    // Destination port
    NumRange<uint16_t> dst_port;
    // Sequence number
    NumComparable<uint32_t> seq;
    // Acknowledge number
    NumComparable<uint32_t> ack_seq;
    // Window size
    NumComparable<uint16_t> win;
    // Length TCP packet
    NumComparable<uint16_t> len;
    // TCP flags
    tcp_flags flags;
};

#endif // end TCP_HPP