#include "BitClient.hpp"

using namespace std;

BitClient(const char* filename) {
  chunk_manager = ChunkManager(name);
  tfile_manager = TorrentFileManager(string(filename));
  m_filename = name; 
  return;
}

void run() {
  int num_chunks;
  int finished;
  string hash;

  num_chunks = tfile_manager.getNumChunks();
  hash = tfile_manager.getFilehash();
  finished = 0;
 
  while (!finished) {
    finished = 1;
    for (int i=0; i<num_chunks; i++) {
      // Check if we already have the file chunk      
      if (chunk_manager.chunkAvailable((long)i)) {
        continue;  
      } 
      else {
        finished = 0;

        Interest interest(Name("/tracker/BitTorrent/" + hash + "/" + to_string(i)));
        interest.setInterestLifetime(time::milliseconds(1000));
        interest.setMustBeFresh(true);

        m_face.expressInterest(interest,
                               bind(&BitClient::onData, this, _1, _2),
                               bind(&BitClient::onTimeout, this, _1));

        cout << "Sending " << interest << endl;
      }
    }
    m_face.processEvents();
  }
}

void onData(const Interest& interest, const Data&data) {
  cout << "<< Received " << interest << endl;

  const Blob b = data.getContent();

  // Hash the data received and make sure it matches
  /* HASH CODE */
  
  // Write the data to disk
  Name interestName(interest.getName());
  string chunkNum = interestName.getSubname(3, Name::npos).toUri();
  chunk_manager.writeChunk(chunkNum, (char*)b.buf());

  const uint8_t* content = b.value();
  for (size_t i = 0; i < b.value_size(); i++)
    cout << content[i];
  cout << endl;

  return;
}

void onTimeout(const Interest& interest) {
  cout << "Timeout " << interest << endl;
  return;
}



int main (int argc, char** argv) {
  ndn::BitClient client;
  if (argc < 3) {
    cout << "Usage: [argv1=filename]" << endl;
    cout << "Example: ./BitClient foo.txt" << endl;
    return 0;
  }
  try {
    client.run(argv[1]);
  }
  catch (const exception& e) {
    cerr << "ERROR: " << e.what() << endl;
  }
  return 0;
}
