#ifndef JNP6_PLAYEREXCEPTION_H__
#define JNP6_PLAYEREXCEPTION_H__

#include <iostream>

class PlayerException : std::exception {
  public:
    const char *what() const noexcept override { return "Player Exception"; }
};

class EmptyError : public PlayerException {
    const char *what() const noexcept override { return "Playlist is empty"; }
};

class PositionError : public PlayerException {
    const char *what() const noexcept override { return "Invalid position"; }
};

class LoopError : public PlayerException {
    const char *what() const noexcept override { return "Loop in playlist"; }
};

class FileFormatError : public PlayerException {
    const char *what() const noexcept override { return "corrupt file"; }
};

class IncorrectTypeError : public PlayerException {
    const char *what() const noexcept override { return "unsupported type"; }
};

class CorruptedContentError : public PlayerException {
    const char *what() const noexcept override { return "corrupt content"; }
};

#endif /* PLAYEREXCEPTION_H */
