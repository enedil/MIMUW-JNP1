#include "playlist.h"
#include "playerexception.h"
#include <iostream>

Playlist::Playlist(std::string name)
    : playList(), name(name), mode(createSequenceMode()) {}

bool Playlist::check(Element *element) const {
    if (this == element)
        return false;
    for (auto el : playList)
        if (!el->check(element))
            return false;
    return true;
}

void Playlist::add(std::shared_ptr<Element> element) {
    if (!(element->check(this)))
        throw LoopError();
    playList.emplace_back(element);
}

void Playlist::add(std::shared_ptr<Element> element, size_t position) {
    if (position > playList.size())
        throw PositionError();
    if (!(element->check(this)))
        throw LoopError();
    playList.emplace(playList.begin() + position, element);
}

void Playlist::remove() {
    if (playList.empty())
        throw EmptyError();
    playList.pop_back();
}

void Playlist::remove(size_t position) {
    if (position >= playList.size())
        throw PositionError();
    playList.erase(playList.begin() + position);
}

void Playlist::setMode(std::shared_ptr<Mode> mode) { this->mode = mode; }

void Playlist::play() const {
    out << "Playlist [" << name << "]\n";
    mode->playInit(playList);
    for (size_t it = 0; it < playList.size(); ++it)
        mode->next()->play();
}
