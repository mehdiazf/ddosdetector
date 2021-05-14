#include "client.hpp"

Client::Client(std::string ip_, unsigned short port_)
	:ip(ip_), 
	port(port_)
	{
		connect();
	}
Client::~Client(){
	close();
}
	
bool Client::connect(){
	struct sockaddr_in sa;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return 1;
	}
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = 0;
	if (::connect(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		perror("connect");
		::close(s);
		return 2;
	}
	::fcntl(s, F_SETFL, O_NONBLOCK);
	return true;
}
void Client::close(){
	::close(s);
}

bool Client::send(const std::string str){
	write(s, str.c_str(), str.size());
	return true;
}

std::string Client::read(std::string delim){
	char buff[50];
	::read(s, buff, sizeof(buff));
	printf("Response: %s\n", buff);
	return std::string(buff);
}
