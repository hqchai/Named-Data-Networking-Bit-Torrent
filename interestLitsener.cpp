#include "face.hpp"
#include "security/key-chain.hpp"
#include "ChunkManager.cpp"
#include <map>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

const string configure_file = "test.txt";

using namespace std;

inline bool exists_file (const string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

inline string get_bitString (const string& name) {
  if (!exists_file(name))
    return "";
  ifstream infile(name);
  string line;
  getline(infile, line);
  getline(infile, line);
  if (!getline(infile, line))
    return "";
  return line;
}

namespace ndn {

class InterestListener {
  public:
    void run() {
      ifstream infile(configure_file);
      string line;
      while (getline(infile, line)) {
        string ndn_name, file_name;
        ndn_name = line.substr(0, line.find(" "));
        file_name = line.substr(line.find(" ") + 1);
        if (exists_file(file_name)) {
          string bitString = get_bitString(file_name + ".meta");
          m_map[ndn_name] = file_name;
          for (size_t i = 0; i < bitString.length(); i++)
            if (bitString[i] == '1')
              m_face.setInterestFilter(ndn_name + "/" + to_string(i),
                                 bind(&InterestListener::onInterest, this, _1, _2),
                                 RegisterPrefixSuccessCallback(),
                                 bind(&InterestListener::onRegisterFailed, this, _1, _2));
        }
        else
         delete_entry(ndn_name, true);
      }
      m_face.processEvents();
    }

    void list() {
      ifstream infile(configure_file);
      string line;
      while (getline(infile, line))
        cout << line << endl;
    }

    void delete_entry(string delete_name, bool silence) {
      ifstream names(configure_file);
      ofstream temp("temp.txt");
      int found_name = 0;

      string name, file_name;
      while (names >> name >> file_name) {
        if (delete_name != name) {
          temp << name << ' ' << file_name << endl;
        } else
	  found_name = 1;
      }
      names.clear();
      names.seekg(0, ios::beg);
      names.close();
      temp.close();
      remove(configure_file.c_str());
      rename("temp.txt", configure_file.c_str());
      if (!silence && found_name == 0)
        cout << "Couldn't find such name." << endl;
      if (!silence && found_name == 1)
        cout << "Name deleted." << endl;
    }

    void add_entry(string add_name, string file_name) {
      ofstream outfile(configure_file, ios::app);
      outfile << add_name << " " << file_name << endl;
    }

  private:
    void onInterest(const InterestFilter& filter, const Interest& interest) {
      cout << "<< I: " << interest << endl;

      Name interestName(interest.getName());
      string chunkNum = interestName.getSubName(3, Name::npos).toUri();
      chunkNum = chunkNum.substr(1, chunkNum.length()-1);
      string dataName = interestName.toUri();
      dataName = dataName.substr(0, dataName.find_last_of("/"));
      string file_name = m_map.find(dataName)->second;

      char* buffer;
      buffer = (char*) malloc (file_name.length()+1);
      strcpy(buffer, file_name.c_str());
      ChunkManager chunkManager(buffer);
      char temp[10];
      int bytes_read = chunkManager.readChunk(atoi(chunkNum.c_str()), temp);
      string content(temp, bytes_read);

      shared_ptr<Data> data = make_shared<Data>();
      data->setName(interestName);
      data->setFreshnessPeriod(time::seconds(10));
      data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());

      m_keyChain.sign(*data);
      cout << ">> D: " << *data << endl;
      m_face.put(*data);

      free(buffer);
    }

    void onRegisterFailed(const Name& prefix, const string&reason) {
      cerr << "ERRPR: Failed to register prefix \""
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

int main(int argc, char** argv)
{
  ndn::InterestListener listener;
  /*if (argc < 2) {
     cout << "Usage: [argv=path_to_file_list]" << endl;
     cout << "Example: ./interestLitsener ./list.txt";
     return 0;
  }*/
  string list = "list";
  string delete_entry = "delete";
  if (argc == 2 && list.compare(argv[1]) == 0) {
    listener.list();
  }
  else 
  if (argc == 3 && delete_entry.compare(argv[1]) == 0) {
    listener.delete_entry(argv[2], false);
  }
  else {
    try {
      listener.run();
    }
    catch (const exception& e) {
      cerr << "ERROR: " << e.what() << endl;
    }
  }
  return 0;
}
