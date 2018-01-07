#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <netdb.h>
#include <string>
#include <thread>
#include <vector>
#include <utility>

#define BACKLOG 50

struct PeerInfo {
	int sock;
	std::string id;
	std::string ip;
	std::string port;
};

class Server {
public:
	Server(std::string);
	void start();

private:
	void check_alive_peers();
	void accept_peers();
	void send_peer_list(int);
	void accept_input();
	std::string list_peers();

	int sock;
	std::string port;
	struct addrinfo hints;
	std::map<std::string, PeerInfo> peers;

	std::thread acceptor;
	std::thread alive_checker;

	bool close_server = false;

    void set_connection_hints();
};

#endif // SERVER_HPP
