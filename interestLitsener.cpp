#include "face.hpp"
#include "security/key-chain.hpp"
#include "ChunkManager.cpp"
#include <map>
#include <fstream>

using namespace std;

namespace ndn {

class InterestListener {
  public:
    void run(string file_name) {
      ifstream infile(file_name);
      string line;
      while (getline(infile, line)) {
        string ndn_name, file_name;
        ndn_name = line.substr(0, line.find(" "));
        file_name = line.substr(line.find(" "));
        m_map[ndn_name] = file_name;
        m_face.setInterestFilter(ndn_name,
                                 bind(&InterestListener::onInterest, this, _1, _2),
                                 RegisterPrefixSuccessCallback(),
                                 bind(&InterestListener::onRegisterFailed, this, _1, _2));
      
      }
      m_face.processEvents();
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
      //string content(temp, bytes_read);
      string content = to_string(bytes_read);

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
  if (argc < 2) {
     cout << "Usage: [argv=path_to_file_list]" << endl;
     cout << "Example: ./interestLitsener ./list.txt";
     return 0;
  }
  try {
    listener.run(argv[1]);
  }
  catch (const exception& e) {
    cerr << "ERROR: " << e.what() << endl;
  }
  return 0;
}
