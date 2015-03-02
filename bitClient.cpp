#include "face.hpp"
#include <string>
#include <iostream>
#include <fstream>

#include "BitClient.hpp"

using namespace std;

BitClient(const char* filename) {
  chunk_manager = ChunkManager(name);
  m_filename = name; 
  return;
}

void run() {
  int num_chunks;
  int finished;
  string hash;

  num_chunks = chunk_manager.getNumChunks();
  hash = 
  finished = 0;
 
  while (!finished) {
    finished = 1;
    for (int i=0; i<num_chunks; i++) {
      Interest interest(Name("/tracker/BitTorrent/" + hash + "/" + to_string(i)));
      interest.setInterestLifetime(time::milliseconds(1000));
      interest.setMustBeFresh(true);

      m_face.expressInterest(interest,
                             bind(&BitClient::onData, this, _1, _2),
                             bind(&BitClient::onTimeout, this, _1));

      cout << "Sending " << interest << endl;
      m_face.processEvents();
    }
  }
}

void onData(const Interest& interest, const Data&data) {
  const Block b = data.getContent();
  const uint8_t* content = b.value();
  for (size_t i = 0; i < b.value_size(); i++)
    cout << content[i];
  cout << endl;
}

void onTimeout(const Interest& interest) {
  cout << "Timeout " << interest << endl;
}



/*
int main (int argc, char** argv) {
  ndn::BitClient client;
  if (argc < 3) {
    cout << "Usage: [argv1=metadata_hash] [argv2=client_name]" << endl;
    cout << "Example: ./BitClient 123 /A" << endl;
    return 0;
  }
  try {
    client.run(argv[1], argv[2]);
  }
  catch (const exception& e) {
    cerr << "ERROR: " << e.what() << endl;
  }
  return 0;
}
*/
