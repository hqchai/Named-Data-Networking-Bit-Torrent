#include "face.hpp"
#include "security/key-chain.hpp"

using namespace std;
const int MAX_CLIENT_NUM = 20000;

namespace ndn {

class BitTracker {
  public:
    void run() {
      m_face.setInterestFilter("/tracker/BitTorrent",
                               bind(&BitTracker::onInterest, this, _1, _2),
                               RegisterPrefixSuccessCallback(),
                               bind(&BitTracker::onRegisterFailed, this, _1, _2));
      m_face.processEvents();
    }

  private:
    string getPeerList(string name) {
      client_names[client_num] = name;
      client_num++;
      string peerList = "";
      for (size_t i = 0; i < client_num; i++)
        peerList += client_names[i] + '\n';
      return peerList;
    }

    void onInterest(const InterestFilter& filter, const Interest& interest) {
      cout << "<< I: " << interest << endl;

      Name dataName(interest.getName());
      Name clientName = dataName.getSubName(3, Name::npos);
      string content;
      if (client_num >= MAX_CLIENT_NUM)
        content = "Peer List Full.";
      else
        content = getPeerList(clientName.toUri());

      shared_ptr<Data> data = make_shared<Data>();
      data->setName(dataName);
      data->setFreshnessPeriod(time::seconds(10));
      data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());

      m_keyChain.sign(*data);
      cout << ">> D: " << *data << endl;
      m_face.put(*data);
    }

    void onRegisterFailed(const Name& prefix, const string&reason) {
      cerr << "ERROR: Failed to register prefix \""
           << prefix << "\" in local hub's daemon {" << reason << ")"
           << endl;
      m_face.shutdown();
    }

    Face m_face;
    KeyChain m_keyChain;
    size_t client_num = 0;
    string client_names[MAX_CLIENT_NUM];
};

}

int main(int argc, char** argv)
{
  ndn::BitTracker tracker;
  try {
    tracker.run();
  }
  catch (const exception& e) {
    cerr << "ERROR: " << e.what() << endl;
  }
  return 0;
}
