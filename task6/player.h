#ifndef JNP6_PLAYER_H__
#define JNP6_PLAYER_H__
#include "file.h"
#include "playlist.h"
#include "track.h"
#include <exception>
#include <iostream>
#include <memory>
#include <unordered_map>

class Player {
  public:
    std::shared_ptr<Playlist> createPlaylist(std::string);
    std::shared_ptr<Track> openFile(const File &file);
  private:
    class TrackFactory {
      public:
        TrackFactory();
        std::shared_ptr<Track> make(const File& file);

      private:
        std::unordered_map<std::string, std::shared_ptr<Track>(*)(const File&)> ctors;
        template<typename TrackT>
        static std::shared_ptr<Track> maker(const File &file) {
            return std::make_shared<TrackT>(file);
        }
    };
    TrackFactory trackFactory;
};

#endif /* PLAYER_H */
