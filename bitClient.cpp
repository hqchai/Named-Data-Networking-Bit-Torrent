#include "face.hpp"
#include <string>
#include <iostream>
#include <istream>
#include <iostream>
#include <fstream>

using namespace std;

// Directory to store downloaded files
static string DOWNLOADPATH = "./Downloads";
// Directory to keep chunks of downloaded files
static string CHUNKPATH = "./Downloads/.chunks";
// temp global
static int NUMCHUNKS = 10;

int get_filesize(fstream& file);

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

    // TODO Make metafile as input 
    void scatter(const string filename) {
    // Given a file, creates n chunks (specified in metadata file)
    // Places chunks in the application's chunks directory
    // Chunks are given name "filename.chunk_x" where x is the chunk number
      int nchunks;
      ofstream filechunk;
      int chunksize;  // size of chunk in bytes
      int filesize;
      string filechunk_path;
      char* data_buffer;
      int bytes_read;

      // TODO Read metafile for nchunks 
      nchunks = NUMCHUNKS;

      // Open file for reading
      ifstream file;
      string filepath = DOWNLOADPATH + filename;
      file.open(filepath);
      if (!file) {
        // TODO Make exception?
        cout << "Warning: Can't open file for scattering" << endl;
        return;
      }
      filesize = get_filesize(file);

      // Calculate the size of each chunk
      // TODO Get this from metafile
      chunksize = filesize/nchunks;  
      if (filesize%nchunks != 0) {
        // If the filesize doesn't divide evenly with nchunks, we need to 
        // make the chunks larger and have a smaller final chunk
        chunksize++;  
      } 

      // Write each file chunk
      data_buffer = new char[chunksize];
      for (int i=0; i<nchunks; i++) {
        // Open new file chunk
        filechunk_path = CHUNKPATH + filename + ".chunk_" + to_string(i);
        filechunk.open(filechunk_path);

        // Read data from file into buffer
        file.seekg(i*chunksize);
        file.read(data_buffer, chunksize);
        
        // Confirm number of bytes read
        bytes_read = file.gcount();

        // Write the data to file chunk
        filechunk.write(data_buffer, bytes_read);
        filechunk.close();
      }

      delete[] data_buffer;
      return;

    }

    // TODO take metafile as input to get nchunks & others
    void combine(string fileprefix) {
    // Combines all chunks in chunks directory with given fileprefix
    // Combined file will be placed in application's downloads directory
    // Example file chunks: "spiderman2.avi.chunk_0, spiderman2.avi.chunk_1"
    // Prefix will be "spiderman2.avi"
    // Combined filename will be called spiderman2.avi
      int nchunks = NUMCHUNKS;
      int chunksize;
 
      // TODO
      // Check if directories exist and have permissions
      // Check if all chunks exist

      // Open new file for writing
      // TODO Handle if file exists
      ofstream output_file;
      ifstream chunk_file;
      string chunk_filepath;
      char* data_buffer;
      int bytes_read;

      output_file.open(fileprefix);

      // Calculate the size of each chunk
      // TODO Get this from metafile
      chunksize = filesize/nchunks;  
      if (filesize%nchunks != 0) {
        // If the filesize doesn't divide evenly with nchunks, we need to 
        // make the chunks larger and have a smaller final chunk
        chunksize++;  
      } 
      data_buffer = new char[chunksize];

      // Open each chunk file and write to output file
      for (int i=0; i<nchunks; i++) {
        // TODO Checksums, confirm read all bytes, confirm chunks all exist
        // Read in data
        chunk_filepath = fileprefix + ".chunk_" + to_string(i);
        chunk_file.open(chunk_filepath);
        chunk_file.read(data_buffer, chunksize);

        // Confirm number of bytes read
        bytes_read = chunk_file.gcount();
        
        // Write the data to file chunk
        output_file.seekg(i*chunksize);
        output_file.write(data_buffer, bytes_read);
        chunk_file.close();
      } 
      return;
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


int get_filesize(fstream& file) {
// Returns the size of given file
  file.seekg(0, file.end);
  int size = file.tellg();
  file.seekg(0, file.beg);
  return size;
}


int main (int argc, char** argv) {
  ndn::BitClient client;
  client.scatter("test.pi");
  return 0;
}

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
