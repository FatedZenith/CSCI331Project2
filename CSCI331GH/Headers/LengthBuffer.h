#ifndef LENGTHBUFFER_H
#define LENGTHBUFFER_H

#include <fstream>
#include <cstdint>
#include "ZipCodeRecordBuffer.h"

class LengthBuffer {
public:
    // Write a single ZipCodeRecordBuffer to the file
    static bool writeRecord(std::ofstream& out, const ZipCodeRecordBuffer& record);

    // Read the next ZipCodeRecordBuffer from the file
    static bool readNextRecord(std::ifstream& in, ZipCodeRecordBuffer& record);
};

#endif // LENGTHBUFFER_H
