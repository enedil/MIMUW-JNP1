#pragma once

#include <cstdint>
#include <functional>

#include "color.h"
#include "coordinate.h"
#include "functional.h"

using Fraction = double;

template <typename T> using Base_image = std::function<T(Point const &)>;

using Region = Base_image<bool>;
using Image = Base_image<Color>;
using Blend = Base_image<Fraction>;

namespace Detail {
inline Coordinate fst(Point const &p) { return p.first; }
inline Coordinate snd(Point const &p) { return p.second; }
inline bool is_polar(Point const &p) { return p.is_polar; }
inline Point make_point(Coordinate first, Coordinate second, bool is_polar) {
    return Point{first, second, is_polar};
}
inline auto conditional(auto const& this_way, auto const& that_way) {
    return [this_way, that_way](bool c) { return c ? this_way : that_way; };
}
inline Point ensure_cartesian(Point const &p) {
    return Detail::is_polar(p) ? from_polar(p) : p;
}
inline Point ensure_polar(Point const &p) {
    return Detail::is_polar(p) ? p : to_polar(p);
}
inline auto div_by(double d) {
    return [d](auto x) { return x / d; };
}
} // namespace Detail

template <typename T> Base_image<T> constant(T const &t) {
    return [t](__attribute__((unused)) Point const &p) { return t; };
}

template <typename T> Base_image<T> rotate(Base_image<T> const &image, double phi) {
    return compose(to_polar,
                   lift(Detail::make_point, Detail::fst,
                        compose(Detail::snd, [phi](auto x) { return x - phi; }),
                        Detail::is_polar),
                   image);
}

template <typename T> Base_image<T> translate(Base_image<T> const &image, Vector v) {
    auto difference = [](auto x, auto y) { return x - y; };
    auto first = lift(difference, Detail::fst,
                      [v](__attribute__((unused)) auto x) { return v.first; });
    auto second = lift(difference, Detail::snd,
                       [v](__attribute__((unused)) auto x) { return v.second; });
    return compose(lift(Detail::make_point, first, second, Detail::is_polar), image);
}

template <typename T> Base_image<T> scale(Base_image<T> const &image, double s) {
    return compose(lift(Detail::make_point, compose(Detail::fst, Detail::div_by(s)),
                        compose(Detail::snd, Detail::div_by(s)), Detail::is_polar),
                   image);
}

template <typename T>
Base_image<T> circle(Point q, double r, T const &inner, T const &outer) {
    auto inside = [q, r](auto p) { return distance(p, q) <= r; };
    return compose(inside, Detail::conditional(inner, outer));
}

template <typename T>
Base_image<T> checker(double d, T const &this_way, T const &that_way) {
    auto floor_div_by_d = compose(Detail::div_by(d), floor);
    auto what_color = [](auto a, auto b) {
        return static_cast<intmax_t>(a + b) % 2 == 0;
    };
    auto lifted = lift(what_color, compose(Detail::fst, floor_div_by_d),
                       compose(Detail::snd, floor_div_by_d));
    return compose(lifted, Detail::conditional(this_way, that_way));
}

template <typename T>
Base_image<T> rings(Point q, double r, T const &this_way, T const &that_way) {
    auto floor_div_by_r = compose(Detail::div_by(r), floor);
    auto dist = [q](auto p) { return distance(p, q); };
    auto is_even = [](auto n) { return static_cast<intmax_t>(n) % 2 == 0; };
    return compose(dist, floor_div_by_r, is_even,
                   Detail::conditional(this_way, that_way));
}

template <typename T>
Base_image<T> polar_checker(double d, int n, T const &this_way, T const &that_way) {

    auto sc = lift(
        Detail::make_point, Detail::fst,
        compose(Detail::snd, [n, d](double theta) { return d * n * theta / (M_PI * 2); }),
        Detail::is_polar);
    return compose(Detail::ensure_polar, sc, checker(d, this_way, that_way));
}

template <typename T>
Base_image<T> vertical_stripe(double r, T const &this_way, T const &that_way) {
    auto inside = [r](double x) { return 2 * fabs(x) <= r; };
    return compose(Detail::ensure_cartesian, Detail::fst, inside,
                   Detail::conditional(this_way, that_way));
}

Image cond(Region const& region, Image const& this_way, Image const& that_way);
Image lerp(Blend const& blend, Image const& this_way, Image const& that_way);
Image darken(Image const& image, Blend const& blend);
Image lighten(Image const& image, Blend const& blend);
