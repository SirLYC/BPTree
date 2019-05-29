//
// Created by 刘育氚 on 2019/5/27.
//

#ifndef BPTREE_LIST_H
#define BPTREE_LIST_H

#include <cstdlib>
#include <algorithm>
#include "Comp.h"

template<typename T>
class List {
private:
    comparator<T> comp;
    int cap{};
    int size = 0;
    T *values;

    inline void expandTo(int cap);

public:
    List() : List(2, NULL) {}

    explicit List(int cap) : List(cap, NULL) {}

    explicit List(int cap, comparator<T> comp) : cap(std::max(1, cap)), comp(comp) {
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
    }

    void set(int index, const T &value);

    void add(const T &value);

    void add(const List<T> &another);

    void add(const List<T> &another, int startIndex);

    void add(const List<T> &another, int startIndex, int endIndex);

    void insert(int index, const T &value);

    void insert(int index, const List<T> &another);

    // start: include
    // end: exclude
    void insert(int index, const List<T> &another, int startIndex, int count);

    void removeAt(int index);

    void removeRange(int start, int count);

    int binaryFind(const T &val) const;

    bool remove(T &value);

    T &operator[](int index);

    const T &operator[](int index) const;

    T &get(int index);

    int getSize();

    bool isEmpty();

    void reserve(int cap);

    void clear();

    void trimToSize();

    inline void checkRange(const int index) const;

    inline size_t byteSize(int count);
};

template<typename T>
void List<T>::add(const List<T> &another) {
    insert(size, another);
}

template<typename T>
void List<T>::add(const T &value) {
    insert(size, value);
}

template<typename T>
void List<T>::add(const List<T> &another, int startIndex, int endIndex) {
    insert(size, another, startIndex, endIndex);
}

template<typename T>
void List<T>::insert(int index, const T &value) {
    if (size + 1 >= cap) {
        expandTo(cap << 1);
    }
    memmove(values + index + 1, values + index, byteSize(size - index));
    values[index] = value;
    size++;
}

template<typename T>
void List<T>::insert(int index, const List<T> &another) {
    int s = size;
    int cap = this->cap;
    while ((s + another.size) >= cap) {
        cap <<= 1;
    }
    expandTo(cap);

    memmove(values + another.size + index, values + index, byteSize(size - index));
    memcpy(values + index, another.values, byteSize(another.size));
    size += another.size;
}

template<typename T>
void List<T>::insert(int index, const List<T> &another, int startIndex, int count) {
    if (count <= 0) return;
    another.checkRange(startIndex);
    another.checkRange(startIndex + count - 1);
    int s = size + count;
    int cap = this->cap;
    while (s >= cap) {
        cap <<= 1;
    }
    expandTo(cap);

    memmove(values + count + index, values + index, byteSize(size - index));
    memcpy(values + index, another.values, byteSize(count));
    size = s;
}

template<typename T>
void List<T>::trimToSize() {
    if (cap > size) {
        cap = std::max(size, 1);
        values = (T *) realloc(values, byteSize(cap));
    }
}

template<typename T>
void List<T>::clear() {
    size = 0;
}

template<typename T>
void List<T>::reserve(int cap) {
    this->cap = std::min(size, cap);
    expandTo(cap);
}

template<typename T>
bool List<T>::isEmpty() {
    return !size;
}

template<typename T>
T &List<T>::get(int index) {
    return this->operator[](index);
}

template<typename T>
int List<T>::getSize() {
    return size;
}

template<typename T>
void List<T>::checkRange(const int index) const {
    if (index < 0 || index >= size) {
        throw "index out of size";
    }
}

template<typename T>
void List<T>::removeAt(int index) {
    checkRange(index);
    memmove(values + index, values + index + 1, byteSize(size - index - 1));
    size--;
}

template<typename T>
bool List<T>::remove(T &value) {
    for (int i = 0; i < size; ++i) {
        if (compare(values[i], value, comp) == 0) {
            removeAt(i);
            return true;
        }
    }

    return false;
}

template<typename T>
void List<T>::set(int index, const T &value) {
    if (index >= 0 && index < size) {
        values[index] = value;
    } else {
        throw "index out of size";
    }
}

template<typename T>
void List<T>::expandTo(int cap) {
    values = (T *) realloc(values, byteSize(cap));
    if (!values) {
        throw "re-alloc failed!";
    }
    this->cap = cap;
}

template<typename T>
void List<T>::removeRange(int start, int count) {
    int endIndex = start + count - 1;
    if (endIndex < start) {
        return;
    }
    checkRange(start);
    checkRange(endIndex);
    memmove(values + start, values + endIndex + 1, byteSize(size - endIndex - 1));
    size -= count;
}

template<typename T>
size_t List<T>::byteSize(int count) {
    return count * sizeof(T);
}

template<typename T>
T &List<T>::operator[](int index) {
    checkRange(index);
    return values[index];
}

template<typename T>
const T &List<T>::operator[](int index) const {
    checkRange(index);
    return values[index];
}

template<typename T>
void List<T>::add(const List<T> &another, int startIndex) {
    add(another, startIndex, another.size - startIndex);
}

template<typename T>
inline int List<T>::binaryFind(const T &val) const {
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
        c = compare(val, values[mid], comp);
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

#endif //BPTREE_LIST_H
