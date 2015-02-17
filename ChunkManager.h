#ifndef CHUNK_MANAGER
#define CHUNK_MANAGER

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
using namespace std;

class ChunkManager {
    public:
        //Creates metafile and file
        ChunkManager(const char* filename, const long fileSize, const long chunkSize);
        //Reads existing metafile
        ChunkManager(const char* filename);
        ~ChunkManager();
        long readChunk(const long chunkNum, char* data);
        long writeChunk(const long chunkNum, char* data);
        long getNumChunks();
        long getChunkSize();
        long getLastChunkSize();
        
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