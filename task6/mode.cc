#include "mode.h"
#include <algorithm>

ShuffleMode::ShuffleMode(unsigned seed) : generator(seed), playOrder() {}

void SequenceMode::playInit(const std::vector<std::shared_ptr<Element>> &l) { now = l.begin(); }

void ShuffleMode::playInit(const std::vector<std::shared_ptr<Element>> &l) {
    playOrder = l;
    std::shuffle(playOrder.begin(), playOrder.end(), generator);
    now = playOrder.begin();
}

void OddEvenMode::playInit(const std::vector<std::shared_ptr<Element>> &l) {
    even = l.begin();
    now = std::next(even);
    end = l.end();
}

std::shared_ptr<Element> SequenceMode::next() { return *(now++); }

std::shared_ptr<Element> OddEvenMode::next() {
    auto prev = now;
    now++;
    if (now != end)
        now++;
    if (now == end)
        now = even;
    return *prev;
}

std::shared_ptr<Element> ShuffleMode::next() { return *(now++); }

std::shared_ptr<Mode> createSequenceMode() {
    return std::make_shared<SequenceMode>();
}

std::shared_ptr<Mode> createShuffleMode(unsigned seed) {
    return std::make_shared<ShuffleMode>(seed);
}

std::shared_ptr<Mode> createOddEvenMode() {
    return std::make_shared<OddEvenMode>();
}
