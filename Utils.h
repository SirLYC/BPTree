//
// Created by 刘育氚 on 2019/5/31.
//

#ifndef BPTREE_UTILS_H
#define BPTREE_UTILS_H

#include <cstdio>
#include <cerrno>
#include <string>

namespace bp_tree_utils {
    inline FILE *fopen(const char *path, const char *mode) {
        FILE *fptr = std::fopen(path, mode);
        if (!fptr) {
            throw std::string("cannot open file: ") + path;
        }
        return fptr;
    }

    inline void fwrite(const void *ptr, size_t size, size_t nitems,
                       FILE *stream) {
        if (std::fwrite(ptr, size, nitems, stream) != nitems) {
            throw "Write to file failed";
        }
    }

    inline void fread(void *ptr, size_t size, size_t nitems,
                      FILE *stream) {
        if (std::fread(ptr, size, nitems, stream) != nitems) {
            throw "Read from file failed";
        }
    }

    template<typename T>
    void writeVal(const T &val, FILE *stream) {
        bp_tree_utils::fwrite(&val, sizeof(T), 1, stream);
    }

    template<typename T>
    T readVal(FILE *stream) {
        T val;
        bp_tree_utils::fread(&val, sizeof(T), 1, stream);
        return val;
    }

    template<typename ... Args>
    std::string stringFormat(const std::string &format, Args ... args) {
        // https://stackoverflow.com/questions/35035982/is-snprintfnull-0-behavior-standardized
        // get size of result string
        int size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), static_cast<size_t>(size), format.c_str(), args ...);
        return std::string(buf.get(), buf.get() + size - 1);
    }
}

#endif //BPTREE_UTILS_H
