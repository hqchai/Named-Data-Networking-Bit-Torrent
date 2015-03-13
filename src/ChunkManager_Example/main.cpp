#include <string.h>
#include "ChunkManager.hpp"

int main() {
    //Creates src.txt and dst.txt and bitmap files
    //Copies chunk at index 1 from src to dst
    
    string srcFile = "src.txt";
    ChunkManager* src = new ChunkManager(srcFile.c_str(), 20, 2);
    src->writeChunk(0, "he");
    src->writeChunk(1, "ll");
    src->writeChunk(2, "o!");
    char* data = new char[src->getChunkSize()+1];
    src->readChunk(1, data);
    
    string dstFile = "dst.txt";
    ChunkManager* dst = new ChunkManager(dstFile.c_str(), 20, 2);
    dst->writeChunk(0, "he");
    dst->writeChunk(1, data);
    dst->writeChunk(2, "o?");
    
    delete src;
    delete dst;
}
