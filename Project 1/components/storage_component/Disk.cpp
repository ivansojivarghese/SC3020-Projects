#include "Disk.h"
#include "Block.h"
#include <fstream>
#include <iostream>
#include <string>

// Constructor
Disk::Disk(const std::string& file) : filename(file) {}

// Write a block to disk
void Disk::writeBlock(const Block& block, int blockId) {
    std::ofstream outFile(filename, std::ios::binary | std::ios::app);
    if (!outFile) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    std::vector<char> buffer = block.serialize();
    outFile.write(buffer.data(), BLOCK_SIZE);
    outFile.close();
}

// Read a block from disk
Block Disk::readBlock(int blockId) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error opening file for reading!" << std::endl;
        return Block();
    }

    inFile.seekg(blockId * BLOCK_SIZE, std::ios::beg);
    std::vector<char> buffer(BLOCK_SIZE);
    inFile.read(buffer.data(), BLOCK_SIZE);
    inFile.close();

    return Block::deserialize(buffer, blockId);
}