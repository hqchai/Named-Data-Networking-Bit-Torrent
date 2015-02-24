#include "ChunkManager.hpp"

//Creates metafile and file
ChunkManager::ChunkManager(const char* filename, const long fileSize, const long chunkSize) : m_chunkSize(chunkSize){
    string metaFilename(filename);
    metaFilename += ".meta";
    m_metaFileStream.open(metaFilename.c_str(), ios::in|ios::out|ios::trunc);
    
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


//Helper: parses meta file and initializes variables
void ChunkManager::readMetaFile() {
    string bitString;
    m_metaFileStream >> m_chunkSize;
    m_metaFileStream >> m_lastChunkSize;
    m_metaFileStream >> bitString;
    
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
    m_metaFileStream.seekp(ios::beg);
    m_metaFileStream << m_chunkSize << "\n";
    m_metaFileStream << m_lastChunkSize << "\n";
    char* bitString = new char[m_numChunks];
    for(int i = 0; i < m_numChunks; i++) {
        if(m_chunks[i])
            bitString[i] = '1';
        else
            bitString[i] = '0';
    }
    m_metaFileStream << bitString;
    delete[] bitString;
    m_metaFileStream.sync();
}