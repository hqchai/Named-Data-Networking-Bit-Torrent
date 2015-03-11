#include "TorrentFileManager.hpp"

using namespace std;

int DEFAULT_CHUNK_SIZE = 16;//256*1024;
int debug = 1;

TorrentFileManager::TorrentFileManager(string filename) {
// Checks if metadata file for file already exists
// If it doesn't it initializes a new one and sets variables
// Otherwise, it will read the existing metafile and set variables

  // Check if file exists
  ifstream tfilestream;
  string tfilename = filename + ".torrent";

  tfilestream.open(tfilename.c_str(), fstream::in);
  if (tfilestream.fail()) {
    // If metafile doesn't exist, create a new one    
    // If the original file also doesn't exist, throw an error
    tfilestream.open(filename.c_str(), fstream::in);
    if (tfilestream.fail()) {
      cout << "Failed to find original file to create torrent file. " << 
              "Check that file " << filename << " exists\n";
      return;
    }
    else {
      cout << "Entering\n";
      this->createTorrentFile(filename);
      cout << "Leaving\n";
    }
  }
  else {
    // Otherwise, read in values and set variables
    this->setFromTorrentFile(filename);
  }
  tfilestream.close();

  if (debug) {
    cout << "filename = " << this->getFilename() << endl;
    cout << "filesize = " << to_string(this->getFilesize()) << endl;
    cout << "filehash = " << this->getFilehash() << endl;
    cout << "num chunks = " << to_string(this->getNumChunks()) << endl;
    cout << "chunk size = " << to_string(this->getChunkSize()) << endl;
    for (int i=0; i<this->numchunks; i++) {
      cout << "chunk hash " <<to_string(i) << " = " << this->getChunkHash(i) << endl;
    }
  }
  return;
}

void TorrentFileManager::createTorrentFile(string filename) {
// Analyzes contents of file and writes to metafilestream  
// Sets filename filesize filehash numchunks chunksize chunkhashses
// Must reopen metafilestream
  ofstream tfilestream;
  ifstream fstream;
  fstream.open(filename.c_str(), ifstream::in);
 
  this->filename = filename; 

  // Get file size
  fstream.seekg (0, fstream.end);
  this->filesize = fstream.tellg();
  fstream.seekg (0, fstream.beg);

  // Calculate SHA1 hash of file
	char* filebuf = new char[this->filesize];
  unsigned char hash[20];
  char hexhash[40];

	fstream.read(filebuf, this->filesize);
  SHA1((unsigned char*)filebuf, this->filesize, hash);

  this->filehash = hashToHex((unsigned char*)filebuf, this->filesize);

	this->chunksize = DEFAULT_CHUNK_SIZE;

  // Calculate number of chunks and allocate memory for chunk hashes
  this->numchunks = filesize/chunksize;
  if (filesize%chunksize != 0) {    // If filesize does not divide evenly, we need another chunk at end
    this->numchunks++;
  }

  this->chunkhashes = new string[numchunks]; 

  // Hash each chunk	
	unsigned char chunkhash[20];

  for (int i=0; i<this->numchunks; i++) {
    if (i+this->chunksize < filesize) {
      this->chunkhashes[i] = hashToHex((unsigned char*)&filebuf[i*this->chunksize], this->chunksize);
    }
    else {
      this->chunkhashes[i] = hashToHex((unsigned char*)&filebuf[i*this->filesize-i], this->chunksize);
    }
  }

  // Write everything to metafile
  string line;
  int linelength;

  tfilestream.open((filename + ".torrent").c_str(), ofstream::out);
    
  line = "filename: " + this->filename + "\n";
  tfilestream.write(line.c_str(), line.length());
  
  line = "filesize: " + to_string(this->filesize) + "\n";
  tfilestream.write(line.c_str(), line.length());

  line = "filehash: " + this->filehash + "\n";
  tfilestream.write(line.c_str(), line.length());

  line = "numchunks: " + to_string(this->numchunks) + "\n";
  tfilestream.write(line.c_str(), line.length()); 

  line = "chunksize: " + to_string(this->chunksize) + "\n";
  tfilestream.write(line.c_str(), line.length());

  for (int i=0; i<numchunks; i++) {
    line = "hash" + to_string(i) + ": " + this->chunkhashes[i] + "\n";
    tfilestream.write(line.c_str(), line.length());
  }

  // Free buffers and return
  delete filebuf;
  fstream.close();
  tfilestream.close();

	return;
}

void TorrentFileManager::setFromTorrentFile(string filename) {
  // Open the file for reading
  string tfilename = filename + ".torrent";
  std::ifstream ifs;

  ifs.open(tfilename, std::ifstream::in);
  if (!ifs.is_open()) {
    std::cout << "Couldn't open " + tfilename + "for reading\n";
    return;
  }

  //  Read each variable in
  string line;
  int n;
  
  // Get filename
  getline(ifs, line);
  n = line.find(":");
  this->filename = line.substr(n+2, line.size());

  // Get filesize
  getline(ifs, line);
  n = line.find(":");
  this->filesize = stoi(line.substr(n+2, line.size()));

  // Get filehash
  getline(ifs, line);
  n = line.find(":");
  this->filehash = line.substr(n+2, line.size());

  // Get numchunks
  getline(ifs, line);
  n = line.find(":");
  this->numchunks = stoi(line.substr(n+2, line.size()));

  // Get chunksize
  getline(ifs, line);
  n = line.find(":");
  this->chunksize = stoi(line.substr(n+2, line.size()));


  // Get chunkhashes
  this->chunkhashes = new string[this->numchunks];
  for (int i=0; i<this->numchunks; i++) {
    getline(ifs, line);
    n = line.find(":");
    this->chunkhashes[i] = line.substr(n+2, line.size());
  }

  return; 
}


string TorrentFileManager::getFilename() {
  return this->filename;
}

int TorrentFileManager::getFilesize() {
  return this->filesize;
}

string TorrentFileManager::getFilehash() {
  return this->filehash;
}

int TorrentFileManager::getNumChunks() {
  return this->numchunks;
}

int TorrentFileManager::getChunkSize() {
  return this->chunksize;
}

string TorrentFileManager::getChunkHash(int chunknum) {
  if (chunknum > this->numchunks || chunknum < 0) {
    cout << "Invalid chunk number passed into getChunkHash\n";
    return "";
  }
  else
    return this->chunkhashes[chunknum];
}


string TorrentFileManager::hashToHex(unsigned char* contents, int size) {
// Takes SHA1 hash of contents and returns hexadecimal representation of
// the 20 byte SHA1 hash
  unsigned char hash[20];
  string shash;
  char hexhash[40];

  SHA1((unsigned char*)contents, size, hash);

  for (int i=0; i<20; i++) {
    sprintf(hexhash+i*2, "%02x", hash[i]);
  }
  
  for (int i=0; i<40; i++) {
    shash += hexhash[i];
  }
  
  return shash;
}


int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "Usage: TorrentFileManager <filename>\n";
    return 1;
  }
  string filename(argv[1]);
  TorrentFileManager tfilem(filename); 
  return 0;
}
