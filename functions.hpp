#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <stdlib.h> // atoi
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <algorithm>
#include <vector>

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>


// Logging
#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/Priority.hh"
#include "log4cpp/PatternLayout.hh"

/*
 Initialization of the logger logger, determination of the logging level and purpose
 Where the log will be written.
 @param debug: debug mode
 @param file: file where to write the log, if == ', then output to the console
*/
void init_logging(log4cpp::Category& logger, bool debug,
                  const std::string& file);
/*
 Switching the network card to promisc on mode
*/
#ifdef __linux__
bool manage_interface_promisc_mode(const std::string& interface_name,
                                   bool switch_on);
#endif
/*
 Converts the interface name to netmap format, i.e. From eth5 will make
 Netmap: eth5 as required to run nm_open () function from library
 If the interface name already contains 'netmap:', then no changes are made
*/
std::string get_netmap_intf(const std::string& intf);
/*
 Checks if a file exists, check is done via unix stat
*/
bool is_file_exist(const std::string& file_name);
/*
 Checks if executable file
*/
bool is_executable(const std::string& file_name);
/*
 Formats a string to a specific length to align the output
 @param s: string
 @param len: minimum length of the output string
*/
std::string format_len(const std::string& s, unsigned int len);
/*
 Divides the input string into elements based on separator and forms
 Result in vector <string>
*/
typedef boost::escaped_list_separator<char> separator_type;
std::vector<std::string> tokenize(const std::string& input,
                                  const separator_type& separator);
std::vector<std::string> tokenize(const std::string& input);
/*
 Returns the number of value in the vec list, or throws an exception
 @param vec: the vector in which to search for an element
 @param value: the element to search for
*/
template<typename T>
int get_index(const std::vector<T>& vec, const T& value)
{
    auto it = std::find(vec.begin(), vec.end(), value);
    if (it == vec.end())
    {
        throw std::invalid_argument("unsupported value");
    } else
    {
        return std::distance(vec.begin(), it);
    }
}

template <typename T>
std::string to_string(T val)
{
    std::stringstream stream;
    stream << val;
    return stream.str();
}

#endif // end FUNCTIONS_HPP