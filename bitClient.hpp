#ifndef _BitClient_h_guard_
#define _BitClient_h_guard_

#include "face.hpp"
#include "data.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include "ChunkManager.hpp"
#include "TorrentFileManager.hpp"

using namespace ndn;

class BitClient {
  public:
    BitClient(string filename);
    void run();

  private:
    void onData(const Interest& interest, const Data&data);
    void onTimeout(const Interest& interest);

    Face m_face;
    string filename;
    ChunkManager chunk_manager;
    TorrentFileManager tfile_manager;
    
};

#endif
