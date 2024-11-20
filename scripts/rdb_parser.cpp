#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>



class RedisRDBParserTest {
public:
    explicit RedisRDBParserTest(const std::string &filename) : filename(filename) {}

    bool parse() {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Unable to open file.\n";
            return false;
        }
        // while(file){
        //     uint64_t byte;
        //     file.read(reinterpret_cast<char*>(&byte), sizeof(byte));
        //     uint64_t reverseByte = reverseEndianness(byte);
        //     std::cout << std::hex << reverseByte << std::endl;
        // }
        // return true;
        if (!readHeader(file)) {
            std::cerr << "Error: Invalid RDB header.\n";
            return false;
        }
        while (file) {
            uint8_t indicator;
            file.read(reinterpret_cast<char *>(&indicator), sizeof(indicator));

            // Parse data entries based on type
            switch (indicator) {
                case 0xFA:  // String type
                    readMetaData(file);
                    break;
                // Additional data types go here...
                case 0xFE:
                    readDatabase(file);
                    break;
                case 0xFF:

                default:
                    break;
            }
        }

        return true;
    }

private:
    std::string filename;

    bool readMetaData(std::ifstream& file){
        std::cout << std::endl << "Printing MetaDta Information" << std::endl;
        std::string attribute = readLengthPrefixedString(file);
        std::string value = readLengthPrefixedString(file);
        std::cout << "Attribute: " << attribute << ", Value: " << value << std::endl;
    }

    void readDatabase(std::ifstream& file){
        std::cout << std::endl << "--------------------Hash Table Information--------------------" << std::endl;
        uint8_t nextByte;
        file.read(reinterpret_cast<char*>(&nextByte), sizeof(nextByte));
        uint64_t database_index = readSizeEncoded(file, nextByte);
        print("Database Index: ", database_index);
        file.read(reinterpret_cast<char*>(&nextByte), sizeof(nextByte));
        print("Hash Table Indicator: ", nextByte);
        if (nextByte == 0xFB){
            file.read(reinterpret_cast<char*>(&nextByte), sizeof(nextByte));
            uint8_t hash_table_size = readSizeEncoded(file, nextByte);
            print("Hash Table size: ", hash_table_size);
            file.read(reinterpret_cast<char*>(&nextByte), sizeof(nextByte));
            uint8_t expiry_hash_table_size = readSizeEncoded(file, nextByte);
            print("Expiry hash table size: ", expiry_hash_table_size);
            std::cout << std::endl << "--------------------Key and Value pairs follow--------------------" << std::endl << std::endl;
            uint8_t value_type;
            for (int i = 0; i < hash_table_size; i++){
                file.read(reinterpret_cast<char*>(&nextByte), sizeof(nextByte));
                if (nextByte == 0xFC){
                    uint64_t time_millisec;
                    file.read(reinterpret_cast<char*>(&time_millisec), sizeof(time_millisec));
                    file.read(reinterpret_cast<char*>(&value_type), sizeof(value_type));
                    if (value_type == 0x0){
                        readStringKeyValue(file);
                    }
                    else {
                        print("Value Type not handled yet: ", value_type);
                    }
                    print("Expiry value in milliseconds: ", time_millisec);
                }
                else if(nextByte == 0xFD){
                    uint32_t time_sec;
                    file.read(reinterpret_cast<char*>(&time_sec), sizeof(time_sec));
                    file.read(reinterpret_cast<char*>(&value_type), sizeof(value_type));
                    if (value_type == 0x0){
                        readStringKeyValue(file);
                    }
                    else {
                        print("Value Type not handled yet: ", value_type);
                    }
                    print("Expiry value in milliseconds: ", time_sec);
                }
                else {
                    value_type = nextByte;
                    if (value_type == 0x0){
                        readStringKeyValue(file);
                    }
                    else {
                        print("Value Type not handled yet: ", value_type);
                    }
                }
            }
        }
    }

    void readStringKeyValue(std::ifstream& file){
        std::string key = readLengthPrefixedString(file);
        std::string value = readLengthPrefixedString(file);
        std::cout << "String Entry - Key: " << key << ", Value: " << value << std::endl;
    }
    uint64_t readSizeEncoded(std::ifstream& file, uint8_t& sizeEncoding){
        switch (sizeEncoding & 0xC0)
        {
        case 0x0:
            return sizeEncoding;
        case 0x40:
            uint8_t nextByte;
            file.read(reinterpret_cast<char*>(&nextByte), sizeof(nextByte));
            return (static_cast<uint16_t>(sizeEncoding)<<8 | static_cast<uint16_t>(nextByte));
        case 0x80:
            uint64_t next4Bytes;
            file.read(reinterpret_cast<char*>(&next4Bytes), sizeof(next4Bytes));
            return next4Bytes;
        default:
            break;
        }
    }

    void print(std::string message, uint64_t value){
        std::cout << message << std::hex << value << std::endl;
    }

    bool readHeader(std::ifstream &file) {
        char header[9];
        file.read(header, 9);
        if (std::string(header, 5) != "REDIS") {
            return false;
        }

        std::string version(header + 5, 4);  // Extract the version number
        std::cout << "--------------------RDB version: " << version << "--------------------" << std::endl;
        return true;
    }

    std::string readLengthPrefixedString(std::ifstream &file) {
        uint8_t lengthByte;
        file.read(reinterpret_cast<char *>(&lengthByte), sizeof(lengthByte));

        if ((lengthByte & 0xC0) == 0xC0) {  // Encoded string
            return readEncodedString(file, lengthByte);
        } else {
            // Length-prefixed raw string
            std::string result(lengthByte, '\0');
            file.read(&result[0], lengthByte);
            return result;
        }
    }

    std::string readEncodedString(std::ifstream &file, uint8_t encodingType) {
        switch (encodingType ^ 0xC0) {  // Mask to get the encoding type
            case 0:  // Integer encoding
                return readIntegerEncodedString(file, encodingType);
            case 1:  // Integer encoding
                return readIntegerEncodedString(file, encodingType);
            case 2:  // Integer encoding
                return readIntegerEncodedString(file, encodingType);
            case 3:  // LZF compression
                return readLZFCompressedString(file);
            default:
                std::cerr << "Error: Unknown encoding type: ";
                std::cout << std::hex << encodingType << std::endl;
                return "";
        }
    }

    std::string readIntegerEncodedString(std::ifstream &file, uint8_t encodingType) {
        int64_t value;
        switch (encodingType & 0xFF) {
            case 0xC0:  // 8-bit integer
                uint8_t int8Val;
                file.read(reinterpret_cast<char*>(&int8Val), sizeof(int8Val));
                value = int8Val;
                break;
            case 0xC1:  // 16-bit integer
                uint16_t int16Val;
                file.read(reinterpret_cast<char*>(&int16Val), sizeof(int16Val));
                value = int16Val;
                break;
            case 0xC2:  // 32-bit integer
                uint32_t int32Val;
                file.read(reinterpret_cast<char*>(&int32Val), sizeof(int32Val));
                value = int32Val;
                break;
            default:
                std::cerr << "Error: Unknown integer encoding.\n";
                return "";
        }
        return std::to_string(value);
    }

    std::string readLZFCompressedString(std::ifstream &file) {
        uint32_t originalLen, compressedLen;
        file.read(reinterpret_cast<char *>(&compressedLen), sizeof(compressedLen));
        file.read(reinterpret_cast<char *>(&originalLen), sizeof(originalLen));

        std::vector<char> compressedData(compressedLen);
        file.read(compressedData.data(), compressedLen);

        // Placeholder for LZF decompression function
        return lzfDecompress(compressedData, originalLen);
    }

    std::string lzfDecompress(const std::vector<char> &compressedData, uint32_t originalLen) {
        // Placeholder: Implement or link to an LZF decompression function
        // This should decompress the compressedData to the originalLen
        std::string decompressedData(originalLen, '\0');
        // Decompression logic goes here...

        return decompressedData;
    }

    void parseStringEntry(std::ifstream &file) {
        std::string key = readLengthPrefixedString(file);
        std::string value = readLengthPrefixedString(file);
        std::cout << "String Entry - Key: " << key << ", Value: " << value << std::endl;
    }

    void skipUnknownEntry(std::ifstream &file) {
        std::string key = readLengthPrefixedString(file);
        std::string value = readLengthPrefixedString(file);
        std::cerr << "Skipped unknown entry - Key: " << key << std::endl;
    }
};

int main() {
    RedisRDBParserTest parser("dump.rdb");
    if (parser.parse()) {
        std::cout << "RDB file parsed successfully.\n";
    } else {
        std::cerr << "Failed to parse RDB file.\n";
    }
    return 0;
}
