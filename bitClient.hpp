#include "face.hpp"
#include <string>
#include <iostream>

class BitClient {
  public:
    BitClient(const char* metafile);
    void run(const string hash, const string name);

  private:
    void onData(const Interest& interest, const Data&data);
    void onTimeout(const Interest& interest);

    Face m_face;
    string filename;
    ChunkManager chunk_manager;
    
};
