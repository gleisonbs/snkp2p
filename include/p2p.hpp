#ifndef P2P_HPP
#define P2P_HPP

#include <map>
#include <string>
#include <thread>
#include <map>
#include <mutex>
#include <vector>

#include <conn_manager.hpp>
#include <file_manager.hpp>
#include <parser.hpp>

enum LocalRequest : unsigned int { 
	Quit 							= 0, 
	AddFolder 				= 1, 
	ListLocalFiles 		= 2, 
	ListLocalFolders 	= 3,
	Connect 					= 4,
	ListPeers 				= 5,
	ListPeerFiles			= 6,
	Search 						= 7,
	RequestDownload		= 8
};

enum RemoteRequest : unsigned int { 
	EndConnection 	= 0,
	ListPeerFiles_R	= 1,
	Search_R				= 2,
	SendFile				= 3,
	ReceivedFile		= 4
};

class P2P
{
public:
	P2P(int);
	~P2P();
	void run();
	void search_file(std::string&);
	void add_folder(std::string);
    void display_menu();

	std::string id;

private:
	void generate_id();
	void handle_peer_request();
	bool handle_local_request(std::string&); // RETURNS TRUE CASE THE "QUIT" REQUEST IS ENTERED

	int local_port;
	int upload_port = 5005;
	std::vector<int> peers;

	//Crypto cryp;
	ConnectionManager cm;
	FileManager fm;
    Parser parser;

	std::thread request_handler;
	std::vector<std::thread> reader_threads;
	std::vector<std::thread> downloads;
	std::vector<std::thread> uploads;

	bool should_close = false;
};

#endif //P2P_HPP
