//
// Created by 刘育氚 on 2019/5/24.
//

#ifndef BPTREE_BPTREE_H
#define BPTREE_BPTREE_H

#include <algorithm>
#include <string>
#include "Comp.h"
#include "List.h"
#include "Utils.h"

const unsigned int MIN_ORDER = 2;

template<typename K, typename V>
class BPTree {
private:
    static const std::string SUFFIX;

    struct Node {
        Node *parentPtr = NULL;
        Node *previous = NULL;
        Node *next = NULL;
        bool leaf;
        List<Node *> childNodePtrs;
        List<V> values;
        List<K> keys;

        Node(unsigned int initCap, bool leaf);;
    };

    unsigned int order;
    unsigned int size;
    unsigned int minLoad;
    comparator<K> comp;
    unsigned int initCap = 0;
    Node *root;

    bool putToNode(Node *nodePtr, const K &key, const V *value, Node *insertNodePtr);

    int deleteFromNode(Node *nodePtr, const K &key);

    void updateParentKey(Node *node, K &oldKey, K &newKey, int pos);

    Node *fixNode(Node *nodePtr);

    Node *split(Node *nodePtr);

    Node *getFirstLeaf();

    Node *getLastLeaf();

    void clear(Node *node);

    // return node's offset
    long serializeNode(const Node *node, FILE *f);

    // ftell must be at the start offset of target node
    Node *deserializeNode(FILE *f, Node *parentNode);

    /**
     * @return NULL if this node is last node of its layer
     */
    Node *getNextSibling(Node *node);

public:

    explicit BPTree(unsigned int order) : BPTree(order, 0, NULL) {}

    BPTree(unsigned int order, comparator<K> comp) : BPTree(order, 0, comp) {}

    explicit BPTree(unsigned int order, unsigned int initCap, comparator<K> comp) :
            order(std::max(MIN_ORDER, order)),
            size(0),
            minLoad((this->order + 1) / 2),
            initCap(std::max(std::min(this->order,
                                      initCap), this->minLoad)),
            comp(comp),
            root(new Node(this->initCap,
                          true)) {}

    BPTree(const BPTree &another) {
        order = another.order;
        minLoad = another.minLoad;
        size = another.minLoad;
    }

    ~BPTree();

    /**
     * deserialize from a file
     */
    static std::shared_ptr<BPTree<K, V>> deserialize(const std::string &path);

    static std::shared_ptr<BPTree<K, V>> deserialize(const std::string &path, comparator<K> comp);

    void put(const K &key, const V &value);

    void remove(K &key);

    /**
     * @return NULL if not exists else a pointer to the value
     */
    V *get(const K &key);

    bool containsKey(const K &key);

    int getOrder();

    int getSize();

    /**
     * iterate order by key
     * @param func call func(const K& key, const V& value) for each. func returns true means iteration ends
     */
    template<typename BiApply>
    void foreach(BiApply func);

    template<typename BiApply>
    void foreachReverse(BiApply func);

    /**
     * iterate order by key with index (starts from 0)
     * @param func call func(int index, const K& key, const V& value) for each. func returns true means iteration ends
     */
    template<typename BiApplyIndex>
    void foreachIndex(BiApplyIndex func);

    template<typename BiApplyIndex>
    void foreachIndexReverse(BiApplyIndex func);

    void serialize(std::string &path);

    /**
     * clear the tree
     * note that all values allocated will be freed
     */
    void clear();
};

template<typename K, typename V>
BPTree<K, V>::Node::Node(unsigned int initCap, bool leaf) :
        leaf(leaf) {
    childNodePtrs.reserve(initCap);
    if (leaf) {
        values.reserve(initCap);
    } else {
        keys.reserve(initCap);
    }
}


template<typename K, typename V>
const std::string BPTree<K, V>::SUFFIX = ".bpt";

template<typename K, typename V>
void BPTree<K, V>::put(const K &key, const V &value) {
    bool result;
    Node *insertNode = root;
    int pos = -1;
    while (!insertNode->leaf) {
        int s = insertNode->childNodePtrs.getSize();
        List<K> constKeys = insertNode->keys;
        pos = constKeys.binaryFind(key);
        if (pos == s) {
            pos--;
        }
        insertNode = insertNode->childNodePtrs[pos];
    }

    result = putToNode(insertNode, key, &value, NULL);
    if (result) size++;

    Node *tmp = insertNode->parentPtr;
    while (tmp) {
        List<K> &keys = tmp->keys;
        K oldKey = keys[pos];
        if (pos >= 0 && pos < keys.getSize() && compare(oldKey, key, comp) < 0) {
            keys[pos] = key;
            if (pos == keys.getSize() - 1 && (tmp = tmp->parentPtr)) {
                pos = tmp->keys.binaryFind(oldKey);
                continue;
            }
        }
        break;
    }

    int s = insertNode->keys.getSize();
    if (result && s > order) {
        Node *node = insertNode;
        Node *splitParent;
        Node *p;
        do {
            p = node->parentPtr;
            splitParent = split(node);
            if (splitParent != p) {
                this->root = splitParent;
                break;
            } else if (splitParent->keys.getSize() > order) {
                node = splitParent;
            } else {
                break;
            }
        } while (true);
    }
}

template<typename K, typename V>
void BPTree<K, V>::remove(K &key) {
    Node *deleteNodePtr = root;
    int pos = -1;
    while (!deleteNodePtr->leaf) {
        int s = deleteNodePtr->childNodePtrs.getSize();
        List<K> constKeys = deleteNodePtr->keys;
        pos = constKeys.binaryFind(key);
        if (pos == s) {
            pos--;
        }
        deleteNodePtr = deleteNodePtr->childNodePtrs[pos];
    }

    int deleteKeyPos = deleteFromNode(deleteNodePtr, key);
    if (deleteKeyPos < 0 || deleteNodePtr == root) {
        return;
    }
    if (pos == -1) {
        pos = deleteKeyPos;
    }


    if (deleteKeyPos == deleteNodePtr->keys.getSize()) {
        updateParentKey(deleteNodePtr, key, deleteNodePtr->keys[deleteNodePtr->keys.getSize() - 1], pos);
    }

    Node *tmpNodePtr = deleteNodePtr;
    while ((tmpNodePtr = fixNode(tmpNodePtr)));
    if (root->childNodePtrs.getSize() == 1) {
        Node *oldRoot = root;
        root = root->childNodePtrs[0];
        root->parentPtr = NULL;
        delete oldRoot;
    }
}

template<typename K, typename V>
V *BPTree<K, V>::get(const K &key) {
    Node *curPtr = root;
    int pos;
    while (!curPtr->leaf) {
        int s = curPtr->childNodePtrs.getSize();
        List<K> constKeys = curPtr->keys;
        pos = constKeys.binaryFind(key);
        if (pos == s) {
            pos--;
        }
        curPtr = curPtr->childNodePtrs[pos];
    }
    const List<K> &constKeys = curPtr->keys;
    pos = constKeys.binaryFind(key);
    if (pos >= 0 && pos < curPtr->values.getSize() && compare(curPtr->keys[pos], key, comp) == 0) {
        return &(curPtr->values[pos]);
    }

    return NULL;
}

template<typename K, typename V>
int BPTree<K, V>::getOrder() {
    return order;
}

template<typename K, typename V>
int BPTree<K, V>::getSize() {
    return size;
}

template<typename K, typename V>
bool BPTree<K, V>::putToNode(BPTree::Node *nodePtr, const K &key, const V *value, BPTree::Node *insertNodePtr) {
    int toIndex = nodePtr->keys.binaryFind(key);
    bool present = toIndex < nodePtr->keys.getSize() && compare(key, nodePtr->keys[toIndex], comp) == 0;

    if (present) {
        if (nodePtr->leaf) {
            nodePtr->values[toIndex] = *value;
        } else {
            nodePtr->childNodePtrs[toIndex] = insertNodePtr;
        }
        return false;
    } else {
        nodePtr->keys.insert(toIndex, key);
        if (nodePtr->leaf) {
            nodePtr->values.insert(toIndex, *value);
        } else {
            nodePtr->childNodePtrs.insert(toIndex, insertNodePtr);
        }
    }

    return true;
}

template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::split(BPTree::Node *nodePtr) {
    List<K> &keys = nodePtr->keys;
    if (keys.getSize() <= order) {
        return NULL;
    }

    int mid = (keys.getSize() + 1) / 2;
    Node *parentPtr = nodePtr->parentPtr;
    if (!parentPtr) {
        parentPtr = new Node(initCap, false);
        nodePtr->parentPtr = parentPtr;
    }

    // split
    Node *left = new Node(initCap, nodePtr->leaf);
    List<K> &leftKeys = left->keys;
    List<K> &rightKeys = nodePtr->keys;
    leftKeys.add(rightKeys, 0, mid);
    rightKeys.removeRange(0, mid);
    left->parentPtr = parentPtr;
    if (nodePtr->leaf) {
        left->values.add(nodePtr->values, 0, mid);
        nodePtr->values.removeRange(0, mid);
        left->previous = nodePtr->previous;
        nodePtr->previous = left;
        left->next = nodePtr;
        if (left->previous) {
            left->previous->next = left;
        }
    } else {
        left->childNodePtrs.add(nodePtr->childNodePtrs, 0, mid);
        nodePtr->childNodePtrs.removeRange(0, mid);
        for (int i = 0, s = left->childNodePtrs.getSize(); i < s; ++i) {
            Node *&n = left->childNodePtrs[i];
            n->parentPtr = left;
        }
    }
    putToNode(parentPtr, leftKeys[leftKeys.getSize() - 1], NULL, left);
    putToNode(parentPtr, rightKeys[rightKeys.getSize() - 1], NULL, nodePtr);
    return parentPtr;
}

template<typename K, typename V>
int BPTree<K, V>::deleteFromNode(BPTree::Node *nodePtr, const K &key) {
    List<K> &keys = nodePtr->keys;
    int pos = keys.binaryFind(key);
    if (pos < 0 || pos >= nodePtr->keys.getSize() || keys[pos] != key) {
        return -1;
    }
    keys.removeAt(pos);
    if (nodePtr->leaf) {
        nodePtr->values.removeAt(pos);
    } else {
        Node *node = nodePtr->childNodePtrs.get(pos);
        nodePtr->childNodePtrs.removeAt(pos);
        delete node;
    }
    return pos;
}

template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::fixNode(BPTree::Node *nodePtr) {
    if (minLoad <= nodePtr->keys.getSize() || !nodePtr->parentPtr) {
        return NULL;
    }

    List<K> &parentKeys = nodePtr->parentPtr->keys;
    int parentKeySize = parentKeys.getSize();
    K &key = nodePtr->keys[nodePtr->keys.getSize() - 1];
    int pos = nodePtr->parentPtr->keys.binaryFind(key);

    Node *sibling;
    // borrow from previous
    int previousIndex = pos - 1;
    bool hasPrevious = previousIndex >= 0 && previousIndex < parentKeySize;
    if (hasPrevious && (sibling = nodePtr->parentPtr->childNodePtrs[previousIndex])->keys.getSize() > minLoad) {
        int s = sibling->keys.getSize();
        K &oldKey = sibling->keys[s - 1];
        K &newKey = sibling->keys[s - 2];
        nodePtr->keys.insert(0, sibling->keys[s - 1]);

        if (nodePtr->leaf) {
            nodePtr->values.insert(0, sibling->values[s - 1]);
            sibling->values.removeAt(s - 1);
        } else {
            nodePtr->childNodePtrs.insert(0, sibling->childNodePtrs[s - 1]);
            sibling->childNodePtrs.get(s - 1)->parentPtr = nodePtr;
            sibling->childNodePtrs.removeAt(s - 1);
        }
        updateParentKey(sibling, oldKey, newKey, previousIndex);
        sibling->keys.removeAt(s - 1);
        return NULL;
    }

    // borrow from next
    int nextIndex = pos + 1;
    bool hasNext = nextIndex >= 0 && nextIndex < parentKeySize;
    if (hasNext && (sibling = nodePtr->parentPtr->childNodePtrs[nextIndex])->keys.getSize() > minLoad) {
        int s = nodePtr->keys.getSize();
        K &oldKey = nodePtr->keys[s - 1];
        K &newKey = sibling->keys[0];
        nodePtr->keys.add(newKey);

        if (nodePtr->leaf) {
            nodePtr->values.add(sibling->values[0]);
            sibling->values.removeAt(0);
        } else {
            nodePtr->childNodePtrs.add(sibling->childNodePtrs[0]);
            sibling->childNodePtrs.get(0)->parentPtr = nodePtr;
            sibling->childNodePtrs.removeAt(0);
        }
        updateParentKey(nodePtr, oldKey, newKey, pos);
        sibling->keys.removeAt(0);
        return NULL;
    }

    // merge with previous
    if (hasPrevious) {
        sibling = nodePtr->parentPtr->childNodePtrs[previousIndex];
        nodePtr->keys.insert(0, sibling->keys);
        if (nodePtr->leaf) {
            nodePtr->values.insert(0, sibling->values);
            if (sibling->previous) {
                sibling->previous->next = nodePtr;
            }
            nodePtr->previous = sibling->previous;
        } else {
            nodePtr->childNodePtrs.insert(0, sibling->childNodePtrs);
            int s = sibling->childNodePtrs.getSize();
            for (int i = 0; i < s; ++i) {
                sibling->childNodePtrs.get(i)->parentPtr = nodePtr;
            }
        }
        nodePtr->parentPtr->keys.removeAt(previousIndex);
        nodePtr->parentPtr->childNodePtrs.removeAt(previousIndex);
        delete sibling;
        return nodePtr->parentPtr;
    }

    if (hasNext) {
        sibling = nodePtr->parentPtr->childNodePtrs[nextIndex];
        sibling->keys.insert(0, nodePtr->keys);
        if (nodePtr->leaf) {
            sibling->values.insert(0, nodePtr->values);
            if (nodePtr->previous) {
                nodePtr->previous->next = sibling;
            }
            sibling->previous = nodePtr->previous;
        } else {
            sibling->childNodePtrs.insert(0, nodePtr->childNodePtrs);
            int s = nodePtr->childNodePtrs.getSize();
            for (int i = 0; i < s; ++i) {
                nodePtr->childNodePtrs.get(i)->parentPtr = sibling;
            }
        }
        sibling->parentPtr->keys.removeAt(pos);
        sibling->parentPtr->childNodePtrs.removeAt(pos);
        delete nodePtr;
        return sibling->parentPtr;
    }

    // parent is root
    if (nodePtr->parentPtr == root) {
        root->childNodePtrs.clear();
        root->values.add(nodePtr->values);
        root->keys.add(nodePtr->keys);
        return NULL;
    }

    return NULL;
}

template<typename K, typename V>
void BPTree<K, V>::updateParentKey(BPTree::Node *node, K &oldKey, K &newKey, int pos) {
    Node *tmpNodePtr = node;
    // delete old key, update parent
    while (tmpNodePtr && tmpNodePtr->parentPtr) {
        tmpNodePtr = tmpNodePtr->parentPtr;
        tmpNodePtr->keys[pos] = newKey;
        if (pos == tmpNodePtr->keys.getSize() - 1 && tmpNodePtr->parentPtr) {
            pos = tmpNodePtr->parentPtr->keys.binaryFind(oldKey);
            continue;
        }
        break;
    }
}

template<typename K, typename V>
void BPTree<K, V>::clear(BPTree::Node *node) {
    if (!node->leaf) {
        int s = node->childNodePtrs.getSize();
        for (int i = 0; i < s; ++i) {
            clear(node->childNodePtrs[i]);
        }
    }

    int s = node->childNodePtrs.getSize();
    for (int i = 0; i < s; ++i) {
        delete node->childNodePtrs[i];
    }
    node->childNodePtrs.clear();
    node->keys.clear();
    node->values.clear();
}

template<typename K, typename V>
void BPTree<K, V>::clear() {
    if (size == 0)
        return;
    clear(root);
    root->leaf = true;
    size = 0;
}

template<typename K, typename V>
bool BPTree<K, V>::containsKey(const K &key) {
    Node *curPtr = root;
    int pos;
    while (!curPtr->leaf) {
        int s = curPtr->childNodePtrs.getSize();
        List<K> constKeys = curPtr->keys;
        pos = constKeys.binaryFind(key);
        if (pos == s) {
            pos--;
        }
        curPtr = curPtr->childNodePtrs[pos];
    }
    const List<K> &constKeys = curPtr->keys;
    pos = constKeys.binaryFind(key);
    if (pos >= 0 && pos < curPtr->values.getSize()) {
        return compare(curPtr->keys[pos], key, comp) == 0;
    }

    return false;
}

template<typename K, typename V>
template<class BiApply>
void BPTree<K, V>::foreach(BiApply func) {
    Node *node = getFirstLeaf();
    while (node) {
        for (int i = 0, s = node->values.getSize(); i < s; ++i) {
            const K &constKey = node->keys[i];
            const V &constValue = node->values[i];
            if (func(constKey, constValue)) {
                return;
            }
        }
        node = node->next;
    }
}

template<typename K, typename V>
template<class BiApplyIndex>
void BPTree<K, V>::foreachIndex(BiApplyIndex func) {
    Node *node = getFirstLeaf();
    int index = 0;
    while (node) {
        for (int i = 0, s = node->values.getSize(); i < s; ++i, index++) {
            const K &constKey = node->keys[i];
            const V &constValue = node->values[i];
            if (func(index, constKey, constValue)) {
                return;
            }
        }
        node = node->next;
    }
}

template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::getFirstLeaf() {
    Node *node = root;
    while (!node->leaf) {
        node = node->childNodePtrs[0];
    }
    return node;
}


template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::getLastLeaf() {
    Node *node = root;
    while (!node->leaf) {
        node = node->childNodePtrs[node->childNodePtrs.getSize() - 1];
    }
    return node;
}

template<typename K, typename V>
template<class BiApply>
void BPTree<K, V>::foreachReverse(BiApply func) {
    Node *node = getLastLeaf();
    while (node) {
        for (int i = node->values.getSize() - 1; i >= 0; --i) {
            const K &constKey = node->keys[i];
            const V &constValue = node->values[i];
            if (func(constKey, constValue)) {
                return;
            }
        }
        node = node->previous;
    }
}

template<typename K, typename V>
template<class BiApplyIndex>
void BPTree<K, V>::foreachIndexReverse(BiApplyIndex func) {
    Node *node = getLastLeaf();
    int index = size - 1;
    while (node) {
        for (int i = node->values.getSize() - 1; i >= 0; --i, --index) {
            const K &constKey = node->keys[i];
            const V &constValue = node->values[i];
            if (func(index, constKey, constValue)) {
                return;
            }
        }
        node = node->previous;
    }
}

template<typename K, typename V>
void BPTree<K, V>::serialize(std::string &path) {
    if (!path.rfind(SUFFIX)) {
        path = path += SUFFIX;
    }
    char *cPath = const_cast<char *>(path.c_str());
    FILE *f = bp_tree_utils::fopen(cPath, "w");

    // head: tree info
    bp_tree_utils::fwrite("LYCBP", 1, 5, f);
    bp_tree_utils::writeValLittle((unsigned char) sizeof(unsigned short), f);
    bp_tree_utils::writeValLittle((unsigned char) sizeof(unsigned int), f);
    bp_tree_utils::writeValLittle((unsigned char) sizeof(unsigned long), f);
    bp_tree_utils::writeValLittle((unsigned int) sizeof(K), f);
    bp_tree_utils::writeValLittle((unsigned int) sizeof(V), f);
    bp_tree_utils::writeValLittle(order, f);
    bp_tree_utils::writeValLittle(initCap, f);
    bp_tree_utils::writeValLittle(size, f);
    if (size > 0) {
        const BPTree::Node *constRoot = root;
        serializeNode(constRoot, f);
    }
    fflush(f);
    fclose(f);
}

template<typename K, typename V>
long BPTree<K, V>::serializeNode(const BPTree::Node *node, FILE *f) {
    auto myOffset = ftell(f);

    bp_tree_utils::writeVal(node->leaf, f);
    const List<K> &keys = node->keys;
    unsigned int s = keys.getSize();
    assert(s >= minLoad || !node->parentPtr);
    bp_tree_utils::writeValLittle(s, f);
    for (int i = 0; i < s; ++i) {
        bp_tree_utils::writeValLittle(keys[i], f);
    }

    // leaf: write
    if (node->leaf) {
        for (int i = 0; i < s; ++i) {
            bp_tree_utils::writeValLittle(node->values[i], f);
        }
    } else {
        const List<Node *> &children = node->childNodePtrs;
        long childPtrsStartOffset = ftell(f);
        std::unique_ptr<long[]> childOffsets(new long[s]);
        bp_tree_utils::writeArrayLittle(childOffsets.get(), s, f);
        for (unsigned int i = 0; i < s; ++i) {
            const Node *nPtr = children[i];
            childOffsets[i] = serializeNode(nPtr, f);
        }
        long childEndOffset = ftell(f);
        fseek(f, childPtrsStartOffset, SEEK_SET);
        bp_tree_utils::writeArrayLittle(childOffsets.get(), s, f);
        fseek(f, childEndOffset, SEEK_SET);
    }

    return myOffset;
}

template<typename K, typename V>
std::shared_ptr<BPTree<K, V>> BPTree<K, V>::deserialize(const std::string &path) {
    return deserialize(path, NULL);
}

template<typename K, typename V>
std::shared_ptr<BPTree<K, V>> BPTree<K, V>::deserialize(const std::string &path, comparator<K> comp) {
    FILE *f = bp_tree_utils::fopen(path.c_str(), "r");
    char buf[6];
    bp_tree_utils::fread(buf, 1, 5, f);
    buf[5] = 0;
    if (strcmp(buf, "LYCBP") != 0) {
        throw bp_tree_utils::stringFormat("Check file header failed: expected 'LYCBP' but got '%s' (offset: 0)", buf);
    }

    bp_tree_utils::fread(buf, 1, 3, f);
    if (buf[0] != sizeof(short) || buf[1] != sizeof(unsigned int) || buf[2] != sizeof(long)) {
        throw std::string("this file is not compatible with machine");
    }
    auto sizeofK = bp_tree_utils::readVal<unsigned int>(f);
    if (sizeofK < sizeof(K)) {
        throw bp_tree_utils::stringFormat("Wrong sizeof(K): expected %d but got %d (offset: 8)", sizeof(K), sizeofK);
    }

    auto sizeofV = bp_tree_utils::readVal<unsigned int>(f);
    if (sizeofV != sizeof(V)) {
        throw bp_tree_utils::stringFormat("Wrong sizeof(V): expected %d but got %d (offset: 12)", sizeof(V), sizeofV);
    }


    auto order = bp_tree_utils::readVal<unsigned int>(f);
    if (order < 2) {
        throw bp_tree_utils::stringFormat("Wrong order: %d (offset: 16)", order);
    }

    auto initCap = bp_tree_utils::readVal<unsigned int>(f);
    if (initCap > order) {
        throw bp_tree_utils::stringFormat("Wrong initCap: %d (offset: 20)", initCap);
    }

    auto size = bp_tree_utils::readVal<unsigned int>(f);
    if (size < 0) {
        throw bp_tree_utils::stringFormat("Wrong size: %d (offset: 24)", initCap);
    }
    std::shared_ptr<BPTree> treePtr(new BPTree(order, initCap, comp));
    treePtr->size = size;

    if (size > 0) {
        treePtr->root = treePtr->deserializeNode(f, NULL);
        Node *pre = NULL;
        Node *node = treePtr->getFirstLeaf();
        while (node) {
            node->previous = pre;
            if (pre) {
                pre->next = node;
            }
            pre = node;
            node = treePtr->getNextSibling(node);
        }
    }

    Node *node = treePtr->getFirstLeaf();
    K *lastKey = NULL;
    int count = 0;
    int s;
    while (node) {
        s = node->values.getSize();
        for (int i = 0; i < s; ++i) {
            count++;
            if (lastKey) {
                const K &key = *lastKey;
                if (compare(node->keys[i], key, comp) <= 0)
                    throw std::string("invalid bp tree struct");
            }
            lastKey = &(node->keys[i]);
        }
        node = node->next;
    }
    if (count != size) {
        throw bp_tree_utils::stringFormat("Wrong size: expected %d but got %d", size, count);
    }

    fclose(f);
    return treePtr;
}

template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::deserializeNode(FILE *f, Node *parentNode) {
    bool leaf = bp_tree_utils::readVal<bool>(f) != 0;
    auto s = bp_tree_utils::readVal<unsigned int>(f);
    if ((parentNode && s < minLoad) || s > order) {
        throw bp_tree_utils::stringFormat("Illegal keys size: size: %d, order: %d(offset: %ld)",
                                          s, order, ftell(f) - sizeof(int));
    }
    Node *node = new Node(initCap, leaf);
    node->parentPtr = parentNode;
    for (int i = 0; i < s; ++i) {
        node->keys.add(bp_tree_utils::readVal<K>(f));
    }
    if (leaf) {
        for (int i = 0; i < s; ++i) {
            node->values.add(bp_tree_utils::readVal<V>(f));
        }
    } else {
        long offset;
        for (int i = 0; i < s; ++i) {
            long childOffset = bp_tree_utils::readVal<long>(f);
            offset = ftell(f);
            fseek(f, childOffset, SEEK_SET);
            node->childNodePtrs.add(deserializeNode(f, node));
            fseek(f, offset, SEEK_SET);
        }
    }

    return node;
}

template<typename K, typename V>
BPTree<K, V>::~BPTree() {
    clear(root);
    delete root;
}

template<typename K, typename V>
typename BPTree<K, V>::Node *BPTree<K, V>::getNextSibling(BPTree::Node *node) {
    if (!node) {
        return NULL;
    }
    if (!node->parentPtr) {
        return NULL;
    }
    K &key = node->keys[node->keys.getSize() - 1];
    int pos = node->parentPtr->keys.binaryFind(key);
    if (pos >= 0 && pos <= node->parentPtr->keys.getSize() && compare(key, node->parentPtr->keys[pos], comp) == 0) {
        if (pos == node->parentPtr->keys.getSize() - 1) {
            Node *parentSibling = getNextSibling(node->parentPtr);
            if (parentSibling && !parentSibling->childNodePtrs.isEmpty()) {
                return parentSibling->childNodePtrs[0];
            } else if (!parentSibling) {
                return NULL;
            }
        } else {
            return node->parentPtr->childNodePtrs[pos + 1];
        }
    }

    throw bp_tree_utils::stringFormat("the structure of the bp tree is not correct");
}


#endif //BPTREE_BPTREE_H
