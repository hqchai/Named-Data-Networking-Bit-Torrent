#ifndef CHUNK_MANAGER
#define CHUNK_MANAGER

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
using namespace std;

class ChunkManager {
    public:
        //Use if bitmap file does not exists
        //If file does not exist, file is preallocated and bitmap is all 0's
        //If file does exists, assumed to be complete and bitmap is all 1's
        ChunkManager(const char* filename, const long fileSize, const long chunkSize);
        //Use if bitmap file and file exists
        ChunkManager(const char* filename);
        ~ChunkManager();
        long readChunk(const long chunkNum, char* data);
        long writeChunk(const long chunkNum, char* data);
        long getNumChunks(); //Total number of chunks
        long getChunkSize(); //Size of chunk, except last
        long getLastChunkSize(); //Size of last chunk
        bool* getChunks(); //Return pointer to bitmap
        bool chunkAvailable(const long chunkNum); //Yes/No if chunk has been downloaded
        string getBitstring();
        
    private:
        fstream m_fileStream;
        fstream m_metaFileStream;
        long m_lastChunkSize;
        long m_chunkSize;
        long m_numChunks;
        bool* m_chunks;
        
        void readMetaFile();
        void writeMetaFile();
};

#endif
