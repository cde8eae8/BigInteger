#include <iostream>
#include <limits>
#include "big_integer.h"

int main() {

    bool res = big_integer(50) < big_integer(100);
    std::cout << 50 << (res ? "<" : ">") << 100 << std::endl;



    res = big_integer(-50) < big_integer(-100);
    std::cout << -50 << (res ? "<" : ">") << -100 << std::endl;
//    (a % b, "0");
//
//    a /= b;
//    (a, "4");
//
//    c %= b;
//    (c, "0");
}