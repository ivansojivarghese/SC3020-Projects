#ifndef BLOCK_H
#define BLOCK_H

#include "Record.h"
#include <vector>
#include <string>

const int BLOCK_SIZE = 4096; // Block size in bytes (4 KB)
const int RECORDS_PER_BLOCK = BLOCK_SIZE / RECORD_SIZE; // 102 records per block

class Block {
private:
    std::vector<Record> records;
    int blockId;

public:
    Block(int id = 0);

    // Add a record to the block
    bool addRecord(const Record& record);

    // Check if the block is full
    bool isFull() const;

    // Serialize the block into a byte stream
    std::vector<char> serialize() const;

    // Deserialize a byte stream into a block
    static Block deserialize(const std::vector<char>& buffer, int id);

    // Get all records in the block
    std::vector<Record> getRecords() const;
};

#endif // BLOCK_H