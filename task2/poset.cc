#include <cassert>

#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "poset.h"

#define ERROR(...)                                                             \
    do {                                                                       \
        if (debug) {                                                           \
            const std::string header = RED + "ERROR" + RESET + ": ";                 \
            logDebug(header, __PRETTY_FUNCTION__, __VA_ARGS__);                \
            assert(false);                                                     \
        }                                                                      \
    } while (false)
#define INFO(...)                                                              \
    do {                                                                       \
        if (debug) {                                                           \
            const std::string header = GREEN + "INFO" + RESET + ": ";                \
            logDebug(header, __PRETTY_FUNCTION__, __VA_ARGS__);                \
        }                                                                      \
    } while (false)

// the following are done as macros, so that the __PRETTY_FUNCTION__ macro
// indicates correct function
#define POSET_NOT_FOUND(id) INFO("poset does not exit, id=", (id))
#define RETURNS(val) INFO("returns ", (val))

namespace {
enum PosetElem { NORMAL, REVERSED, NAMES, NEXT_FREE_SPOT };

using ID = unsigned long;
using NameToId = std::unordered_map<std::string, ID>;
using NeighbourSet = std::unordered_set<ID>;
using AdjacencyLists = std::unordered_map<ID, NeighbourSet>;
using Poset = std::tuple<AdjacencyLists, AdjacencyLists, NameToId, size_t>;

#ifdef NDEBUG
const bool debug = false;
#else
const bool debug = true;
#endif

const std::string RED = "\e[31m", GREEN = "\e[32m", RESET = "\e[39m";

std::unordered_map<ID, Poset> &posets() {
    static std::unordered_map<ID, Poset> posets_ = {};
    return posets_;
}

template <typename T> void logDebug(T t) { std::cerr << t << std::endl; }

template <typename T, typename... Args> void logDebug(T t, Args... args) {
    std::ios_base::Init();
    std::cerr << t;
    logDebug(args...);
}

template <typename T, typename T_elem>
void assert_contains(T const &container, T_elem const &elem) {
    if (debug) {
        if (container.find(elem) == container.end()) {
            ERROR("container doesn't hold element");
        }
    }
}

std::string quoted_or_null(const char *str) {
    if (str == nullptr) {
        return "nullptr";
    }
    return "\"" + std::string{str} + "\"";
}

void assert_poset_contains_ids(Poset const &poset, ID name1_id, ID name2_id) {
    assert_contains(std::get<NORMAL>(poset), name1_id);
    assert_contains(std::get<REVERSED>(poset), name1_id);
    assert_contains(std::get<NORMAL>(poset), name2_id);
    assert_contains(std::get<REVERSED>(poset), name2_id);
}

void add_relation_unchecked(Poset &poset, ID name1_id, ID name2_id) {
    INFO("calling with args: ", name1_id, ", ", name2_id);
    assert_poset_contains_ids(poset, name1_id, name2_id);
    NeighbourSet &uppers = std::get<NORMAL>(poset)[name2_id];
    uppers.insert(name1_id);
    NeighbourSet &lowers = std::get<REVERSED>(poset)[name1_id];
    lowers.insert(name2_id);
}

void del_relation_unchecked(Poset &poset, ID name1_id, ID name2_id) {
    INFO("calling with args: ", name1_id, ", ", name2_id);
    assert_poset_contains_ids(poset, name1_id, name2_id);
    NeighbourSet &uppers = std::get<NORMAL>(poset).at(name2_id);
    uppers.erase(name1_id);
    NeighbourSet &lowers = std::get<REVERSED>(poset).at(name1_id);
    lowers.erase(name2_id);
}

bool test_relation_unchecked(Poset const &poset, ID name1_id, ID name2_id) {
    INFO("calling with args: ", name1_id, ", ", name2_id);
    assert_poset_contains_ids(poset, name1_id, name2_id);
    NeighbourSet const &uppers = std::get<NORMAL>(poset).at(name2_id);
    if (debug) {
        NeighbourSet const &lowers = std::get<REVERSED>(poset).at(name1_id);
        if (lowers.count(name2_id) != uppers.count(name1_id)) {
            ERROR("REVERSED is not the reverse of NORMAL");
        }
    }
    return static_cast<bool>(uppers.count(name1_id));
}

bool in_between(Poset const &poset, ID name1_id, ID name2_id) {
    assert_poset_contains_ids(poset, name1_id, name2_id);
    NeighbourSet const &uppers = std::get<NORMAL>(poset).at(name2_id);
    for (auto upper : uppers) {
        if (upper != name1_id and
            test_relation_unchecked(poset, name1_id, upper)) {
            return true;
        }
    }
    return false;
}
} // namespace

namespace jnp1 {
ID poset_new(void) {
    INFO("called");
    static size_t next_free_poset = {};
    posets()[next_free_poset] = {};
    INFO("id=", next_free_poset);
    return next_free_poset++;
}

void poset_delete(ID id) {
    INFO("id=", id);
    POSET_NOT_FOUND(id);
    posets().erase(id);
}

size_t poset_size(ID id) {
    INFO("id=", id);
    auto it = posets().find(id);
    if (it == posets().end()) {
        INFO("poset does not exits, id=", id);
        return 0;
    }
    size_t size = std::get<NAMES>(it->second).size();
    INFO("size=", size, ", id=", id);
    return size;
}

bool poset_insert(ID id, char const *value) {
    INFO("id=", id, ", value=", quoted_or_null(value));
    if (value == nullptr) {
        INFO("invalid value: nullptr");
        RETURNS(false);
        return false;
    }
    auto it = posets().find(id);
    if (it == posets().end()) {
        POSET_NOT_FOUND(id);
        RETURNS(false);
        return false;
    }

    Poset &p = it->second;
    if (std::get<NAMES>(p).count(value) > 0) {
        INFO("poset already contains value=\"", value);
        RETURNS(false);
        return false;
    }
    ID &free = std::get<NEXT_FREE_SPOT>(p);
    INFO("value \"", value, "\" gets id=", free);
    std::get<NORMAL>(p)[free] = {};
    std::get<REVERSED>(p)[free] = {};
    std::get<NAMES>(p)[value] = free++;
    RETURNS(true);
    return true;
}

bool poset_remove(ID id, char const *value) {
    INFO("id=", id, ", value=", quoted_or_null(value));
    if (value == nullptr) {
        INFO("invalid value: nullptr");
        RETURNS(false);
        return false;
    }
    auto poset_it = posets().find(id);
    if (poset_it == posets().end()) {
        POSET_NOT_FOUND(id);
        RETURNS(false);
        return false;
    }
    Poset &poset = poset_it->second;
    auto name_iter = std::get<NAMES>(poset).find(value);
    if (name_iter == std::get<NAMES>(poset).end()) {
        RETURNS(false);
        return false;
    }
    ID name_id = name_iter->second;
    NeighbourSet uppers = std::get<NORMAL>(poset).at(name_id);
    for (auto upper : uppers)
        del_relation_unchecked(poset, upper, name_id);
    NeighbourSet lowers = std::get<REVERSED>(poset).at(name_id);
    for (auto lower : lowers)
        del_relation_unchecked(poset, name_id, lower);
    std::get<NAMES>(poset).erase(value);
    std::get<NORMAL>(poset).erase(name_id);
    std::get<REVERSED>(poset).erase(name_id);
    RETURNS(true);
    return true;
}

bool poset_add(ID id, char const *value1, char const *value2) {
    INFO("id=", id, ", value1=", quoted_or_null(value1),
         ", value2=", quoted_or_null(value2));
    if (value1 == nullptr or value2 == nullptr) {
        INFO("invalid value: nullptr");
        RETURNS(false);
        return false;
    }
    auto poset_it = posets().find(id);
    if (poset_it == posets().end()) {
        POSET_NOT_FOUND(id);
        RETURNS(false);
        return false;
    }
    Poset &poset = poset_it->second;
    NameToId const &names = std::get<NAMES>(poset);
    if (names.count(value1) == 0 || names.count(value2) == 0) {
        INFO("one of the vertices doesn't exist");
        RETURNS(false);
        return false;
    }
    ID name1_id = names.at(value1);
    ID name2_id = names.at(value2);
    if (test_relation_unchecked(poset, name1_id, name2_id) ||
        test_relation_unchecked(poset, name2_id, name1_id) ||
        name1_id == name2_id) {
        INFO("vertices are already in a relation");
        RETURNS(false);
        return false;
    }
    add_relation_unchecked(poset, name1_id, name2_id);
    NeighbourSet &uppers = std::get<NORMAL>(poset).at(name1_id);
    NeighbourSet &lowers = std::get<REVERSED>(poset).at(name2_id);
    for (auto lower_it : lowers) {
        for (auto upper_it : uppers)
            add_relation_unchecked(poset, upper_it, lower_it);
    }
    for (auto lower_it : lowers) {
        add_relation_unchecked(poset, name1_id, lower_it);
    }
    for (auto upper_it : uppers) {
        add_relation_unchecked(poset, upper_it, name2_id);
    }
    RETURNS(true);
    return true;
}

bool poset_del(ID id, char const *value1, char const *value2) {
    INFO("id=", id, ", value1=", quoted_or_null(value1),
         ", value2=", quoted_or_null(value2));
    if (value1 == nullptr or value2 == nullptr) {
        INFO("invalid value: nullptr");
        RETURNS(false);
        return false;
    }
    auto poset_it = posets().find(id);
    if (poset_it == posets().end()) {
        POSET_NOT_FOUND(id);
        RETURNS(false);
        return false;
    }
    Poset &poset = poset_it->second;
    NameToId const &names = std::get<NAMES>(poset);
    if (names.count(value1) == 0 || names.count(value2) == 0) {
        RETURNS(false);
        return false;
    }
    ID name1_id = names.at(value1);
    ID name2_id = names.at(value2);
    if (name1_id == name2_id) {
        INFO("deleting would break reflexivity");
        RETURNS(false);
        return false;
    }
    if (!test_relation_unchecked(poset, name1_id, name2_id)) {
        INFO("vertices are not in relation");
        RETURNS(false);
        return false;
    }
    if (in_between(poset, name1_id, name2_id)) {
        INFO("deleting would break transitivity");
        RETURNS(false);
        return false;
    }
    del_relation_unchecked(poset, name1_id, name2_id);
    RETURNS(true);
    return true;
}

bool poset_test(ID id, char const *value1, char const *value2) {
    INFO("id=", id, ", value1=", quoted_or_null(value1),
         ", value2=", quoted_or_null(value2));
    if (value1 == nullptr or value2 == nullptr) {
        INFO("invalid value: nullptr");
        RETURNS(false);
        return false;
    }
    auto poset_it = posets().find(id);
    if (poset_it == posets().end()) {
        POSET_NOT_FOUND(id);
        return false;
    }
    Poset const &poset = poset_it->second;
    NameToId const &names = std::get<NAMES>(poset);
    if (names.count(value1) == 0 || names.count(value2) == 0) {
        INFO("poset (id=", id, ") doesn't hold either of the values");
        RETURNS(false);
        return false;
    }
    ID name1_id = names.at(value1);
    ID name2_id = names.at(value2);
    bool ret = std::string{value1} == std::string{value2} or
               test_relation_unchecked(poset, name1_id, name2_id);
    RETURNS(ret);
    return ret;
}

void poset_clear(ID id) {
    INFO("id=", id);
    if (posets().count(id) != 0) {
        Poset &poset = posets().at(id);
        std::get<NORMAL>(poset).clear();
        std::get<REVERSED>(poset).clear();
        std::get<NAMES>(poset).clear();
    } else {
        POSET_NOT_FOUND(id);
    }
}
} // namespace jnp1
