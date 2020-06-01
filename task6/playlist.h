#ifndef JNP6_PLAYLIST_H__
#define JNP6_PLAYLIST_H__ 

#include "element.h"
#include "mode.h"
#include <iostream>
#include <memory>
#include <vector>

class Playlist : public Element {
  private:
    std::vector<std::shared_ptr<Element>> playList;
    std::string name;
    std::shared_ptr<Mode> mode;

  public:
    Playlist(std::string name);
    bool check(Element *element) const override;
    void add(std::shared_ptr<Element> element);
    void add(std::shared_ptr<Element> element, size_t position);
    void remove();
    void remove(size_t position);
    void setMode(std::shared_ptr<Mode> mode);
    void play() const override;
};

#endif /* PLAYLIST_H */
