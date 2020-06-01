#include "file.h"
#include "playerexception.h"

File::File(const std::string &content) : type(""), content(""), metadata() {
    static const std::string
        type_f          = "([^|]*)",
        metadata_f      = "([^:|]*):([^|]*)",
        media_content_f = "([a-zA-Z\\s0-9,.!?':;-]*)",
        all             = type_f + "(\\|" + metadata_f + ")*\\|" + media_content_f;
    static const std::regex line(all), metadata_clause(metadata_f);
    std::smatch matching;
    if (!std::regex_match(content, matching, line)) {
        throw FileFormatError();
    }
    this->type = matching[1];
    this->content = matching[matching.size() - 1];

    auto metadata_begin =
        std::sregex_iterator(content.begin(), content.end(), metadata_clause);
    auto metadata_end = std::sregex_iterator();
    for (std::sregex_iterator i = metadata_begin; i != metadata_end; ++i) {
        std::smatch m = *i;
        const std::string &key = m[1], value = m[2];
        this->metadata[key] = value;
    }
}

const std::string &File::getContent() const { return content; }

const std::string &File::getType() const { return type; }

const std::string &File::operator[](const std::string &key) const {
    return metadata.at(key);
}
