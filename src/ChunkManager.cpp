#include "ChunkManager.hpp"

static int CHUNKMANAGER_DEBUG = 0;

bool checkFileExists(const std::string& filename);

//Creates metafile and file
ChunkManager::ChunkManager(const char* filename, const int fileSize, const int chunkSize) : m_chunkSize(chunkSize){
    if (CHUNKMANAGER_DEBUG)
      cout << "In constructor for ChunkManager\n";

    string metaFilename(filename);
    metaFilename += ".meta";
    
    bool fileExists = false;
    if(checkFileExists(string(filename))) {
        //File exists
        fileExists = true;
    }
    else {
        //Create preallocated file
        m_fileStream.open(filename, ios::in|ios::out|ios::trunc);
        m_fileStream.seekp(fileSize-1);
        m_fileStream.write("", 1);
        m_fileStream.close();
    }
    m_fileStream.open(filename, ios::in|ios::out);
    
    if (checkFileExists(metaFilename)) {
      m_metaFileStream.open(metaFilename.c_str(), ios::in|ios::out);
      readMetaFile();
    }
    else { 
    // If file exists and metafile doesn't we need to make an assumption about the chunks we have
    // If the file exists, assume we have full file i.e. all the chunks are available
    // Otherwise, assume we have none of the file i.e. none of the chunks are available
      m_metaFileStream.open(metaFilename.c_str(), ios::in|ios::out|ios::trunc);
    
      m_numChunks = fileSize/chunkSize;
      if(fileSize % chunkSize > 0) {
          m_numChunks++;
          m_lastChunkSize = fileSize % chunkSize;
      }
      else {
          m_lastChunkSize = chunkSize;
      }
      
      m_chunks = new bool[m_numChunks];
      for(int i = 0; i < m_numChunks; i++) {
          m_chunks[i] = fileExists;
      }
      
      writeMetaFile();
    }

    if (CHUNKMANAGER_DEBUG) {
      cout << "chunkSize: " <<  m_chunkSize << endl;
      cout << "lastChunkSize: " << m_lastChunkSize << endl;
      cout << "numChunks: " << m_numChunks << endl;
      cout << "bitString: ";
      for (int i=0; i<m_numChunks; i++) {
        cout << m_chunks[i];
      }
      cout << endl;
    }
}

//Reads metafile
ChunkManager::ChunkManager(const char* filename) {
    if (CHUNKMANAGER_DEBUG)
      cout << "In constructor for ChunkManager\n";

    int fileSize;
    int chunkSize;
    string file(filename);
    TorrentFileManager tfman(file);
    string metaFilename(filename);

    metaFilename += ".meta";
    fileSize = tfman.getFilesize();
    chunkSize = tfman.getChunkSize();
    m_chunkSize = chunkSize;

    bool fileExists = false;
    m_fileStream.open(filename, ios::in|ios::out|ios::binary);
    if(checkFileExists(string(filename))) {
        //File exists
        fileExists = true;
    }
    else {
        //Create preallocated file
        m_fileStream.open(filename, ios::in|ios::out|ios::trunc|ios::binary);
        m_fileStream.seekp(fileSize-1);
        m_fileStream.write("", 1);
        m_fileStream.close();
        m_fileStream.open(filename, ios::in|ios::out|ios::binary);
    }
    m_fileStream.open(filename, ios::in|ios::out|ios::binary);
    
    if (checkFileExists(metaFilename)) {
      m_metaFileStream.open(metaFilename.c_str(), ios::in|ios::out);
      readMetaFile();
    }
    else { 
    // If file exists and metafile doesn't we need to make an assumption about the chunks we have
    // If the file exists, assume we have full file i.e. all the chunks are available
    // Otherwise, assume we have none of the file i.e. none of the chunks are available
      m_metaFileStream.open(metaFilename.c_str(), ios::in|ios::out|ios::trunc);
    
      m_numChunks = fileSize/chunkSize;
      if(fileSize % chunkSize > 0) {
          m_numChunks++;
          m_lastChunkSize = fileSize % chunkSize;
      }
      else {
          m_lastChunkSize = chunkSize;
      }
      
      m_chunks = new bool[m_numChunks];
      for(int i = 0; i < m_numChunks; i++) {
          m_chunks[i] = fileExists;
      }
      
      writeMetaFile();
    }

    if (CHUNKMANAGER_DEBUG) {
      cout << "chunkSize: " <<  m_chunkSize << endl;
      cout << "lastChunkSize: " << m_lastChunkSize << endl;
      cout << "numChunks: " << m_numChunks << endl;
      cout << "bitString: ";
      for (int i=0; i<m_numChunks; i++) {
        cout << m_chunks[i];
      }
      cout << endl;
    }
}

ChunkManager::~ChunkManager() {
    writeMetaFile();
    m_fileStream.close();
    m_metaFileStream.close();
    delete[] m_chunks;
}

int ChunkManager::readChunk(const int chunkNum, char* data) {
    if(m_fileStream.is_open() && chunkNum < m_numChunks && m_chunks[chunkNum]) {
        int offset = chunkNum * m_chunkSize;
        m_fileStream.seekg(offset);
        int readSize;
        if(chunkNum == m_numChunks - 1)
            readSize = m_lastChunkSize;
        else
            readSize = m_chunkSize;
        m_fileStream.read(data, readSize);
        return readSize;
    }
    else {
        return 0;
    }
}

int ChunkManager::writeChunk(const int chunkNum, char* data) {
    if(m_fileStream.is_open() && chunkNum < m_numChunks) {
        m_chunks[chunkNum] = true;
        int offset = chunkNum * m_chunkSize;
        m_fileStream.seekp(offset);
        int writeSize;
        if(chunkNum == m_numChunks - 1)
            writeSize = m_lastChunkSize;
        else
            writeSize = m_chunkSize;
        m_fileStream.write(data, writeSize);
        m_fileStream.sync();
        writeMetaFile();
        return writeSize;
    }
    else {
        return 0;
    }
}

int ChunkManager::getNumChunks() {
    return m_numChunks;
}

int ChunkManager::getChunkSize() {
    return m_chunkSize;
}

int ChunkManager::getLastChunkSize() {
    return m_lastChunkSize;
}

bool* ChunkManager::getChunks() {
    return m_chunks;
}

bool ChunkManager::chunkAvailable(const int chunkNum) {
    return m_chunks[chunkNum];
}

string ChunkManager::getBitstring() {
  string bitstring = "";
  for (int i=0; i<m_numChunks; i++) {
    if (chunkAvailable(i))
      bitstring += '1';
    else
      bitstring += '0';
  }
  return bitstring;
}


//Helper: parses meta file and initializes variables
void ChunkManager::readMetaFile() {
    string bitString;
    string line;
    int n;

    if (CHUNKMANAGER_DEBUG)
      cout << "Reading from existing metafile\n";

    m_metaFileStream.seekp(ios::beg);

    // Get chunk size
    getline(m_metaFileStream, line);
    n = line.find(":");
    this->m_chunkSize = stoi(line.substr(n+2, line.size()));

    // Get last chunk size
    getline(m_metaFileStream, line);
    n = line.find(":");
    this->m_lastChunkSize = stoi(line.substr(n+2, line.size()));

    // Get bitstring
    getline(m_metaFileStream, line);
    n = line.find(":");
    bitString = line.substr(n+2, line.size());

    m_numChunks = bitString.length();
    m_chunks = new bool[m_numChunks];
    for(int i = 0; i < m_numChunks; i++) {
        if(bitString[i] == '0') {
            m_chunks[i] = false;
        }
        else {
            m_chunks[i] = true;
        }
    }
}

//Helper: updates meta file
void ChunkManager::writeMetaFile() {
    string line;

    if (CHUNKMANAGER_DEBUG)
      cout << "Writing new metafile\n";

    m_metaFileStream.seekp(ios::beg);

    line = "chunkSize: " + to_string(m_chunkSize) + "\n";
    m_metaFileStream.write(line.c_str(), line.length());

    line = "lastChunkSize: " + to_string(m_lastChunkSize) + "\n";
    m_metaFileStream.write(line.c_str(), line.length());

    line = "bitString: ";
    for(int i = 0; i < m_numChunks; i++) {
        if(m_chunks[i])
            line += '1';
        else
            line += '0';
    }
    line += "\n";
    m_metaFileStream.write(line.c_str(), line.length());

    m_metaFileStream.sync();
}

bool checkFileExists(const std::string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) == 0)
    {
        return true;
    }
    return false;
}
 
