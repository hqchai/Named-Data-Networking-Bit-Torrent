#include <string.h>
#include "ChunkManager.hpp"
#include "TorrentFileManager.hpp"

int main() {
    //Creates src.txt and dst.txt and bitmap files
    //Copies chunk at index 1 from src to dst
    
    string srcFile = "src.txt";
    string dstFile = "dst.txt";

    TorrentFileManager tfm(srcFile);

    ChunkManager* src = new ChunkManager(srcFile.c_str(), tfm.getFilesize(), tfm.getChunkSize());
    ChunkManager* dst = new ChunkManager(dstFile.c_str(), tfm.getFilesize(), tfm.getChunkSize());

    char* data = new char[src->getChunkSize()+1];
    for (int i=0; i<tfm.getNumChunks(); i++) {
      src->readChunk(i, data);
      dst->writeChunk(i, data);
    }
    
    delete src;
    delete dst;
}
