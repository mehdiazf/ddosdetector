#ifndef CLIENT_HPP
#define CLIENT_HPP

#include<sys/types.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>

#include<iostream>
#include<string>
#include<vector>


class Client {

public:
	Client(std::string ip, unsigned short port);
	bool send(const std::string);
	std::string read(const std::string delim);
	~Client();

private:
	bool connect();
	void close();
	int s;
	std::string ip;
	unsigned short port;
};
#endif
