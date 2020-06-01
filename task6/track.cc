#include <algorithm>
#include <cctype>
#include <charconv>
#include <iostream>

#include "playerexception.h"
#include "track.h"

bool Track::check(__attribute__((unused)) Element *element) const { return true; }

Song::Song(const File &file) : Track(), lyrics(file.getContent()), artist(), title() {
    try {
        artist = file["artist"];
        title = file["title"];
    } catch (std::out_of_range &exc) {
        throw CorruptedContentError();
    }
}

void Song::play() const {
    out << "Song [" << artist << ", " << title << "]: " << lyrics << "\n";
}

Movie::Movie(const File &file)
    : Track(), subtitles(decode(file.getContent())), title(), year() {
    try {
        title = file["title"];
        const auto &year_s = file["year"];
        const auto conv =
            std::from_chars(year_s.data(), year_s.data() + year_s.size(), year);
        if (conv.ec != std::errc()) {
            throw CorruptedContentError();
        }
    } catch (std::out_of_range &exc) {
        throw CorruptedContentError();
    }
}

void Movie::play() const {
    out << "Movie [" << title << ", " << year << "]: " << subtitles << "\n";
}

char Movie::rot13(char c) {
    char base = '\0';
    if (islower(c)) {
        base = 'a';
    } else if (isupper(c)) {
        base = 'A';
    } else {
        return c;
    }
    return static_cast<char>(static_cast<char>(c - base + 13) % 26) + base;
}

std::string Movie::decode(std::string_view encoded_subtitles) const {
    std::string out;
    std::transform(encoded_subtitles.begin(), encoded_subtitles.end(),
                   std::back_inserter(out), rot13);
    return out;
}
