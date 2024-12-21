#include "utils/math.hpp"

long div_floor(long a, long b) {
    long res = a / b;
    long rem = a % b;
    long corr = (rem != 0 && ((rem < 0) != (b < 0)));
    return res - corr;
}
