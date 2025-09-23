#include "BPlusTreeNode.h"
#include <cstring>

BPlusTreeNode::BPlusTreeNode(bool isLeafNode) {
    this->isLeafNode = isLeafNode;
    this->maxCapacity = 32;
    this->nextLeafNode = nullptr;
    this->parent = nullptr;
}

std::vector<char> BPlusTreeNode::serialize() const {
    std::vector<char> buffer;
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&isLeafNode), reinterpret_cast<const char*>(&isLeafNode) + sizeof(bool));
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&maxCapacity), reinterpret_cast<const char*>(&maxCapacity) + sizeof(int));
    for (float key : nodeKeys) {
        buffer.insert(buffer.end(), reinterpret_cast<const char*>(&key), reinterpret_cast<const char*>(&key) + sizeof(float));
    }
    if (isLeafNode) {
        for (NBARecords* records : recordPointers) {
            for (Record* record : records->records) {
                std::vector<char> recordBuffer = record->serialize();
                buffer.insert(buffer.end(), recordBuffer.begin(), recordBuffer.end());
            }
        }
    } else {
        for (BPlusTreeNode* child : childPointers) {
            std::vector<char> childBuffer = child->serialize();
            buffer.insert(buffer.end(), childBuffer.begin(), childBuffer.end());
        }
    }
    return buffer;
}

BPlusTreeNode* BPlusTreeNode::deserialize(const std::vector<char>& buffer, bool isLeafNode) {
    BPlusTreeNode* node = new BPlusTreeNode(isLeafNode);
    size_t offset = 0;
    std::memcpy(&node->isLeafNode, buffer.data() + offset, sizeof(bool));
    offset += sizeof(bool);
    std::memcpy(&node->maxCapacity, buffer.data() + offset, sizeof(int));
    offset += sizeof(int);
    while (offset < buffer.size()) {
        float key;
        std::memcpy(&key, buffer.data() + offset, sizeof(float));
        node->nodeKeys.push_back(key);
        offset += sizeof(float);
    }
    return node;
}

BPlusTreeNode* BPlusTreeNode::getNextLeafNode() {
    return this->nextLeafNode;
}

std::vector<float> BPlusTreeNode::getNodeKeys() {
    return this->nodeKeys;
}