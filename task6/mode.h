#ifndef JNP6_MODE_H__
#define JNP6_MODE_H__

#include "element.h"
#include <ctime>
#include <memory>
#include <random>
#include <vector>

class Mode {
  public:
    virtual void playInit(const std::vector<std::shared_ptr<Element>> &l) = 0;
    virtual std::shared_ptr<Element> next() = 0;
    virtual ~Mode() = default;
};

class SequenceMode : public Mode {
  public:
    void playInit(const std::vector<std::shared_ptr<Element>> &) override;
    std::shared_ptr<Element> next() override;
    ~SequenceMode() override = default;

  private:
    std::vector<std::shared_ptr<Element>>::const_iterator now;
};

class ShuffleMode : public Mode {
  public:
    ShuffleMode(unsigned seed);
    void playInit(const std::vector<std::shared_ptr<Element>> &) override;
    std::shared_ptr<Element> next() override;
    ~ShuffleMode() override = default;

  private:
    std::vector<std::shared_ptr<Element>>::const_iterator now;
    std::default_random_engine generator;
    std::vector<std::shared_ptr<Element>> playOrder;
};

class OddEvenMode : public Mode {
  public:
    void playInit(const std::vector<std::shared_ptr<Element>> &) override;
    std::shared_ptr<Element> next() override;
    ~OddEvenMode() override = default;

  private:
    std::vector<std::shared_ptr<Element>>::const_iterator now;
    std::vector<std::shared_ptr<Element>>::const_iterator even;
    std::vector<std::shared_ptr<Element>>::const_iterator end;
};

std::shared_ptr<Mode> createSequenceMode();

std::shared_ptr<Mode> createShuffleMode(unsigned seed);

std::shared_ptr<Mode> createOddEvenMode();

#endif /* MODE_H */
