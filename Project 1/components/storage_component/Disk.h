#ifndef DISK_H
#define DISK_H

#include "Block.h"
#include <string>

class Disk {
private:
    std::string filename;

public:
    Disk(const std::string& file);

    // Write a block to disk
    void writeBlock(const Block& block, int blockId);

    // Read a block from disk
    Block readBlock(int blockId);
};

#endif // DISK_H