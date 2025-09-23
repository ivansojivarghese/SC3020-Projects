#include "Block.h"
#include <string>

// Constructor
Block::Block(int id) : blockId(id) {}

// Add a record to the block
bool Block::addRecord(const Record& record) {
    if (isFull()) {
        return false; // Block is full
    }
    records.push_back(record);
    return true;
}

// Check if the block is full
bool Block::isFull() const {
    return records.size() >= RECORDS_PER_BLOCK;
}

// Serialize the block into a byte stream
std::vector<char> Block::serialize() const {
    std::vector<char> buffer(BLOCK_SIZE);
    size_t offset = 0;
    for (const auto& record : records) {
        std::vector<char> recordBuffer = record.serialize();
        std::memcpy(buffer.data() + offset, recordBuffer.data(), RECORD_SIZE);
        offset += RECORD_SIZE;
    }
    return buffer;
}

// Deserialize a byte stream into a block
Block Block::deserialize(const std::vector<char>& buffer, int id) {
    Block block(id);
    size_t offset = 0;
    while (offset < BLOCK_SIZE) {
        std::vector<char> recordBuffer(buffer.begin() + offset, buffer.begin() + offset + RECORD_SIZE);
        block.addRecord(Record::deserialize(recordBuffer));
        offset += RECORD_SIZE;
    }
    return block;
}

// Get all records in the block
std::vector<Record> Block::getRecords() const {
    return records;
}