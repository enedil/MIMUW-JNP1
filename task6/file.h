#ifndef JNP6_FILE_H__
#define JNP6_FILE_H__
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>

class File {
  public:
    File(const std::string &);
    const std::string &getContent() const;
    const std::string &getType() const;
    const std::string &operator[](const std::string &) const;

  private:
    std::string type;
    std::string content;
    std::unordered_map<std::string, std::string> metadata;
};
#endif
