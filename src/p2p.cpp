#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/types.h>
#include <cstdlib>
#include <ctime>

#include <parser.hpp>
#include <p2p.hpp>

using namespace std;

P2P::P2P(int port) : local_port(port) {
	srand(time(0));
	generate_id();

	cm.start_server(to_string(local_port));

	//string ip = cm.get_external_ip();
	cm.connect_to_rendezvouz_server("localhost", to_string(4000), id, "localhost", to_string(local_port));
	//cm.receive_peer_list();

	request_handler = thread(&P2P::handle_peer_request, this);
}

P2P::~P2P() {}

void P2P::run() {
	while(true) {
        this->display_menu();
		string request;
		cin >> request;

		// RETURNS TRUE IF THE "QUIT" REQUEST IS ENTERED
		bool should_quit = handle_local_request(request);
		if(should_quit) break;
	}
}

void P2P::handle_peer_request() {
	while(not should_close) {
		sleep(2);
		while(not cm.requests.empty()) {
			string req = cm.requests.front();
			cm.requests.pop();
			cout << req << endl;
		}
		// vector<string> fields = parse_request(request);
		// unsigned int request_type;

		// try {
		// 	request_type = stoi(fields[0]); // ONE BYTE LONG
		// }
		// catch(const exception &e) {
		// 	request_type = -1;
		// }

		// switch(request_type) {

		// 	case RemoteRequest::EndConnection: { // 0
		// 		for(size_t i = 0; i < peers.size(); ++i) {
		// 			if(peers[i] == peer) {
		// 				cout << "Peer " << peers[i] << " closed." << endl;
		// 				peers.erase(peers.begin() + i);
		// 				break;
		// 			}
		// 		}
		// 	}

		// 	case RemoteRequest::ListPeerFiles_R: { // 1
		// 		string file_list = fm.get_file_list();
		// 		write(peer, file_list.c_str(), file_list.size());
		// 	}
		// 		break;

		// 	case Search_R: { // 2

		// 	}
		// 		break;
		// 	case RemoteRequest::SendFile: { // 3
		// 		string filename = fields[1];
		// 		size_t file_size = fm.get_file_size(filename);
		// 		string request = to_string(RemoteRequest::ReceivedFile) + ":" + filename + ":" + to_string(file_size) + ":" + to_string(upload_port);

		// 		// thread upload = thread(&P2P::file_sender, this,
		// 		// 	upload_port,
		// 		// 	filename);
		// 		// uploads.push_back(move(upload));
		// 		// write(peer, request.c_str(), request.size());
		// 	}
		// 		break;
		// 	case RemoteRequest::ReceivedFile: { // 4
		// 		string filename = fields[1];
		// 		int file_size = stoi(fields[2]);
		// 		int port = stoi(fields[3]);
		// 		// thread download = thread(&P2P::receive_file, this,
		// 		// 	filename, file_size, port);
		// 		// downloads.push_back(move(download));
		// 	}
		// 		break;
		// 	case -1:
		// 		cout << request << endl;
		// }
	}
}

bool P2P::handle_local_request(std::string &request) {

	vector<string> fields = this->parser.parse(request, ':');
    cout << "Fields" << endl;
    for (auto &f : fields) {
        cout << f << endl;
    }

	int request_type;
	try {
		request_type = stoi(fields[0]); // ONE BYTE LONG
	}
	catch (exception &e) {
		request_type = -1;
	}

	switch(request_type) {
		case LocalRequest::Quit: { // 0

			cout << "Quiting..." << endl;

			should_close = true;

			for(auto &peer : cm.peers) {
				string end_connection = "0";
				write(peer, end_connection.c_str(), end_connection.size());
				close(peer);
			}

			cm.finish();
			return true;
		}
		case LocalRequest::AddFolder: { // 1
            if (fields.size() != 2) {
                cout << "Second argument must be a folder" << endl;
            } else {
    			fm.add_folder(fields[1]);
            }
		}
			break;
		case LocalRequest::ListLocalFiles: { // 2
			cout << "--> File List:\n"
					 << "\t" << fm.get_file_list() << endl;
		}
			break;
		case LocalRequest::ListLocalFolders: { // 3
			cout << "--> Folder List:\n"
					 << "\t" << fm.get_folder_list() << endl;
		}
			break;
		case LocalRequest::Connect: { // 4
            if (fields.size() != 3) {
                cout << "Second argument must be a ip, third a port" << endl;
            }
            else {
                cm.connect_to_peer(fields[1], fields[2]);
            }
		}
			break;
		case LocalRequest::ListPeers: { // 5
			cout << "--> Peer List:\n"
					 << cm.list_peers() << endl;
		}
			break;
		case LocalRequest::ListPeerFiles: { // 6
			int peer = stoi(fields[1]);
			string list_files_request = "1";
			write(peer, list_files_request.c_str(), list_files_request.size());
		}
			break;
		case LocalRequest::Search: { // 7
			string filename = fields[1];
			for(auto &peer : peers) {
				write(peer, request.c_str(), request.size());
			}
		}
			break;
		case LocalRequest::RequestDownload: { // 8
			int peer = stoi(fields[1]);
			string filename = fields[2];

			string file_request = to_string(RemoteRequest::SendFile) + ":" + filename;
			write(peer, file_request.c_str(), file_request.size());
		}
			break;
		case -1:
			cout << "Command not recognized" << endl;
	}

	return false;
}


void P2P::add_folder(string folder) {
	fm.add_folder(folder);
}

void P2P::generate_id() {
	id = "sdafasfasfas";//cryp.md5(to_string(rand()));
}

void P2P::display_menu() {
    cout
        << "\nMenu:\n"
        << "1 - Add Folder (Folder Path)\n"
        << "2 - List Local Files\n"
        << "3 - List Local Folders\n"
        << "4 - Connect to (Host:Port)\n"
        << "5 - List Peers\n"
        << "6 - List Peer Files (Peer)\n"
        << "7 - Search File (File)\n"
        << "8 - Request Download (Peer:File)\n"
        << endl;
}
