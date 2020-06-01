#pragma once

#include <list>
#include <memory>
#include <unordered_map>
#include <functional>

class lookup_error : public std::exception
{
    public:
    virtual const char* what() const noexcept override {
        return "lookup_error: element wasn't found in the collection";
    }
};

namespace details
{
    /*
        Map implementing insertion_ordered_map functionality without copy-on-write.

        It has strong exception safety guarantees.
    */
    template <class K, class V, class Hash = std::hash<K>>
    class BackendMap
    {
    private:
        struct PtrHash;
        struct PtrEqual;

        using sequential_container = std::list<std::pair<K, V>>;
        using assoc_container = std::unordered_map<const K*, typename sequential_container::iterator, PtrHash, PtrEqual>;

    public:
        BackendMap() = default;
        BackendMap(const BackendMap& other);
        BackendMap(BackendMap&& other) noexcept = default;

        BackendMap& operator=(const BackendMap& other);
        BackendMap& operator=(BackendMap&& other) noexcept = default;

        bool insert(const K& key, const V& value);

        void erase(const K& key);

        void merge(const BackendMap& other);

        V& at(const K& key);
        const V& at(const K& key) const;

        V& operator[](const K& key);

        std::size_t size() const noexcept;
        bool empty() const noexcept;

        void clear();

        bool contains(const K& key) const noexcept(noexcept(assoc_container().find( static_cast<const K*>(nullptr) )));

        using iterator = typename sequential_container::const_iterator;
        using const_iterator = iterator;

        iterator begin() const noexcept;
        iterator end() const noexcept;

    private:
        struct PtrHash : public Hash {
            std::size_t operator()(const K* p) const {
                return Hash::operator()(*p);
            }
        };

        struct PtrEqual {
            bool operator()(const K* p1, const K* p2) const {
                return *p1 == *p2;
            }
        };

        assoc_container map;
        sequential_container list;

        bool insert(const K& key, const V& value, typename assoc_container::iterator* it);
    };

    /*
        Lazy container handles storing some class in a copy-on-write manner
        
        All references remain valid until some mutable change is completed.
        If mutable change throws then old state is restored and references remain valid
        (assuming underlying class has strong exception guarantees).
    */
    template <class T>
    class LazyContainer
    {
    public:
        LazyContainer();

        const T& getConst() const;

        // Please don't save mutable references
        void doMutable(const std::function<void(T&)>&);

        void doMutableAndSaveReference(const std::function<void(T&)>&);

        LazyContainer(const LazyContainer& other);
        LazyContainer(LazyContainer&& other);

        LazyContainer& operator=(const LazyContainer& other);
        LazyContainer& operator=(LazyContainer&& other);
        
        // If can avoid copying then calls clear() else makes new default data.
        void clearInternal();

    private:
        struct Data
        {
            T t;
            bool issuedMutableReference = false;
        };

        std::shared_ptr<Data> dataPtr;
    };
}

/*
    insertion_odered_map uses LazyContainer to store BackendMap

    Both LazyContainer and BackendMap have strong exception guarantees 
    making insertion_ordered_map also strongly exception safe. All operations
    are delegated to LazyContainer - that's why we didn't comment on 
    insertion_ordered_map's methods and their relations with string exception safety.
*/
template <class K, class V, class Hash = std::hash<K>>
class insertion_ordered_map
{
private:
    using backend_map = details::BackendMap<K, V, Hash>;

public:
    insertion_ordered_map() = default;
    insertion_ordered_map(const insertion_ordered_map& other) = default;
    insertion_ordered_map(insertion_ordered_map&& other) noexcept = default;

    insertion_ordered_map& operator=(insertion_ordered_map other);
    insertion_ordered_map& operator=(insertion_ordered_map&& other) noexcept = default;

    bool insert(const K& key, const V& value);

    void erase(const K& key);

    void merge(const insertion_ordered_map& other);

    V& at(const K& key);
    const V& at(const K& key) const;

    V& operator[](const K& key);

    std::size_t size() const noexcept;
    bool empty() const noexcept;

    void clear();

    bool contains(const K& key) const noexcept(noexcept(backend_map().contains( std::declval<K>() )));

    using iterator = typename backend_map::const_iterator;
    using const_iterator = iterator;

    iterator begin() const noexcept;
    iterator end() const noexcept;

private:

    details::LazyContainer<backend_map> container;
};


namespace details
{
    template<class K, class V, class H>
    BackendMap<K, V, H>::BackendMap(const BackendMap& other) {
        
        // Exception safety is guaranteed by operator=(const BackendMap&).
        *this = other;
    }

    template<class K, class V, class H>
    BackendMap<K, V, H>& BackendMap<K, V, H>::operator=(const BackendMap& other) {

        // Exception safety is achieved by first creating new containers and then noexcept assigning.
        if (this == &other) {
            return *this;
        }

        sequential_container newList = other.list;
        assoc_container newMap;

        for(auto it = newList.begin(); it != newList.end(); it++)
            newMap.emplace(&it->first, it);

        static_assert(noexcept( std::declval<sequential_container>() = std::move(std::declval<sequential_container>()) ));
        static_assert(noexcept( std::declval<assoc_container>() = std::move(std::declval<assoc_container>()) ));
        map = std::move(newMap);
        list = std::move(newList); 

        return *this;
    }

    template <class K, class V, class H>
    bool BackendMap<K, V, H>::insert(const K& key, const V& value) {

        // Exception safety is guaranteed by insert(key, value, &it).
        typename assoc_container::iterator it;
        return insert(key, value, &it);
    }

    template <class K, class V, class H>
    bool BackendMap<K, V, H>::insert(const K& key, const V& value, typename assoc_container::iterator* it) {
        auto findIt = map.find(&key);

        if(findIt != map.end()) {

            auto elementIt = findIt->second;
            list.splice(list.end(), list, elementIt);
            *it = findIt;

            return false;
        }
        // Until now nothing has changed so it's exception safe.

        // Insert into list - if exception happens: OK state hasn't changed.
        list.emplace_back(key, value);

        try {
            *it = map.emplace(&list.back().first, std::prev(list.end())).first;
        } catch(...) {
            // Now if inserting into map fails we have to remove item from the list.
            // Otherwise state would be invalid.
            static_assert(noexcept( sequential_container().pop_back() ));
            list.pop_back();
            throw;
        }

        return true;
    }

    template <class K, class V, class H>
    V &BackendMap<K, V, H>::at(K const& k) {
        
        // Nothing is changed so it's exception safe.

        auto it = map.find(&k);
        if (it == map.end()) {
            throw lookup_error();
        }
        return (it->second)->second;
    }

    template <class K, class V, class H>
    V const&BackendMap<K, V, H>::at(K const& k) const {

        // Nothing is changed so it's exception safe.

        auto it = map.find(&k);
        if (it == map.end()) {
            throw lookup_error();
        }
        auto& KVpair = it->second;
        return KVpair->second;
    }

    template <class K, class V, class H>
    V& BackendMap<K, V, H>::operator[](K const& k) {

        // If `contains` throws, then we haven't modified the map. Otherwise,
        // after the only modification, we don't do any throwing operations.
        
        if (!contains(k)) {
            typename assoc_container::iterator it;
            insert(k, {}, &it);
            return it->second->second;
        }
        return at(k);
    }

    template <class K, class V, class H>
    bool BackendMap<K, V, H>::contains(const K& key) const noexcept(noexcept(assoc_container().find( static_cast<const K*>(nullptr) ))) {
        
        // Nothing is changed so it's exception safe.
        return (map.find(&key) != map.end());
    }

    template <class K, class V, class H>
    typename BackendMap<K, V, H>::iterator BackendMap<K, V, H>::begin() const noexcept {
        
        // Nothing is changed so it's exception safe.
        return list.begin();
    }

    template <class K, class V, class H>
    typename BackendMap<K, V, H>::iterator BackendMap<K, V, H>::end() const noexcept {
        
        // Nothing is changed so it's exception safe.
        return list.end();
    }

    template <class K, class V, class H>
    void BackendMap<K, V, H>::clear() {

        // According to c++ guidelines, clear() shouldn't throw.

        list.clear();
        map.clear();
    }

    template <class K, class V, class H>
    bool BackendMap<K, V, H>::empty() const noexcept {
        // Nothing is changed so it's exception safe.
        return map.empty();
    }


    template <class K, class V, class H>
    std::size_t BackendMap<K, V, H>::size() const noexcept {
        // Nothing is changed so it's exception safe.
        return map.size();
    }


    template <class K, class V, class H>
    void BackendMap<K, V, H>::erase(const K& key) {

        if (!contains(key))
            throw lookup_error();

        auto it = map.find(&key);
        // it != map.end(), as it was checked before,

        // Until now nothing is changed so it's exception safe.

        // Erasing from list by iterator is exception safe so after erasing form map we can use it safely.
        static_assert(noexcept( sequential_container().erase(std::declval<typename sequential_container::iterator>()) ));

        auto listIt = it->second;
        map.erase(it);
        list.erase(listIt); 
    }

    template <class K, class V, class H>
    void BackendMap<K, V, H>::merge(const BackendMap& other) {

        // Creating a separate copy for merge ensures that if something goes wrong
        // original map remains unchanged

        BackendMap<K, V, H> tmp(*this);
        
        for (auto& el: other) {
            tmp.insert(el.first, el.second);
        }

        // After successfull merge merged map is noexcept moved into this
        static_assert(noexcept( BackendMap() = std::move(BackendMap()) ));
        *this = std::move(tmp);
    }

    template<class T>
    LazyContainer<T>::LazyContainer()
    {
        // If std::make_shared throws object isn't constructed properly and that's OK
        dataPtr = std::make_shared<Data>();
    }

    template<class T>
    const T& LazyContainer<T>::getConst() const
    {
        // Nothing is changed so it's exception safe
        return dataPtr->t;
    }

    template<class T>
    void LazyContainer<T>::doMutable(const std::function<void(T&)>& toDo)
    {
        if(dataPtr.use_count() > 1)
        {
            // In case we need to create a copy of underlying data
            // we first make a copy and run function toDo on it
            // to ensure that in case something goes wrong original copy is still unmodified

            std::shared_ptr<Data> newDataPtr = std::make_shared<Data>();
            newDataPtr->t = dataPtr->t;

            toDo(newDataPtr->t);

            // After succesfull operation we noexcept move new data to dataPtr
            static_assert(noexcept( std::shared_ptr<Data>() = std::move(std::shared_ptr<Data>()) ));
            dataPtr = std::move(newDataPtr);
        }
        else
        {
            // When no copy is needed we depend on underlying class being strongly exception safe
            toDo(dataPtr->t);
        }

        dataPtr->issuedMutableReference = false;
    }

    template<class T>
    void LazyContainer<T>::doMutableAndSaveReference(const std::function<void(T&)>& toDo)
    {
        // Exception safety is guaranteed by doMutable()
        doMutable(toDo);
        dataPtr->issuedMutableReference = true;
    }

    template<class T>
    void LazyContainer<T>::clearInternal()
    {
        // In case std::make_shared<Data>() throws dataPtr is not modified
        // After make_shared has finished nothing can go wrong
        static_assert(noexcept( std::shared_ptr<Data>() = std::shared_ptr<Data>() ));

        if(dataPtr.use_count() > 1)
            dataPtr = std::make_shared<Data>();
        else
            dataPtr->t.clear();
    }

    template<class T>
    LazyContainer<T>::LazyContainer(const LazyContainer& other)
    {
        // Exception safety is guaranteed by operator=
        *this = other;
    }

    template<class T>
    LazyContainer<T>::LazyContainer(LazyContainer&& other)
        : LazyContainer()
    {
        // Exception safety is guaranteed by operator=
        *this = std::move(other);
    }

    template<class T>
    LazyContainer<T>& LazyContainer<T>::operator=(const LazyContainer& other)
    {   

        if(other.dataPtr->issuedMutableReference)
        {
            // If we need to make a copy of underlying data 
            // we first create new data then if everything goes ok we noexcept assign it

            std::shared_ptr<Data> newDataPtr = std::make_shared<Data>();
            newDataPtr->t = other.dataPtr->t;
            dataPtr = std::move(newDataPtr);
            dataPtr->issuedMutableReference = false;
        }
        else
        {
            // Copying std::shared_ptr is exception safe
            dataPtr = other.dataPtr;
        }

        return *this;
    }

    template<class T>
    LazyContainer<T>& LazyContainer<T>::operator=(LazyContainer&& other)
    {
        // std::swap of shared_ptr is noexcept
         static_assert(noexcept( std::shared_ptr<Data>().swap(std::declval<std::shared_ptr<Data>&>()) ));
        dataPtr.swap(other.dataPtr);
        return *this;
    }
}

template<class K, class V, class H>
insertion_ordered_map<K, V, H>& insertion_ordered_map<K, V, H>::operator=(insertion_ordered_map other) {
    container = std::move(other.container);
    return *this;
}

template <class K, class V, class H>
bool insertion_ordered_map<K, V, H>::insert(const K& key, const V& value) {

    bool result;
    container.doMutable([&](backend_map& theMap) {
        result = theMap.insert(key, value);
    });
    return result;
}

template <class K, class V, class H>
V &insertion_ordered_map<K, V, H>::at(K const& k) {

    V* result;
    container.doMutableAndSaveReference([&](backend_map& theMap) {
        result = &theMap.at(k);
    });
    return *result;
}

template <class K, class V, class H>
V const&insertion_ordered_map<K, V, H>::at(K const& k) const {

    return container.getConst().at(k);
}

template <class K, class V, class H>
V& insertion_ordered_map<K, V, H>::operator[](K const& k) {
    
    V* result;
    container.doMutableAndSaveReference([&](backend_map& theMap) {
        result = &theMap[k];
    });
    return *result;
}

template <class K, class V, class H>
bool insertion_ordered_map<K, V, H>::contains(const K& key) const noexcept(noexcept(backend_map().contains( std::declval<K>() ))) {

    return container.getConst().contains(key);
}

template <class K, class V, class H>
typename insertion_ordered_map<K, V, H>::iterator insertion_ordered_map<K, V, H>::begin() const noexcept {

    return container.getConst().begin();
}

template <class K, class V, class H>
typename insertion_ordered_map<K, V, H>::iterator insertion_ordered_map<K, V, H>::end() const noexcept {

    return container.getConst().end();
}

template <class K, class V, class H>
void insertion_ordered_map<K, V, H>::clear() {

    container.clearInternal();
}

template <class K, class V, class H>
bool insertion_ordered_map<K, V, H>::empty() const noexcept {

    return container.getConst().empty();
}


template <class K, class V, class H>
std::size_t insertion_ordered_map<K, V, H>::size() const noexcept {

    return container.getConst().size();
}


template <class K, class V, class H>
void insertion_ordered_map<K, V, H>::erase(const K& key) {

    container.doMutable([&](backend_map& theMap) {
        theMap.erase(key);
    });    
}

template <class K, class V, class H>
void insertion_ordered_map<K, V, H>::merge(const insertion_ordered_map& other) {

    if (this == &other) {
        return;
    }

    const backend_map& otherMap = other.container.getConst();

    container.doMutable([&](backend_map& theMap) {
        theMap.merge(otherMap);
    });    
}


