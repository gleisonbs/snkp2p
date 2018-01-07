#ifndef __CONNECTION_HPP
#define __CONNECTION_HPP

class Connection {
public:
	Connection();
	// starts a socket that will listen for incoming connections
	void listen_at(const &std::string);
	// tries to connect to the given host and port
	void connect_to(const &std::string, const &std::string);
	int get_socket() { return sock };
private:
	int sock;
	constexpr int BACKLOG = 5;
};

#endif __CONNECTION_HPP