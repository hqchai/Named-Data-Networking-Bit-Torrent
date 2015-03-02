#include "Metafile.hpp"

using namespace std;

int DEFAULT_CHUNK_SIZE = 256*1024;

Metafile::Metafile(string filename) {
// Checks if metadata file for file already exists
// If it doesn't it initializes a new one and sets variables
// Otherwise, it will read the existing metafile and set variables

  // Check if file exists
  ifstream metafile;
  string metafilename = filename + ".meta";

  metafile.open(metafilename, ifstream::in);
  if (metafile.fail) {
    // If metafile doesn't exist, create a new one    
    metafile.close();
    this->createMetafile(filename, metafile);
  }
  else {
    // Otherwise, read in values and set variables
    this->setFromMetafile(metafile);
  }
  metafile.close();
  return;
}

void Metafile::createMetafile(string filename, std::ifstream mfilestream) {
// Analyzes contents of file and writes to metafilestream  
// Sets filename filesize filehash numchunks chunksize chunkhashses
// Must reopen metafilestream
  ifstream fstream;
  fstream.open(filename, ifstream::in);
 
  this->filename = filename; 

  // Get file size
  fstream.seekg (0, stream.end);
  this->filesize = fstream.tellg();
  fstream.seekg (0, fstream.beg);

  // Calculate SHA1 hash of file
	char * filebuf = new char[this->filesize];
  char hash[20];

	fstream.read(filebuf, this->filesize);
  SHA1(filebuf, this->filesize, hash);
  this->filehash(hash, 20);

	// Default chunk size is 256KiB
	this->chunk_size = 256*1024;

  // Calculate number of chunks and allocate memory for chunk hashes
  this->numchunks = filesize/chunksize;
  if (filesize%chunksize != 0) {    // If filesize does not divide evenly, we need another chunk at end
    this->numchunks++;
  }
  this->chunkhashes = new char*[numchunks]; for (int i=0; i<this->numchunks; i++) {
    this->chunkhashes[i] = new char[20]; 
  }

  // Hash each chunk	
  char* chunkbuf = new char[this->chunksize];
	char chunkhash char[20];

  for (int i=0; i<this->filesize; i+=this->chunksize) {
    chunkbuf = &filebuf[i];
		SHA1(chunkbuf, this->chunksize, chunkhash);
    this->chunkhashes[i](chunkhash, 20);
  }

  // Initialize bitstring to all 0's
  this->bitstring = new char[numchunks];
  for (int i=0; i<this->numchunks; i++) {
    this->bitstring[i] = "0";
  }

  // Write everything to metafile
  string line;
  int linelength;

  metafile.open(filename + ".meta", ifstream::out);
    
  line = "filename: " + this->filename + "\n";
  metafile.write(line.c_str(), line.length());

  line = "filehash: " + this->filehash + "\n";
  metafile.write(line.c_str(), line.length());

  line = "numchunks: " + this->numchunks + "\n";
  metafile.write(line.c_str(), line.length());

  line = "chunksize: " + this->chunksize + "\n";
  metafile.write(line.c_str(), line.length());

  for (int i=0; i<numchunks; i++) {
    line = "hash" + i + ":" + this->chunkhash[i] + "\n";
    metafile.write(line.c_str(), line.length());
  }
  
  string bstring(this->bitstring, numchunks);
  line = "bitstring: " + bstring + "\n";
  metafile.write(line.c_str(), line.length());

  // Free buffers and return
  free(filebuf);
  free(chunkbuf);
  close(fstream);
	return;
}

void setFromMetafile(string filename, ifstream mfilestream) {
  
}

 
/*
  // Open the file for reading
  std::ifstream ifs;

  ifs.open("filename", std::ifstream::in);
  if (!ifs.is_open()) {
    std::cout << "Couldn't open " + filename + "for reading\n";
    return;
  }

  //  Read each variable in
  string line;

  this->filename = filename;
  while (!ifs.eos()) {
    std::getline(ifs, line);
    
  }

}

*/
