#include <algorithm>
#include <iostream>
#include <limits>
#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using std::begin;
using std::cerr;
using std::cout;
using std::end;
using std::get;
using std::istream;
using std::map;
using std::optional;
using std::ostream;
using std::pair;
using std::string;
using std::tuple;
using std::variant;
using std::vector;

namespace RE {
using std::regex;
regex nazwa_biletu("([a-zA-Z ]+) ");
regex zapytanie_o_bilety("^\\?"                    // znak zapytania
                         "( [a-zA-Z_\\^]+ \\d+)+ " // para: (przystanek, numer kursu)
                         "([a-zA-Z_\\^]+)$");      // ostatni przystanek
regex dodaj_bilet("^([a-zA-Z ]+)"                  // nazwa biletu
                  " "
                  "(\\d+\\.\\d{2})" // cena
                  " "
                  "([1-9]\\d*)$"); // czas w minutach
regex dodaj_kurs("^(\\d+)"         // numer kursu
                 "( [1-9]\\d?:\\d{2} [a-zA-Z_\\^]+)+$");
} // namespace RE

// wzięte z https://en.cppreference.com/w/cpp/utility/variant/visit
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

template <typename S, typename T> std::optional<T> wybierzZMapy(const map<S, T> &m, const S &klucz) {
    auto v = m.find(klucz);
    if (v == end(m)) {
        return {};
    }
    return std::make_optional(v->second);
}

namespace {
using punkt_w_czasie = pair<uint16_t, uint16_t>; // godzina, minuta
using przystanek = string;
using kurs = map<przystanek, punkt_w_czasie>; // przystanek -> czas
using numer_kursu = uint64_t;
using rozklad_kursow = map<numer_kursu, kurs>; // numer linii -> linia

using minuty = uint64_t;
using cena_grosze = uint64_t;
enum bilet_indeks { NAZWA, CENA, CZAS_WAZNOSCI };
using bilet = tuple<string, cena_grosze, minuty>;

using trzeba_czekac = przystanek; // gdzie trzeba czekać
using zestaw_biletow = vector<bilet>;
struct blad {};
struct nie_da_sie_kupic_biletu {};
using wynik_zapytania = variant<zestaw_biletow, trzeba_czekac, nie_da_sie_kupic_biletu, blad>;

ostream &operator<<(ostream &out, const bilet &wynik) {
    out << (get<NAZWA>(wynik));
    return out;
}

ostream &operator<<(ostream &out, __attribute__((unused)) const nie_da_sie_kupic_biletu &e) {
    out << ":-|\n";
    return out;
}

minuty ileMinutOdPolnocy(const punkt_w_czasie &p) {
    const int ileMinut = 60;
    return p.first * ileMinut + p.second;
}

void wypiszWynikZapytania(const wynik_zapytania &wynik, size_t &liczba_sprzedanych_biletow) {
    std::visit(overloaded{[&liczba_sprzedanych_biletow](const zestaw_biletow &zestaw) {
                              zestaw_biletow nowy;
                              std::copy_if(begin(zestaw), end(zestaw), std::back_inserter(nowy),
                                           [](const bilet &b) { return get<CENA>(b) != 0; });

                              liczba_sprzedanych_biletow += nowy.size();
                              cout << "! ";
                              if (nowy.size() > 0) {
                                  for (auto it = begin(nowy); next(it) != end(nowy); it++) {
                                      cout << *it << "; ";
                                  }
                                  cout << nowy.back();
                              }
                              cout << "\n";
                          },
                          [](const trzeba_czekac &t) { cout << ":-( " << t << "\n"; },
                          [](const nie_da_sie_kupic_biletu &e) { cout << e; },
                          [](__attribute__((unused)) const blad &b) {}},
               wynik);
}

void blednaLinia(const string &linia, size_t numer) { cerr << "Error in line " << numer << ": " << linia << "\n"; }

bool dodajBiletDoRozkladu(zestaw_biletow &zestaw, const string &nazwa, uint64_t zlote, uint64_t grosze, minuty m) {
    cena_grosze c = zlote * 100 + grosze;
    if (c == 0) {
        return false;
    }
    auto it = std::find_if(begin(zestaw), end(zestaw), [nazwa](auto b) { return get<NAZWA>(b) == nazwa; });
    if (it == end(zestaw)) {
        zestaw.emplace_back(nazwa, c, m);
        return true;
    }
    return false;
}

bool dodajKursDoRozkladu(rozklad_kursow &rozklad, numer_kursu numerKursu, const kurs &k) {
    auto it = rozklad.find(numerKursu);
    if (it != end(rozklad)) {
        return false;
    }
    rozklad[numerKursu] = k;
    return true;
}

variant<punkt_w_czasie, przystanek, blad> moznaOdjechacZPrzystanku(const kurs &k, const przystanek &startowy,
                                                                   const przystanek &koncowy,
                                                                   const punkt_w_czasie &godzinaOdjazdu) {
    auto itStartowy = k.find(startowy);
    auto itKoncowy = k.find(koncowy);
    if (itStartowy == end(k) or itKoncowy == end(k)) {
        return blad{};
    }
    if (itStartowy->second >= itKoncowy->second) {
        return blad{};
    }
    if (itStartowy->second < godzinaOdjazdu) {
        return blad{};
    }
    if (itStartowy->second != godzinaOdjazdu) {
        return startowy;
    }
    return itKoncowy->second;
}

variant<minuty, przystanek, blad> znajdzCzasTrasy(const rozklad_kursow &rozklad, const vector<przystanek> &przystanki,
                                                  const vector<numer_kursu> &numeryKursow) {
    if (przystanki.size() == 0) {
        return blad{};
    }
    if (przystanki.size() == 1) {
        return minuty{};
    }
    auto linia = wybierzZMapy(rozklad, numeryKursow.front());
    if (!linia.has_value()) {
        return blad{};
    }
    auto poczatkowaGodzina = wybierzZMapy(*linia, przystanki.front());
    if (!poczatkowaGodzina.has_value()) {
        return blad{};
    }
    punkt_w_czasie godzinaOdjazdu = poczatkowaGodzina.value();

    auto przystankiIt = begin(przystanki);
    auto kursyIt = begin(numeryKursow);
    for (; kursyIt != end(numeryKursow); ++przystankiIt, ++kursyIt) {
        auto linia = wybierzZMapy(rozklad, *kursyIt);
        if (!linia.has_value()) {
            return blad{};
        }
        auto mozna = moznaOdjechacZPrzystanku(*linia, *przystankiIt, *next(przystankiIt), godzinaOdjazdu);

        if (std::holds_alternative<punkt_w_czasie>(mozna)) {
            godzinaOdjazdu = *std::get_if<punkt_w_czasie>(&mozna);
        } else if (std::holds_alternative<przystanek>(mozna)) {
            return *std::get_if<przystanek>(&mozna);
        } else {
            return blad{};
        }
    }
    return ileMinutOdPolnocy(godzinaOdjazdu) - ileMinutOdPolnocy(poczatkowaGodzina.value());
}

zestaw_biletow znajdzNajtanszyZestawBiletow(const zestaw_biletow &zestaw, minuty czas) {
    cena_grosze najtansza_cena = std::numeric_limits<cena_grosze>::max();
    zestaw_biletow wynik{};
    for (const auto &i : zestaw) {
        for (const auto &j : zestaw) {
            for (const auto &k : zestaw) {
                if (get<CZAS_WAZNOSCI>(i) + get<CZAS_WAZNOSCI>(j) + get<CZAS_WAZNOSCI>(k) > czas) {
                    auto cena = get<CENA>(i) + get<CENA>(j) + get<CENA>(k);
                    if (cena <= najtansza_cena) {
                        najtansza_cena = cena;
                        wynik = zestaw_biletow{i, j, k};
                    }
                }
            }
        }
    }
    return wynik;
}

wynik_zapytania zapytaj(const rozklad_kursow &rozklad, const zestaw_biletow &zestaw,
                        const vector<przystanek> &przystanki, const vector<numer_kursu> &numeryKursow) {
    variant<minuty, przystanek, blad> czasPodrozy = znajdzCzasTrasy(rozklad, przystanki, numeryKursow);
    if (std::holds_alternative<minuty>(czasPodrozy)) {
        auto wynik = znajdzNajtanszyZestawBiletow(zestaw, *std::get_if<minuty>(&czasPodrozy));
        if (wynik.size() == 0) {
            return nie_da_sie_kupic_biletu{};
        }
        return wynik;
    } else if (std::holds_alternative<trzeba_czekac>(czasPodrozy)) {
        return *std::get_if<trzeba_czekac>(&czasPodrozy);
    }
    return blad{};
}

namespace Interfejs {
using std::istringstream;

bool dodajBilet(zestaw_biletow &zestaw, const string &linia) {
    std::smatch result;
    regex_search(linia, result, RE::nazwa_biletu);
    string nazwa;

    uint64_t zlote, grosze;
    minuty czas;

    nazwa = result.str(1);

    istringstream ss(result.suffix());

    ss >> zlote;
    ss.ignore(1); // kropka dziesiętna
    ss >> grosze;
    ss >> czas;
    if (zlote == std::numeric_limits<decltype(zlote)>::max()) {
        return false;
    }
    if (grosze == std::numeric_limits<decltype(grosze)>::max()) {
        return false;
    }
    if (czas == std::numeric_limits<decltype(czas)>::max()) {
        return false;
    }
    return dodajBiletDoRozkladu(zestaw, nazwa, zlote, grosze, czas);
}

bool zapytanieOBilety(const rozklad_kursow &rozklad, const zestaw_biletow &zestaw, const string &linia,
                      size_t &liczba_sprzedanych_biletow) {
    istringstream ss(linia);
    ss.ignore(1); // znak zapytania

    vector<przystanek> przystanki;
    vector<numer_kursu> numeryKursow;

    przystanek p;
    numer_kursu n;
    ss >> p;
    przystanki.emplace_back(p);

    while (ss >> n >> p) {
        numeryKursow.emplace_back(n);
        przystanki.emplace_back(p);
    }

    wynik_zapytania wynik = zapytaj(rozklad, zestaw, przystanki, numeryKursow);
    if (std::holds_alternative<blad>(wynik)) {
        return false;
    }
    wypiszWynikZapytania(wynik, liczba_sprzedanych_biletow);
    return true;
}

bool dodajPrzystanekDoKursu(kurs &k, const przystanek &przyst, const punkt_w_czasie &czas) {
    auto it = k.find(przyst);
    if (it != end(k)) {
        return false;
    }
    k[przyst] = czas;
    return true;
}

istream &operator>>(istream &in, punkt_w_czasie &p) {
    char c;
    in >> p.first >> c >> p.second;
    return in;
}

bool dodajKurs(rozklad_kursow &rozklad, const string &linia) {
    const punkt_w_czasie poczatek_pracy{5, 55};
    const punkt_w_czasie koniec_pracy{21, 21};

    istringstream ss(linia);
    numer_kursu numerKursu;
    kurs k;

    ss >> numerKursu;

    przystanek przyst;
    punkt_w_czasie czas, czas_poprzedni{};
    while (ss >> czas >> przyst) {
        if (czas.second >= 60)
            return false;

        if (czas < poczatek_pracy || czas > koniec_pracy)
            return false;

        if (czas_poprzedni >= czas)
            return false;

        if (!dodajPrzystanekDoKursu(k, przyst, czas))
            return false;

        czas_poprzedni = czas;
    }

    return dodajKursDoRozkladu(rozklad, numerKursu, k);
}
} // namespace Interfejs
} // namespace

int main() {
    zestaw_biletow dostepne_bilety{{}};
    rozklad_kursow rozklad;
    size_t liczba_sprzedanych_biletow = 0;

    string linia;
    size_t numer_linii = 0;
    while (getline(std::cin, linia)) {
        numer_linii++;

        bool sukces = false;

        if (regex_match(linia, RE::dodaj_bilet)) {
            sukces = Interfejs::dodajBilet(dostepne_bilety, linia);
        } else if (regex_match(linia, RE::zapytanie_o_bilety)) {
            sukces = Interfejs::zapytanieOBilety(rozklad, dostepne_bilety, linia, liczba_sprzedanych_biletow);
        } else if (regex_match(linia, RE::dodaj_kurs)) {
            sukces = Interfejs::dodajKurs(rozklad, linia);
        } else if (linia.length() == 0) {
            sukces = true;
        }

        if (!sukces) {
            blednaLinia(linia, numer_linii);
        }
    }
    std::cout << liczba_sprzedanych_biletow << std::endl;
}
