#include "fibin.h"

int main() {
    using t1 = decltype(Fibin<int>::eval<Lit<Fib<0>>>);
    static_assert(!std::is_integral<t1>::value);
    using t2 = Fibin<t1>;
    using t3 = decltype(t2::eval<Lit<Fib<0>>>);
}
