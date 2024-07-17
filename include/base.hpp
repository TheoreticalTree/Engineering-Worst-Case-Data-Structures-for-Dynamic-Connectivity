#ifndef GKKT_BASE_HPP
#define GKKT_BASE_HPP

#include <cstdint>
#include <limits>

using count = uint64_t;
using node = uint64_t;

using diff = int64_t;
using cost = int64_t;
using costDiff = int64_t;

struct edge {
    node v = 0, w = 0;

    constexpr edge& operator^=(const edge& e){
        v ^= e.v;
        w ^= e.w;
        return *this;
    }
};

constexpr edge operator^(edge e1, const edge& e2){
    return e1 ^= e2;
}

constexpr bool operator==(const edge& e1, const edge& e2){
    return e1.v == e2.v && e1.w == e2.w;
}

constexpr node none = std::numeric_limits<node>::max();

constexpr edge noEdge = {0, 0};

#endif //GKKT_BASE_HPP
