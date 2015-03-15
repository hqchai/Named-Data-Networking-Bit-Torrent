#ifndef _BitClient_h_guard_
#define _BitClient_h_guard_

#include "face.hpp"
#include "data.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include "ChunkManager.hpp"
#include "TorrentFileManager.hpp"
#include "security/key-chain.hpp"

using namespace ndn;
using namespace std;

inline bool exists_file (const string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

class BitClient {
  public:
    BitClient(string filename);
    void run();

  private:
    void onData(const Interest& interest, const Data&data);
    void onTimeout(const Interest& interest);
    void onInterest(const InterestFilter& filter, const Interest& interest);
    void onRegisterFailed(const Name& prefix, const string&reason);

    Face m_face;
    string filename;
    TorrentFileManager tfile_manager;
    ChunkManager chunk_manager;
    KeyChain m_keyChain;
    map<string, string> m_map;
};

#endif
