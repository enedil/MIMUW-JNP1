#include "fibin.h"

template<typename Fn, typename ...Ts>
struct InvokeVariadic {};

template<typename Fn, typename T, typename ...Ts>
struct InvokeVariadic<Fn, T, Ts...> {
    using _ = typename InvokeVariadic<Invoke<Fn, T>, Ts...>::_;
};

template<typename Fn>
struct InvokeVariadic<Fn> {
    using _ = Fn;
};

using Ycombinator =
Lambda<
    Var("f"),
    Invoke<
        Lambda<
            Var("x"),
            Invoke<
                Ref<Var("x")>,
                Ref<Var("x")>
            >
        >,
        Lambda<
            Var("x"),
            Invoke<
                Ref<Var("f")>,
                Lambda<
                    Var("args"),
                    Invoke<
                        Invoke<
                            Ref<Var("x")>,
                            Ref<Var("x")>
                        >,
                        Ref<Var("args")>>>>>>>;

template<typename F, typename ...Args>
using Exec = typename InvokeVariadic<Ycombinator, F, Args...>::_;


using InfiniteLoop =
Lambda<
    Var("loop"),
    Lambda<
        Var("x"),
        Invoke<
            Ref<Var("loop")>,
            Ref<Var("x")>>>>;

using ID =
Lambda<
    Var("id"),
    Lambda<
        Var("x"),
        Ref<Var("x")>>>;

using F2 = Lit<Fib<10>>;
using F3 = Lit<Fib<11>>;

template<typename Fun>
using Example = If<Lit<False>, Exec<Fun, F2>, F3>;

using L = Lit<Fib<Var("4")>>;

int main() {
    static_assert(Fibin<int>::eval<Example<ID>>() == 89);
    static_assert(Fibin<int>::eval<Example<InfiniteLoop>>() == 89);
}
