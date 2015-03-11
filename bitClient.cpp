#include "bitClient.hpp"
#include "ChunkManager.cpp"
#include "TorrentFileManager.cpp"

using namespace std;
using namespace ndn;

BitClient::BitClient(string filename) 
  : chunk_manager(filename.c_str()), 
    tfile_manager(filename)
{
  
  this->filename = filename; 
  return;
} 

void BitClient::run() {
  int num_chunks;
  int finished;
  string hash;

  hash = tfile_manager.getFilehash();
  num_chunks = this->tfile_manager.getNumChunks();
  finished = 0;
 
  while (!finished) {
    finished = 1;
    for (int i=0; i<num_chunks; i++) {
      // Check if we already have the file chunk      
      if (this->chunk_manager.chunkAvailable((long)i)) {
        continue;  
      } 
      else {
        finished = 0;

        Interest interest(Name("/BitTorrent/" + hash + "/" + to_string(i)));
        interest.setInterestLifetime(time::milliseconds(1000));
        interest.setMustBeFresh(true);

        this->m_face.expressInterest(interest,
                               bind(&BitClient::onData, this, _1, _2),
                               bind(&BitClient::onTimeout, this, _1));

        cout << "Sending " << interest << endl;
      }
    }
    this->m_face.processEvents();
  }
}

void BitClient::onData(const Interest& interest, const Data&data) {
  cout << "<< Received " << interest << endl;

  const Block b = data.getContent();

  // Hash the data received and make sure it matches
  /* HASH CODE */
  
  // Write the data to disk
  Name interestName(interest.getName());
  string chunkNum = interestName.getSubName(2, Name::npos).toUri();
  this->chunk_manager.writeChunk((long)stoi(chunkNum), (char*)b.value());

  const uint8_t* content = b.value();
  for (size_t i = 0; i < b.value_size(); i++)
    cout << content[i];
  cout << endl;

  return;
}

void BitClient::onTimeout(const Interest& interest) {
  cout << "Timeout " << interest << endl;
  return;
}


/*
int main (int argc, char** argv) {
  if (argc < 3) {
    cout << "Usage: [argv1=torrent filename]" << endl;
    cout << "Example: ./BitClient foo.txt.torrent" << endl;
    return 0;
  }
  string filename = argv[1];
  // Remove '.torrent' at end
  filename = filename.substr(0,filename.size()-8);

  BitClient client(filename);

  try {
    client.run();
  }
  catch (const exception& e) {
    cerr << "ERROR: " << e.what() << endl;
  }
  return 0;
}
*/
