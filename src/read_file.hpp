#ifndef READFILE_HPP
#define READFILE_HPP


#include <iostream>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "./database_handler.hpp"

class RedisRDBParser {
private:
    DatabaseHandler& db_handler;
    std::string filename;
    bool readMetaData(std::ifstream& file);
    bool readDatabase(std::ifstream& file);
    bool readHeader(std::ifstream &file);
    std::string readLengthPrefixedString(std::ifstream &file);
    std::string readEncodedString(std::ifstream &file, uint8_t encodingType);
    std::string readIntegerEncodedString(std::ifstream &file, uint8_t encodingType);
    // void skipUnknownEntry(std::ifstream &file);
    // Helper Functions
    uint64_t readSizeEncoded(std::ifstream& file, uint8_t& sizeEncoding);
    std::pair<std::string, std::string> readStringKeyValue(std::ifstream& file);
    void print(std::string message, uint64_t value);
    // std::string readLZFCompressedString(std::ifstream &file);
    // std::string lzfDecompress(const std::vector<char> &compressedData, uint32_t originalLen);    
public:
    RedisRDBParser(const std::string& filename,
                DatabaseHandler& db_handler) : filename(filename), db_handler(db_handler) {}
    bool parse();
};

#endif //READFILE_HPP
