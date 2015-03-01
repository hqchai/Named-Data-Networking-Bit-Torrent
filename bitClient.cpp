#include "face.hpp"
#include <string>
#include <iostream>

using namespace std;

namespace ndn {

class BitClient {
  public:
    void run(const string hash, const string name) {
      Interest interest(Name("/tracker/BitTorrent/" + hash + name));
      interest.setInterestLifetime(time::milliseconds(1000));
      interest.setMustBeFresh(true);

      m_face.expressInterest(interest,
                             bind(&BitClient::onData, this, _1, _2),
                             bind(&BitClient::onTimeout, this, _1));

      cout << "Sending " << interest << endl;
      m_face.processEvents();
    }

  private:
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

    Face m_face;
};

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
