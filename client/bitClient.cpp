#include "bitClient.hpp"
#include "ChunkManager.cpp"
#include "TorrentFileManager.cpp"

static int CLIENT_DEBUG = 0;
static int LISTEN_DEBUG = 0;

const string configure_file = "listenerfiles.list";

string getBitstring(string filename);
void add_entry(string file_name);
void delete_entry(string delete_name);

BitClient::BitClient(string filename) 
{
  tfile_manager = new TorrentFileManager(filename);
  chunk_manager = new ChunkManager(filename.c_str(), tfile_manager->getFilesize(), tfile_manager->getChunkSize()); 
  this->filename = filename;
  return;
} 

BitClient::BitClient() 
{
  tfile_manager = NULL;
  chunk_manager = NULL;
  this->filename = "";
  return;
}

BitClient::~BitClient() {
  delete tfile_manager;
  delete chunk_manager;
}


void BitClient::run(bool seedonly) {

  // Setup leeching
  if (!seedonly) {
    int num_chunks;
    string hash;

    hash = tfile_manager->getFilehash();
    num_chunks = this->tfile_manager->getNumChunks();
    add_entry(this->filename);

    for (int i=0; i<num_chunks; i++) {
      // Check if we already have the file chunk      
      if (this->chunk_manager->chunkAvailable((long)i)) {
        cout << "Have chunk " << i << endl;
      } 
      else {
        Interest interest(Name("/BitTorrent/" + hash + "/" + to_string(i)));
        interest.setInterestLifetime(time::milliseconds(10000));
        interest.setMustBeFresh(true);

        m_face.expressInterest(interest,
                               bind(&BitClient::onData, this, _1, _2),
                               bind(&BitClient::onTimeout, this, _1));

        cout << "Sending " << interest << endl;
      }
    }
  }

  // Setup seeding
  ifstream infile(configure_file);
  string line;
  while (getline(infile, line)) {
    string ndn_name, file_name;

    ndn_name = line.substr(0, line.find(" "));
    file_name = line.substr(line.find(" ") + 1);
    if (exists_file(file_name)) {
      string bitString = getBitstring(file_name);
      m_map[ndn_name] = file_name;
      for (size_t i = 0; i < bitString.length(); i++) {
        if (bitString[i] == '1') {
          m_face.setInterestFilter("BitTorrent/" + ndn_name + "/" + to_string(i),
                             bind(&BitClient::onInterest, this, _1, _2),
                             RegisterPrefixSuccessCallback(),
                             bind(&BitClient::onRegisterFailed, this, _1, _2));
          cout << "Setting interest filter: BitTorrent/" << ndn_name + "/" + to_string(i) << endl;
          
        }
      }
    }
    // If the file doesn't exist, delete it from the listener list
    else {
     delete_entry(file_name);
    }
  }

  // Process events
  m_face.processEvents();

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
  this->chunk_manager->writeChunk((long)stoi(chunkNum), (char*)b.value());

/*
  const uint8_t* content = b.value();
  for (size_t i = 0; i < b.value_size(); i++)
    cout << content[i];
  cout << endl;
*/

  // Send interest filter for newly downloaded chunk 
  m_face.setInterestFilter(interest.getName().toUri(),
                     bind(&BitClient::onInterest, this, _1, _2),
                     RegisterPrefixSuccessCallback(),
                     bind(&BitClient::onRegisterFailed, this, _1, _2));
  cout << "Setting interest filter: " << interest.getName().toUri() << endl;

  return;
}

void BitClient::onTimeout(const Interest& interest) {
  cout << "Timeout " << interest << endl;

  // Resend interest
  Name interestName(interest.getName());
  
  string hash = interestName.getSubName(1, Name::npos).toUri();
  hash = hash.substr(0, hash.find_last_of("/"));
  hash= hash.substr(1);

  string chunkNum = interestName.getSubName(2, Name::npos).toUri();
  chunkNum = chunkNum.substr(1); 

  Interest reinterest(Name("/BitTorrent/" + hash + "/" + chunkNum));
  reinterest.setInterestLifetime(time::milliseconds(10000));
  reinterest.setMustBeFresh(true);

  this->m_face.expressInterest(interest,
  		     bind(&BitClient::onData, this, _1, _2),
  		     bind(&BitClient::onTimeout, this, _1));


  cout << "Resending " << interest << endl;

  return;
}


void BitClient::onInterest(const InterestFilter& filter, const Interest& interest) {
  cout << "Received interest " << interest << endl;

  Name interestName(interest.getName());
  string chunkNum = interestName.getSubName(2, Name::npos).toUri();
  chunkNum = chunkNum.substr(1, chunkNum.length()-1);
  string dataName = interestName.getSubName(1, Name::npos).toUri();
  dataName = dataName.substr(0, dataName.find_last_of("/"));
  dataName = dataName.substr(1);
  if (m_map.count(dataName) < 1) {
    cout << "Error: Key does not exist\n";
  }
  string file_name = m_map.find(dataName)->second;

  TorrentFileManager tfm(file_name.c_str());

  ChunkManager chunkManager(file_name.c_str(), tfm.getFilesize(), tfm.getChunkSize());
  char* temp = new char[chunkManager.getChunkSize()];
  int bytes_read = chunkManager.readChunk(stoi(chunkNum), temp);
  string content(temp, bytes_read);

  if (LISTEN_DEBUG) {
    cout << "Bytes read: " << bytes_read << endl;
    cout << "Content in char array: ";
    for (int i=0; i<bytes_read; i++) {
      cout << temp[i];
    }
    cout << endl;
  }

  shared_ptr<Data> data = make_shared<Data>();
  data->setName(interestName);
  data->setFreshnessPeriod(time::seconds(10));
  data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());

  m_keyChain.sign(*data);
  m_face.put(*data);

  delete temp;

}

void BitClient::onRegisterFailed(const Name& prefix, const string&reason) {
  cerr << "ERROR: Failed to register prefix \""
       << prefix << "\" in local hub's daemon {" << reason << ")"
       << endl;
  m_face.shutdown();
}

void list_entries() {
// entry is of format <hash> <filename>
// example entry:
// e72b5e6dae3dd602ff305eb737598b3b3ef6486c foo.txt
  ifstream infile(configure_file);
  string line;

  if (!infile.is_open()) {
    cout << configure_file << " doesn't exist\n";
  }
  else {
    while (getline(infile, line))
      cout << line << endl;
  }
}


void delete_entry(string delete_name) {
// Calculates hash of filename given in delete_name
// If the hash matches any in the listeners file, the entry is deleted

  //string filehash = TorrentFileManager(delete_name).getFilehash();
  string tempfile = ".tmp.txt";
  ifstream names(configure_file);
  ofstream temp(tempfile);
  int found_name = 0;

  string name, file_name;
  while (names >> name >> file_name) {
    if (delete_name != file_name) {
      temp << name << ' ' << file_name << endl;
    } 
    else
      found_name = 1;
  }
  names.clear();
  names.seekg(0, ios::beg);
  names.close();
  temp.close();
  remove(configure_file.c_str());
  rename(tempfile.c_str(), configure_file.c_str());

  if (found_name == 0)
    cout << delete_name << " not found in list" << endl;
  if (found_name == 1)
    cout << delete_name << " deleted from list" << endl;
}


bool entry_exists(string filename) {
// Calculates hash of the file given by filename
// If any entries matches the hash, returns true
  //string filehash = TorrentFileManager(filename).getFilehash();
  ifstream names(configure_file);
  string name, file_name;
  while (names >> name >> file_name) {
    if (file_name == filename)
      return true;
  }
  return false;
}


void add_entry(string file_name) {
// Automatically calculates hash and adds entry
  if (!entry_exists(file_name)) {
    string filehash = TorrentFileManager(file_name).getFilehash();
    ofstream outfile(configure_file, ios::app);
    outfile << filehash << " " << file_name << endl;
    cout << file_name << " added to list\n";
  }
}


void print_usage() {
   cout << "Usage: ./bitClient command [options]" << endl;
   cout << "Commands:\n";
   cout << "\tlist\tlists all files server is listening for\n";
   cout << "\tdelete <filename>\tdelete the file from listening list\n";
   cout << "\tadd <filename>\tadd the file to listening list\n";
   cout << "\tmake <filename>\tcreate a torrent file for filename\n";
   cout << "\tseed\tstart seeding all available files in listening list\n";
   return;
}

string getBitstring(string filename) {
  ChunkManager cm(filename.c_str());
  return cm.getBitstring();
}


int main(int argc, char** argv)
{
  if (argc < 2) {
    print_usage();
    return 0;
  }


  string list_s = "list";
  string delete_entry_s = "delete";
  string add_entry_s = "add";
  string make_s = "make";
  string seed_s = "seed";

  bool seedonly = false;

  if (argc == 2 && seed_s.compare(argv[1]) == 0) {
    seedonly = true;
  }

  if (argc == 2 && list_s.compare(argv[1]) == 0) {
    list_entries();
  }
  else if (argc == 3 && delete_entry_s.compare(argv[1]) == 0) {
    delete_entry(argv[2]);
  }
  else if (argc == 3 && add_entry_s.compare(argv[1]) == 0) {
    add_entry(argv[2]);
  }
  else if (argc == 3 && make_s.compare(argv[1]) == 0) {
    TorrentFileManager tfm(argv[2]);
  }
  else {
    try {
      if (seedonly) {
        BitClient client;
        client.run(seedonly);
      }
      else {
        string filename = argv[1];
        // Remove '.torrent' at end
        filename = filename.substr(0,filename.size()-8);
        BitClient client(filename);
        client.run(seedonly);
      }
    }
    catch (const exception& e) {
      cerr << "ERROR: " << e.what() << endl;
    }
  }

  return 0;
}
