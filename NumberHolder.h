//
// Created by nikita on 17.04.19.
//

#ifndef BIGINTEGER_NUMBERHOLDER_H
#define BIGINTEGER_NUMBERHOLDER_H


#include <cstdint>
#include <cstdio>

struct NumberHolder {
    NumberHolder();
    ~NumberHolder();

    uint64_t operator[](size_t i);

 private:
    void resize(size_t new_size);
    void expandSign(size_t highestBit);
    uint64_t *allocate(size_t nElements);

    const size_t INITIAL_SIZE = 4;
    uint64_t *digits;
    size_t size;
};


#endif //BIGINTEGER_NUMBERHOLDER_H
