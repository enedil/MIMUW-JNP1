#ifndef JNP6_ELEMENT_H__
#define JNP6_ELEMENT_H__ 

#include <iostream>
#include <memory>

class Element {
  public:
    virtual bool check(Element *) const = 0;
    virtual void play() const = 0;
    virtual ~Element() = default;

  protected:
    using OutputDevice = std::ostream;
    Element(OutputDevice &out = std::cout) : out(out) {}
    OutputDevice &out;
};

#endif /* ELEMENT_H */
