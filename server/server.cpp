#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

#include "../include/parser.hpp"
#include "server.hpp"

#include <iostream>

using namespace std;

Server::Server(string port) {
	this->port = port;
	start();
	accept_input();

	//alive_checker = thread(&Server::check_alive_peers, this);
}

void Server::start() {
	this->set_connection_hints();

    struct addrinfo *result;
	getaddrinfo(NULL, port.c_str(), &this->hints, &result);

	struct addrinfo *rp;
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		int optval = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
		bind(sock, rp->ai_addr, rp->ai_addrlen);
	}

	listen(sock, BACKLOG);
	freeaddrinfo(result);

	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK);
	acceptor = thread(&Server::accept_peers, this);
}

void Server::set_connection_hints() {
	memset(&this->hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
}

void Server::accept_peers() {
	while(not close_server) {
		struct sockaddr_in peer_addr;
		socklen_t addr_len = sizeof(peer_addr);

		int peer_sock = accept(sock, (struct sockaddr*)&peer_addr, &addr_len);

		if(peer_sock > 0) {

			char buffer[256];
			memset(buffer, 0, 256);
			recv(peer_sock, buffer, sizeof(buffer), 0);
            cout << buffer << endl;
			string buffer_str = buffer;

			vector<string> parsed_address;
            Parser parser;
            parsed_address = parser.parse(buffer_str, ':');

            if (parsed_address.size() != 3) {
                string error_msg = "INVALID ADDRESS";
                send(peer_sock, error_msg.c_str(), error_msg.size(), 0);
                close(peer_sock);
                return;
            }

			send_peer_list(peer_sock);

			string id = parsed_address[0];
			string ip = parsed_address[1];
			string port = parsed_address[2];

			cout << "New peer connected (" << id << ") -> " << ip << ":" << port << endl;

			peers[id] = {peer_sock, id, ip, port};
		}
		sleep(1);
	}
}

void Server::check_alive_peers() {
	while(not close_server) {
		sleep(60);
		for (auto &peer : peers) {
			string ping = "PING";
			int peer_sock = peer.second.sock;
			send(peer_sock, ping.c_str(), ping.size(), 0);

			char answer[4];
			memset(answer, 0, 4);
			recv(peer_sock, answer, 4, 0);
			cout << answer << endl;
		}
	}
}

string Server::list_peers() {
	string peer_list = "";
	for(auto &peer : peers) {
		peer_list += peer.second.id + ":";
		peer_list += peer.second.ip + ":";
		peer_list += peer.second.port + "|";
	}
	if(peer_list.size() > 0)
		peer_list = peer_list.substr(0, peer_list.size()-1);
	else
		peer_list = "no_peers";
	return peer_list;
}

void Server::send_peer_list(int peer) {
	string peer_list = list_peers();
	send(peer, peer_list.c_str(), peer_list.size(), 0);
}

void Server::accept_input() {
	while(not close_server) {
		string input;
		cin >> input;

		if(input == "q") {
			close_server = true;
			for (auto &peer : peers) {
				close(peer.second.sock);
				peers.erase(peer.first);
			}
		}
		else if(input == "l") {
			cout << list_peers() << endl;
		}
	}
	acceptor.join();
}

