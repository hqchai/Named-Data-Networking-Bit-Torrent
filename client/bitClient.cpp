#include "bitClient.hpp"
#include "ChunkManager.cpp"
#include "TorrentFileManager.cpp"

static int CLIENT_DEBUG= 1;

BitClient::BitClient(string filename) 
  : tfile_manager(filename),
    chunk_manager(filename.c_str(), tfile_manager.getFilesize(), tfile_manager.getChunkSize())
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
 
  this->listener.add_entry(this->filename);
  this->listener.run(false);
  while (!finished) {
    finished = 1;
    for (int i=0; i<num_chunks; i++) {
      // Check if we already have the file chunk      
      if (this->chunk_manager.chunkAvailable((long)i)) {
        if (CLIENT_DEBUG) 
          cout << "Already have chunk " << i << endl;
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
  this->listener.stop();
  this->listener.run(true);
}

void BitClient::onData(const Interest& interest, const Data&data) {
  cout << "<< Received " << interest << endl;

  const Block b = data.getContent();

  // Hash the data received and make sure it matches
  /* HASH CODE */
  
  // Write the data to disk
  Name interestName(interest.getName());
  string chunkNum = interestName.getSubName(2, Name::npos).toUri();
  chunkNum = chunkNum.substr(1); 
  cout << "chunkNum: " << chunkNum << endl;
  this->chunk_manager.writeChunk((long)stoi(chunkNum), (char*)b.value());

  const uint8_t* content = b.value();
  for (size_t i = 0; i < b.value_size(); i++)
    cout << content[i];
  cout << endl;

  this->listener.stop();
  this->listener.run(false);
  return;
}

void BitClient::onTimeout(const Interest& interest) {
  cout << "Timeout " << interest << endl;
  return;
}



int main (int argc, char** argv) {
  if (argc < 2) {
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

