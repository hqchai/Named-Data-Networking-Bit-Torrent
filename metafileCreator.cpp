#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>

using namespace std;


int main(int argc, char *argv[]){

	if(argc != 3)
	{
		printf("Usage: First argument: tracker name, second argument: file name");
		exit(1);
	}

	string tracker_name = argv[1];
	string file_name = argv[2];

	//default chunk size is 256KiB
	const int chunk_size = 256*1024;
	ifstream inputfile;
	inputfile.open(file_name);
	
	struct stat file_stat;
	int file_length = 0;
	if(!stat(file_name.c_str(), &file_stat))
		file_length = file_stat.st_size;

	//Here we use SHA1 to hash files and chunks
	char * filebuf = new char[file_length];
	inputfile.read(filebuf, file_length);
    char file_hash[20];
    SHA1(filebuf, file_length, file_hash);
	/*for (i=0; i < 20; i++) {
        printf("%02x", file_hash[i]);
    }*/

	cout << "The tracker name is " << tracker_name << endl;
	cout << "The input file name is " << file_name << endl;
	cout << "The file length is " << file_length << endl;
	cout << "The file hash is " << file_hash << endl;
	cout << "The chunk size is 256KiB" << endl;

	char chunk_hash[20];
	char * chunkbuf[chunk_size];
	int chunk_number = 0;
	for(int i = 0; i < file_length; i=i+chunk_size)
	{
		*chunkbuf = &filebuf[i];
		SHA1(chunkbuf, chunk_size, chunk_hash);
		cout << "The chunk hash is " << chunk_hash << endl;
		chunk_number++;
	}
	*chunkbuf = &filebuf[chunk_number*chunk_size];
	SHA1(chunkbuf, chunk_size, chunk_hash);
	cout << "The chunk hash is " << chunk_hash << endl;
	chunk_number++;

	cout << "Number of chunks: " << chunk_number << endl;

	return 0;
}