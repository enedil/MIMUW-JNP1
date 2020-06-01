#pragma once

#include <functional>

inline namespace {
auto compose() {
    return [](auto x) { return std::forward<decltype(x)>(x); };
}

auto compose(auto const& g, auto const&... fns) {
    auto inner_compose = compose(fns...);
    return [g, inner_compose](auto x) { return inner_compose(g(std::forward<decltype(x)>(x))); };
}

auto lift(auto const& h) {
    return [h](__attribute__((unused)) auto const& x) { return h(); };
}

auto lift(auto const& h, auto const& f, auto const&... fns) {
    return [h, f, fns...](auto x) {
        auto g = std::bind_front(h, f(x));
        return lift(g, fns...)(std::forward<decltype(x)>(x));
    };
}
}
