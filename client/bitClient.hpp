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

const string configure_file = "listenerfiles.list";
static int LISTEN_DEBUG= 1;

inline bool exists_file (const string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

inline string get_bitString (const string& filename) {
  ChunkManager cm(filename.c_str());
  return cm.getBitstring();
}

namespace ndn {

class InterestListener {
  public:

    void run(bool block) {
      ifstream infile(configure_file);
      string line;
      while (getline(infile, line)) {
        string ndn_name, file_name;

        ndn_name = line.substr(0, line.find(" "));
        file_name = line.substr(line.find(" ") + 1);
        if (exists_file(file_name)) {
          string bitString = get_bitString(file_name);
          m_map[ndn_name] = file_name;
          for (size_t i = 0; i < bitString.length(); i++) {
            if (bitString[i] == '1') {
              m_face.setInterestFilter("BitTorrent/" + ndn_name + "/" + to_string(i),
                                 bind(&InterestListener::onInterest, this, _1, _2),
                                 RegisterPrefixSuccessCallback(),
                                 bind(&InterestListener::onRegisterFailed, this, _1, _2));
              if (LISTEN_DEBUG) {
                cout << "Setting interest filter: BitTorrent/" << ndn_name + "/" + to_string(i) << endl;
              }
            }
          }
        }
        // If the file doesn't exist, delete it from the listener list
        else {
         delete_entry(file_name);
        }
      }
      if (!block)
        m_face.processEvents(time::milliseconds::min());
      else
        m_face.processEvents();
    }

    void stop() {
      m_face.shutdown();
    }

    void list() {
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
        } else
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
      }
      cout << file_name << " added to list\n";
    }

  private:
    void onInterest(const InterestFilter& filter, const Interest& interest) {
      if (LISTEN_DEBUG)
        cout << "<< I: " << interest << endl;

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
      cout << ">> D: " << *data << endl;
      m_face.put(*data);

      delete temp;

    }

    void onRegisterFailed(const Name& prefix, const string&reason) {
      cerr << "ERROR: Failed to register prefix \""
           << prefix << "\" in local hub's daemon {" << reason << ")"
           << endl;
      m_face.shutdown();
    }
    
    Face m_face;
    KeyChain m_keyChain;
    //ChunkManager m_chunkManager;
    map<string, string> m_map;
};

}

class BitClient {
  public:
    BitClient(string filename);
    void run();

  private:
    void onData(const Interest& interest, const Data&data);
    void onTimeout(const Interest& interest);

    Face m_face;
    string filename;
    InterestListener listener;
    TorrentFileManager tfile_manager;
    ChunkManager chunk_manager;
};

#endif
