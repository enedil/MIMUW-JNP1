#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "player.h"
#include "playerexception.h"

Player::TrackFactory::TrackFactory() : ctors() {
    ctors["audio"] = maker<Song>;
    ctors["video"] = maker<Movie>;
}

std::shared_ptr<Track> Player::TrackFactory::make(const File& file) {
    return ctors.at(file.getType())(file);
}

std::shared_ptr<Playlist> Player::createPlaylist(std::string name) {
	return std::make_shared<Playlist>(Playlist(name));
}

std::shared_ptr<Track> Player::openFile(const File &file) {
    try {
        return trackFactory.make(file);
    } catch (std::out_of_range &exc) {
        throw IncorrectTypeError();
    }
}
