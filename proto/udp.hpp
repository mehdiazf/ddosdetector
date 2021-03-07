#ifndef UDP_HPP
#define UDP_HPP

#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include <netinet/udp.h>

#include "baserule.hpp"
#include "ip.hpp"

/*
UDP rules class. Contains the verifiable parameters of the package and also the standard
 A set of methods for the proto class.
*/
class UdpRule : public Ipv4Rule, public BaseRule
{
public:
    UdpRule();
    explicit UdpRule(const std::vector<std::string>& tkn_rule);
    bool operator==(const UdpRule& other) const;
    std::string get_description();    
    UdpRule& operator+=(UdpRule& other);
    // Parsing the text representation of the rule according to the opt rules
    void parse(const boost::program_options::options_description& opt);
    // Checking the L4 packet header for a rule match
    bool check_packet(const struct udphdr *udp_hdr,
                      const uint32_t s_addr,
                      const uint32_t d_addr) const;

    // Source port
    NumRange<uint16_t> src_port;
    // Destination port
    NumRange<uint16_t> dst_port;
    // UDP packet length
    NumComparable<uint16_t> len;
};

#endif // end UDP_HPP