////
//// Created by nikita on 17.04.19.
////
//
//#include <algorithm>
//#include "NumberHolder.h"
//
//
//NumberHolder::NumberHolder() {
//    digits = allocate(INITIAL_SIZE);
//    this->size = INITIAL_SIZE;
//}
//
//
//
//void NumberHolder::resize(size_t new_size) {
//    uint64_t *tmp = allocate(new_size);
//    std::copy(this->digits, this->digits + this->size, tmp);
//    std::fill(tmp + this->size, tmp + new_size, UINT64_C(0));
//    this->digits = tmp;
//    this->size = new_size;
//}
//
//void NumberHolder::expandSign(size_t highestBit) {
//    bool negative = static_cast<int64_t>(digits[highestBit]) < 0;
//    std::fill(digits + highestBit + 1, digits + size, negative ? ~UINT64_C(0) : UINT64_C(0));
//}
//
//uint64_t *NumberHolder::allocate(size_t nElements) {
//    return static_cast<uint64_t*>(operator new(nElements * sizeof(uint64_t)));
//}
//
//uint64_t NumberHolder::get(size_t i) {
////    return
//}
