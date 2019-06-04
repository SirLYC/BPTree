//
// Created by 刘育氚 on 2019/5/27.
//

#ifndef BPTREE_LIST_H
#define BPTREE_LIST_H

#include <cstdlib>
#include <algorithm>
#include "Comp.h"


const static unsigned int MIN_CAP = 1;

template<typename T, class Comp = DefaultCompare<T>>
class List {
private:
    const Comp comp;
    unsigned int cap{};
    unsigned int size = 0;
    T *values;

    inline void expandTo(unsigned int cap);

    inline void initAllocate();

public:
    List() : List(2) {}

    explicit List(unsigned int cap) : cap(std::max(MIN_CAP, cap)), comp() {
        initAllocate();
    }

    explicit List(unsigned int cap, Comp comp) : cap(std::max(MIN_CAP, cap)), comp(comp) {
        values = (T *) malloc(byteSize(cap));
        if (!values) {
            throw "cannot malloc";
        }
    }

    List(List &list) : comp(list.comp), cap(list.cap), size(list.size) {
        values = (T *) malloc(byteSize(cap));
        if (!values) {
            throw "cannot malloc";
        }
        memcpy(values, list.values, byteSize(size));
    }

    List(const List &list) : comp(list.comp), cap(list.cap), size(list.size) {
        values = (T *) malloc(byteSize(cap));
        if (!values) {
            throw "cannot malloc";
        }
        memcpy(values, list.values, byteSize(size));
    }

    ~List() {
        free(values);
        values = NULL;
    }

    void set(int index, const T &value);

    void add(const T &value);

    void add(const List<T, Comp> &another);

    void add(const List<T, Comp> &another, int startIndex);

    void add(const List<T, Comp> &another, int startIndex, int endIndex);

    void insert(int index, const T &value);

    void insert(int index, const List<T, Comp> &another);

    // start: include
    // end: exclude
    void insert(int index, const List<T, Comp> &another, int startIndex, int count);

    void removeAt(int index);

    void removeRange(int start, int count);

    int binaryFind(const T &val) const;

    bool remove(T &value);

    T &operator[](int index);

    const T &operator[](int index) const;

    T &get(int index);

    unsigned int getSize() const;

    bool isEmpty();

    void reserve(unsigned int cap);

    void clear();

    inline void intelligentTrim();

    void trimToSize();

    inline void checkRange(int index) const;

    inline size_t byteSize(unsigned int count);
};

template<typename T, class Comp>
void List<T, Comp>::add(const List<T, Comp> &another) {
    insert(size, another);
}

template<typename T, class Comp>
void List<T, Comp>::add(const T &value) {
    insert(size, value);
}

template<typename T, class Comp>
void List<T, Comp>::add(const List<T, Comp> &another, int startIndex, int endIndex) {
    insert(size, another, startIndex, endIndex);
}

template<typename T, class Comp>
void List<T, Comp>::insert(int index, const T &value) {
    if (size + 1 >= cap) {
        expandTo(cap << 1);
    }
    memmove(values + index + 1, values + index, byteSize(size - index));
    values[index] = value;
    size++;
}

template<typename T, class Comp>
void List<T, Comp>::insert(int index, const List<T, Comp> &another) {
    unsigned int s = size;
    unsigned int cap = this->cap;
    while ((s + another.size) >= cap) {
        cap <<= 1;
    }
    expandTo(cap);

    memmove(values + another.size + index, values + index, byteSize(size - index));
    memcpy(values + index, another.values, byteSize(another.size));
    size += another.size;
}

template<typename T, class Comp>
void List<T, Comp>::insert(int index, const List<T, Comp> &another, int startIndex, int count) {
    if (count <= 0) return;
    another.checkRange(startIndex);
    another.checkRange(startIndex + count - 1);
    unsigned int s = size + count;
    unsigned int cap = this->cap;
    while (s >= cap) {
        cap <<= 1;
    }
    expandTo(cap);

    memmove(values + count + index, values + index, byteSize(size - index));
    memcpy(values + index, another.values, byteSize(count));
    size = s;
}

template<typename T, class Comp>
void List<T, Comp>::trimToSize() {
    if (cap > size) {
        cap = std::max(size, MIN_CAP);
        values = (T *) realloc(values, byteSize(cap));
    }
}

template<typename T, class Comp>
void List<T, Comp>::clear() {
    size = 0;
    intelligentTrim();
}

template<typename T, class Comp>
void List<T, Comp>::reserve(unsigned int cap) {
    this->cap = std::max(std::min(size, cap), MIN_CAP);
    expandTo(cap);
}

template<typename T, class Comp>
bool List<T, Comp>::isEmpty() {
    return !size;
}

template<typename T, class Comp>
T &List<T, Comp>::get(int index) {
    return this->operator[](index);
}

template<typename T, class Comp>
unsigned int List<T, Comp>::getSize() const {
    return size;
}

template<typename T, class Comp>
void List<T, Comp>::checkRange(int index) const {
    if (index < 0 || index >= size) {
        throw "index out of size";
    }
}

template<typename T, class Comp>
void List<T, Comp>::removeAt(int index) {
    checkRange(index);
    memmove(values + index, values + index + 1, byteSize(size - index - 1));
    size--;
    intelligentTrim();
}

template<typename T, class Comp>
bool List<T, Comp>::remove(T &value) {
    for (int i = 0; i < size; ++i) {
        if (comp(values[i], value) == 0) {
            removeAt(i);
            return true;
        }
    }

    return false;
}

template<typename T, class Comp>
void List<T, Comp>::set(int index, const T &value) {
    if (index >= 0 && index < size) {
        values[index] = value;
    } else {
        throw "index out of size";
    }
}

template<typename T, class Comp>
void List<T, Comp>::expandTo(unsigned int cap) {
    values = (T *) realloc(values, byteSize(cap));
    if (!values) {
        throw "re-alloc failed!";
    }
    this->cap = cap;
}

template<typename T, class Comp>
void List<T, Comp>::removeRange(int start, int count) {
    int endIndex = start + count - 1;
    if (endIndex < start) {
        return;
    }
    checkRange(start);
    checkRange(endIndex);
    memmove(values + start, values + endIndex + 1, byteSize(size - endIndex - 1));
    size -= count;
    intelligentTrim();
}

template<typename T, class Comp>
size_t List<T, Comp>::byteSize(unsigned int count) {
    return count * sizeof(T);
}

template<typename T, class Comp>
T &List<T, Comp>::operator[](int index) {
    checkRange(index);
    return values[index];
}

template<typename T, class Comp>
const T &List<T, Comp>::operator[](int index) const {
    checkRange(index);
    return values[index];
}

template<typename T, class Comp>
void List<T, Comp>::add(const List<T, Comp> &another, int startIndex) {
    add(another, startIndex, another.size - startIndex);
}

template<typename T, class Comp>
inline int List<T, Comp>::binaryFind(const T &val) const {
    if (!size) {
        return 0;
    }

    int high = size;
    int low = 0;
    int mid;
    int toIndex = -1;
    int c;

    while (low < high) {
        mid = (low + high) >> 1;
        c = comp(val, values[mid]);
        if (c == 0) {
            toIndex = mid;
            break;
        } else if (c > 0) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    if (toIndex == -1) {
        toIndex = low;
    }
    return toIndex;
}

template<typename T, class Comp>
inline void List<T, Comp>::intelligentTrim() {
    if (size < (cap >> 1)) {
        trimToSize();
    }
}

template<typename T, class Comp>
void List<T, Comp>::initAllocate() {
    values = (T *) malloc(byteSize(cap));
    if (!values) {
        throw "cannot malloc";
    }
}

#endif //BPTREE_LIST_H
