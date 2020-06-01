#include "images.h"

Image cond(Region const& region, Image const& this_way, Image const& that_way) {
    return [region, this_way, that_way](Point const &p) {
        return region(p) ? this_way(p) : that_way(p);
    };
}

Image lerp(Blend const& blend, Image const& this_way, Image const& that_way) {
    return [blend, this_way, that_way](Point const &p) {
        return this_way(p).weighted_mean(that_way(p), blend(p));
    };
}

Image darken(Image const& image, Blend const& blend) {
    return lerp(blend, image, constant(Colors::black));
}

Image lighten(Image const& image, Blend const& blend) {
    return lerp(blend, image, constant(Colors::white));
}
