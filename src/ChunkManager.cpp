#include "ChunkManager.hpp"

static int CHUNKMANAGER_DEBUG = 1;

//Creates metafile and file
ChunkManager::ChunkManager(const char* filename, const long fileSize, const long chunkSize) : m_chunkSize(chunkSize){
    if (CHUNKMANAGER_DEBUG)
      cout << "In constructor for ChunkManager\n";

    string metaFilename(filename);
    metaFilename += ".meta";
    
    bool fileExists = false;
    m_fileStream.open(filename, ios::in|ios::out|ios::binary);
    if(m_fileStream.is_open()) {
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
    
    m_metaFileStream.open(metaFilename.c_str(), ios::in);
    if (m_metaFileStream.is_open()) {
      readMetaFile();
    }
    else { 
    // If file exists and metafile doesn't we need to make an assumption about the chunks we have
    // If the file exists, assume we have full file i.e. all the chunks are available
    // Otherwise, assume we have none of the file i.e. none of the chunks are available
      m_metaFileStream.open(metaFilename.c_str(), ios::out|ios::trunc);
    
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
}

//Reads metafile
ChunkManager::ChunkManager(const char* filename) {
    string metaFilename(filename);
    metaFilename += ".meta";
    m_metaFileStream.open(metaFilename.c_str(), ios::in|ios::out);
    readMetaFile();
    
    m_fileStream.open(filename, ios::in|ios::out|ios::binary);
}

ChunkManager::~ChunkManager() {
    writeMetaFile();
    m_fileStream.close();
    m_metaFileStream.close();
    delete[] m_chunks;
}

long ChunkManager::readChunk(const long chunkNum, char* data) {
    if(m_fileStream.is_open() && chunkNum < m_numChunks && m_chunks[chunkNum]) {
        long offset = chunkNum * m_chunkSize;
        m_fileStream.seekg(offset);
        long readSize;
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

long ChunkManager::writeChunk(const long chunkNum, char* data) {
    if(m_fileStream.is_open() && chunkNum < m_numChunks) {
        m_chunks[chunkNum] = true;
        long offset = chunkNum * m_chunkSize;
        m_fileStream.seekp(offset);
        long writeSize;
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

long ChunkManager::getNumChunks() {
    return m_numChunks;
}

long ChunkManager::getChunkSize() {
    return m_chunkSize;
}

long ChunkManager::getLastChunkSize() {
    return m_lastChunkSize;
}

bool* ChunkManager::getChunks() {
    return m_chunks;
}

bool ChunkManager::chunkAvailable(const long chunkNum) {
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
    bitString = stoi(line.substr(n+2, line.size()));

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
