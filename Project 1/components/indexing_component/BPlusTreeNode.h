#ifndef BPLUSTREENODE_H
#define BPLUSTREENODE_H

#include <vector>
#include "../../components/storage_component/Record.h"

class BPlusTreeNode {
public:
    bool isLeafNode;
    int maxCapacity;
    std::vector<float> nodeKeys;
    std::vector<NBARecords*> recordPointers;
    std::vector<BPlusTreeNode*> childPointers;
    BPlusTreeNode* nextLeafNode;
    BPlusTreeNode* parent;

    BPlusTreeNode(bool isLeafNode);

    std::vector<char> serialize() const;
    static BPlusTreeNode* deserialize(const std::vector<char>& buffer, bool isLeafNode);

    BPlusTreeNode* getNextLeafNode();
    std::vector<float> getNodeKeys();
};

#endif // BPLUSTREENODE_H