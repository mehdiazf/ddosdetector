#ifndef ICMP_HPP
#define ICMP_HPP

#include <iostream>
#include <vector>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <boost/program_options.hpp>

#include <netinet/ip_icmp.h>

#include "baserule.hpp"
#include "ip.hpp"

/*
 ICMP rules class. Contains the verifiable parameters of the package and also the standard
 A set of methods for the proto class.
*/
class IcmpRule : public Ipv4Rule, public BaseRule
{
public:
    IcmpRule();
    explicit IcmpRule(const std::vector<std::string>& tkn_rule);
    bool operator==(const IcmpRule& other) const;
    std::string get_description();    
    IcmpRule& operator+=(IcmpRule& other);
    // Parsing the text representation of the rule according to the opt rules
    void parse(const boost::program_options::options_description& opt);
    // Checking the L4 packet header for a rule match
    bool check_packet(const struct icmphdr *icmp_hdr,
                      const uint32_t s_addr,
                      const uint32_t d_addr) const;

    // ICMP packet type
    NumComparable<uint8_t> type;
    // ICMP packet code
    NumComparable<uint8_t> code;
};

#endif // end ICMP_HPP