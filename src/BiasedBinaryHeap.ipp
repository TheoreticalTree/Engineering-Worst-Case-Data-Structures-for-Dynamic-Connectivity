#include "stdexcept"

#include "BiasedBinaryHeap.hpp"

template<class key, class val>
BiasedBinaryHeap<key, val>::BiasedBinaryHeap(std::function<bool(key, key)> pLess) {
    less = pLess;
}

template<class key, class val>
void BiasedBinaryHeap<key, val>::insert(key k, val v, count w) {
    entries.push_back({k, v});
    weightSum += w;
}

template<class key, class val>
val BiasedBinaryHeap<key, val>::remove(key k) {
    weightSum -= k.first;
    for(count i = 0; i < entries.size(); i++){
        if(entries[i].first == k){
            val ret = entries[i].second;
            entries[i] = entries[entries.size() - 1];
            entries.pop_back();
            return ret;
        }
    }

    throw std::runtime_error("Key not stored");
}

template<class key, class val>
val BiasedBinaryHeap<key, val>::changeVal(key k, val v) {
    for(count i = 0; i < entries.size(); i++){
        if(entries[i].first == k){
            val ret = entries[i].second;
            entries[i].second = v;
            return ret;
        }
    }

    throw std::runtime_error("Key not stored, can't be changed");
}

template<class key, class val>
std::pair<key, val> BiasedBinaryHeap<key, val>::getMax() {
    if(entries.empty()) std::runtime_error("No values present");

    std::pair<key, val> max = entries[0];
    for(count i = 1; i < entries.size(); i++){
        if(less(max.first, entries[i].first)){
            max = entries[i];
        }
    }

    return max;
}

template<class key, class val>
bool BiasedBinaryHeap<key, val>::isEmpty() {
    return entries.empty();
}

template<class key, class val>
bool BiasedBinaryHeap<key, val>::contains(key k) {
    for(std::pair<key, val> element : entries){
        if (element.first == k) return true;
    }
    return false;
}

template<class key, class val>
count BiasedBinaryHeap<key, val>::getWeightSum() {
    return weightSum;
}