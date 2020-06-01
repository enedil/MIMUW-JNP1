#ifndef JNP6_TRACK_H__
#define JNP6_TRACK_H__

#include "element.h"
#include "file.h"
#include <memory>
#include <string>
#include <unordered_map>

class Track : public Element {
  public:
    Track() = default;
    virtual ~Track() override = default;
    bool check(Element *) const override;
};

class Song : public Track {
  public:
    Song(const File &);
    void play() const override;

  private:
    const std::string lyrics;
    std::string artist, title;
};

class Movie : public Track {
  public:
    Movie(const File &);
    void play() const override;

  private:
    const std::string subtitles;
    std::string decode(std::string_view) const;
    static char rot13(char c);
    std::string title;
    int year;
};

#endif /* TRACK_H */
