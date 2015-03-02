#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h> 
// A torrent file consists of key: value pairs of information
// relevant to the torrent.  It contains:
// The file's name
// The size of the file in bytes
// The SHA1 hash of the file
// The number of chunks for the file
// The size of each chunk
// The SHA1 hashes of each chunk
// Here is an example metadata file

// filename:foo
// filesize:2048
// filehash:cf23df2207d99a74fbe169e3eba035e633b65d94
// numchunks:2
// chunksize:1024
// hash0:3idkn23a3ao3049kmbe169e3eke9dke633b65d94
// hash1:5o1km19c511u0a74faefae0f39adfi2933b65si2

// Note: The last chunk of a file may be smaller than the chunksize
// Use the ChunkManager class to manage chunks


class TorrentFileManager
{
  public:
    TorrentFileManager(std::string filename);
    std::string getFilename();
    int getFilesize();
    std::string getFilehash();
    int getNumChunks();
    int getChunkSize();
    std::string getChunkHash(int chunknum);

  private:
    void createTorrentFile(std::string filename);
    void setFromTorrentFile(std::string filename);

    std::string filename;
    int filesize;
    std::string filehash;
    int numchunks;
    int chunksize;
    std::string* chunkhashes;
};

