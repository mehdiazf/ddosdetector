#ifndef CONTROLD_HPP
#define CONTROLD_HPP
#include <stdio.h>
#include <signal.h>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
//#include <type_traits>
#include <boost/lexical_cast.hpp>

// Logging
#include "log4cpp/Category.hh"
#include "log4cpp/Priority.hh"

#include "functions.hpp"
#include "parser.hpp"
#include "rules.hpp"

// Get log4cpp logger from main programm
extern log4cpp::Category& logger;

/*
 Client session class. Handles the connection of one client,
 Parses commands received from the client, outputs data
 By the requested commands.
*/
template<class T>
class ControlSession
    : public std::enable_shared_from_this<ControlSession<T>>
{
public:
    ControlSession(T socket, const std::shared_ptr<RulesCollection> c);
    ~ControlSession();
    // Starting the session handling process
    void start();

private:
    // Receiving data from the client, reads into the data_ buffer, of size max_length and
    // adds data to cmd_ until the command termination character is encountered:  n
    void do_read();
    // FUTURE: function to catch Ctrl% D signal (not used yet)
    void do_read_signal();
    // Sending data from the client
    void do_write(const std::string& msg);
    // Parser of commands received from the client
    void parse();

    T socket_;
    // Reference to the reference collection of rules to modify
    std::shared_ptr<RulesCollection> collect_;
    // Buffer
    enum { max_length = 4096 };
    char data_[max_length];
    // Received command from the client
    std::string cmd_;
    // Console greeting
    const std::string cli_ = "ddoscontrold> ";
    // Characters to remove from received commands
    std::string bad_symbols_{'\n', '\r', '\0'};
    // Help text
    const std::string help_ = "Console commands:"
        "<type> - may be TCP, UDP or ICMP; <num> - number (0..65535);\n"
        "  help                                show this help\n"
        "  add rule <type> <rule>              add new rule\n"
        "  insert rule <type> <num> <rule>     insert new rule by number\n"
        "  del rule <type> <num>               add new rule\n"
        "  show rules                          print all rules with counters\n"
        "  reload rules                        reload all rules from file\n"
        "  exit                                close connection\n";
};

/*
 TCP / UNIX server class. During initialization, the server type is determined. If port is
 This number, the TCP server is started on port port. If port is the path to
 File, the UNIX server is started.
*/
class ControlServer
{
public:
    /*
    Server initialization.
     @param io_service: pre-created io_service object
     @param port: the port on which the server is started (or the path to the unix socket)
     @param collect: reference collection of rules
    */
    ControlServer(boost::asio::io_context& io_service, const std::string& port,
                  std::shared_ptr<RulesCollection> collect);
    // The destructor keeps track of the correct deletion of the UNIX socket file
    ~ControlServer();
private:
    // Starting tcp server
    void do_tcp_accept();
    // Starting unix server
    void do_unix_accept();

    // Flag unix server
    bool is_unix_socket_;
    // Launch port
    std::string port_;
    // acceptors
    std::shared_ptr<boost::asio::ip::tcp::tcp::acceptor> tcp_acceptor_;
    std::shared_ptr<boost::asio::local::stream_protocol::acceptor> unix_acceptor_;
    // Sockets
    std::shared_ptr<boost::asio::ip::tcp::tcp::socket> tcp_socket_;
    std::shared_ptr<boost::asio::local::stream_protocol::stream_protocol::socket> unix_socket_;
    // Reference collection
    std::shared_ptr<RulesCollection> collect_;
};

#endif // end CONTROLD_HPP
