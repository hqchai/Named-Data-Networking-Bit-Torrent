#ifndef CHUNK_MANAGER
#define CHUNK_MANAGER

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include "TorrentFileManager.hpp"

using namespace std;

class ChunkManager {
    public:
        //Use if bitmap file does not exists
        //If file does not exist, file is preallocated and bitmap is all 0's
        //If file does exists, assumed to be complete and bitmap is all 1's
        ChunkManager(const char* filename, const int fileSize, const int chunkSize);
        //Use if bitmap file and file exists
        ChunkManager(const char* filename);
        ~ChunkManager();
        int readChunk(const int chunkNum, char* data);
        int writeChunk(const int chunkNum, char* data);
        int getNumChunks(); //Total number of chunks
        int getChunkSize(); //Size of chunk, except last
        int getLastChunkSize(); //Size of last chunk
        bool* getChunks(); //Return pointer to bitmap
        bool chunkAvailable(const int chunkNum); //Yes/No if chunk has been downloaded
        string getBitstring();
        
    private:
        fstream m_fileStream;
        fstream m_metaFileStream;
        int m_lastChunkSize;
        int m_chunkSize;
        int m_numChunks;
        bool* m_chunks;
        
        void readMetaFile();
        void writeMetaFile();
};

#endif
