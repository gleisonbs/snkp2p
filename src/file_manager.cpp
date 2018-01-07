#include <fstream>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <iostream>

#include <file_manager.hpp>

using namespace std;
//using namespace boost::filesystem;

FileManager::FileManager() {}

void FileManager::add_folder(const string folder)
{
	folders.push_back(folder);
	update_file_index();
}

void FileManager::update_file_index()
{
	DIR *dir;
	struct dirent *ent;
	string dir_path = folders.back();
	if((dir = opendir(dir_path.c_str())) != NULL) {
	  /* print all the files and directories within directory */
	  while((ent = readdir(dir)) != NULL) {
	  	
	  	char absolute_path[PATH_MAX];
	  	string relative_path = folders.back() + "/" + ent->d_name;
	  	realpath(relative_path.c_str(), absolute_path);

	  	struct stat s;
	  	stat(absolute_path, &s);
	  	if(s.st_mode & S_IFREG)	  	
	   		file_index[ent->d_name] = absolute_path;
	  }
	  closedir(dir);
	} else {
	  /* could not open directory */
	  cout << "directory \"" << dir_path << "\" not found" << endl;
	}
}

string FileManager::get_file_list()
{
	string file_list;
	for(auto &file : file_index) {
		file_list += file.first + "\n";
	}

	return file_list;
}

string FileManager::get_folder_list()
{
	string folder_list;
	for(auto &folder : folders) {
		folder_list += folder + "\n";
	}

	return folder_list;
}

int FileManager::get_file(string filename, unsigned char **file) {
	string path_to_file = file_index[filename];

	FILE *fp = fopen(path_to_file.c_str(), "rb");

	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	*file = (unsigned char*)malloc(file_size);

	int bytes_read = fread(*file, 1, file_size, fp);

	fclose(fp);

	return bytes_read;
}

bool FileManager::has_file(string &filename) {
	return file_index.count(filename);
}

size_t FileManager::get_file_size(string filename) {
	string file_path = file_index[filename];
	ifstream file(file_path, ios::binary | ios::ate);
	
	size_t file_size = file.tellg();
	
	file.close();

	return file_size;
}

size_t FileManager::save_file(string filename, int file_size, unsigned char *file) {
	string path_to_file = folders.back() + "/" + filename;
	FILE *fd = fopen(path_to_file.c_str(), "wb");
	size_t bytes_writen = fwrite(file, 1, file_size, fd);
	fclose(fd);

	return bytes_writen;
}
