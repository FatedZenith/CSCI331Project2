#ifndef HeaderBuffer_H
#define HeaderBuffer_H

#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>      // Required for the write-to-buffer technique
#include <chrono>       // For getting the current date/time
#include <iomanip>      // For formatting the date/time


// Enum to represent the data type of a field
enum class DataType : uint8_t {
    STRING,
    UINT32,
    FLOAT,
    DOUBLE
};

// Holds metadata for a single field in a record (as per your image)
struct FieldSchema {
    std::string fieldName;
    DataType fieldType;

    // Writes this schema object to a binary stream
    void write(std::ostream& out) const {
        uint16_t nameLen = fieldName.length();
        out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        out.write(fieldName.c_str(), nameLen);
        out.write(reinterpret_cast<const char*>(&fieldType), sizeof(fieldType));
    }

    // Reads this schema object from a binary stream
    void read(std::istream& in) {
        uint16_t nameLen = 0;
        in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        fieldName.resize(nameLen);
        in.read(&fieldName[0], nameLen);
        in.read(reinterpret_cast<char*>(&fieldType), sizeof(fieldType));
    }
};



class HeaderRecordBuffer {
public:
    char fileStructureType[16] = "ZIP_CODE_DATA"; // Fixed-size file type identifier
    uint32_t version = 1;
    uint64_t recordCount = 0;
    uint16_t primaryKeyFieldIndex = 0;      // e.g., 0 for the first field (ZipCode)
    std::string indexFileName;
    std::string creationDate;
    
    // A list of all the fields this file contains
    std::vector<FieldSchema> fields;

    void writeHeader(std::ostream& out) {
        // Automatically generate the creation date string
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream date_ss;
        #ifdef _MSC_VER
            struct tm timeinfo;
            localtime_s(&timeinfo, &in_time_t);
            date_ss << std::put_time(&timeinfo, "%Y-%m-%d %X");
        #else
            date_ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        #endif
        creationDate = date_ss.str();

        // Use a temporary in-memory buffer to easily calculate the total header size
        std::stringstream headerBuffer(std::ios::binary | std::ios::in | std::ios::out);

        // Write all data to the temporary buffer first
        headerBuffer.write(fileStructureType, sizeof(fileStructureType));
        headerBuffer.write(reinterpret_cast<const char*>(&version), sizeof(version));
        headerBuffer.write(reinterpret_cast<const char*>(&recordCount), sizeof(recordCount));
        headerBuffer.write(reinterpret_cast<const char*>(&primaryKeyFieldIndex), sizeof(primaryKeyFieldIndex));

        uint16_t indexNameLen = indexFileName.length();
        headerBuffer.write(reinterpret_cast<const char*>(&indexNameLen), sizeof(indexNameLen));
        headerBuffer.write(indexFileName.c_str(), indexNameLen);

        uint16_t dateLen = creationDate.length();
        headerBuffer.write(reinterpret_cast<const char*>(&dateLen), sizeof(dateLen));
        headerBuffer.write(creationDate.c_str(), dateLen);
        
        uint16_t fieldCount = fields.size();
        headerBuffer.write(reinterpret_cast<const char*>(&fieldCount), sizeof(fieldCount));
        for (const auto& field : fields) {
            field.write(headerBuffer);
        }

        headerBuffer.seekg(0, std::ios::end);
        uint32_t totalHeaderSize = headerBuffer.tellg();

        out.write(reinterpret_cast<const char*>(&totalHeaderSize), sizeof(totalHeaderSize));

        out.write(headerBuffer.str().c_str(), totalHeaderSize);
    }

    bool readHeader(std::istream& in) {
        uint32_t totalHeaderSize = 0;
        if (!in.read(reinterpret_cast<char*>(&totalHeaderSize), sizeof(totalHeaderSize))) return false;

        // For safety, you could read the header into a buffer to avoid reading past its end
        std::vector<char> buffer(totalHeaderSize);
        if (!in.read(buffer.data(), totalHeaderSize)) return false;
        std::stringstream headerBuffer(std::string(buffer.begin(), buffer.end()));

        if (!headerBuffer.read(fileStructureType, sizeof(fileStructureType))) return false;
        if (!headerBuffer.read(reinterpret_cast<char*>(&version), sizeof(version))) return false;
        if (!headerBuffer.read(reinterpret_cast<char*>(&recordCount), sizeof(recordCount))) return false;
        if (!headerBuffer.read(reinterpret_cast<char*>(&primaryKeyFieldIndex), sizeof(primaryKeyFieldIndex))) return false;
        
        uint16_t indexNameLen = 0;
        if (!headerBuffer.read(reinterpret_cast<char*>(&indexNameLen), sizeof(indexNameLen))) return false;
        indexFileName.resize(indexNameLen);
        if (!headerBuffer.read(&indexFileName[0], indexNameLen)) return false;

        uint16_t dateLen = 0;
        if (!headerBuffer.read(reinterpret_cast<char*>(&dateLen), sizeof(dateLen))) return false;
        creationDate.resize(dateLen);
        if (!headerBuffer.read(&creationDate[0], dateLen)) return false;
        
        uint16_t fieldCount = 0;
        if (!headerBuffer.read(reinterpret_cast<char*>(&fieldCount), sizeof(fieldCount))) return false;
        fields.resize(fieldCount);
        for (uint16_t i = 0; i < fieldCount; ++i) {
            fields[i].read(headerBuffer);
        }

        return true;
    }
};

#endif // HeaderBuffer_H