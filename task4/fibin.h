#ifndef FIBIN_H
#define FIBIN_H

#include <iostream>
#include <type_traits>

namespace internal {
constexpr bool isUpper_(int c) { return c >= 'A' and c <= 'Z'; }

constexpr int toLower_(int c) {
    if (isUpper_(c))
        return c | 32;
    return c;
}

template <typename ValueType, ValueType n> struct Number {
    static constexpr ValueType value = n;
};

template <uint64_t id, typename Body, typename Env> struct Function {};

struct Empty {};

template <uint64_t id, typename Value, typename Tail> struct List {};

template <uint64_t id, typename list> struct SearchList {};

template <uint64_t id, typename Value, typename Tail>
struct SearchList<id, List<id, Value, Tail>> {
    using result = Value;
};

template <uint64_t id, uint64_t other_id, typename Value, typename Tail>
struct SearchList<id, List<other_id, Value, Tail>> {
    using result = typename SearchList<id, Tail>::result;
};

template <typename Value> struct LiteralValue { using value = Value; };

} // namespace internal

template <typename Value> struct Lit { using value = Value; };

struct True {};
struct False {};

template <typename... Ts> struct Sum {};

template <typename T> struct Inc1 {};

template <typename T> struct Inc10 {};

template <typename Cond, typename IfTrue, typename IfFalse> struct If {};

template <typename Left, typename Right> struct Eq {};

template <uint64_t Param, typename Body> struct Lambda {};

template <typename Fun, typename Param> struct Invoke {};

template <uint64_t Param, typename Value, typename Body> struct Let {};

template <uint64_t Var> struct Ref {};

constexpr uint64_t Var(const char *name) {
    if (name == nullptr) {
        throw std::domain_error{"CPP.lang.NullPointerException"};
    }
    if (name[0] == '\0') {
        throw std::length_error{"CPP.lang.IndexOutOfBoundsException"};
    }
    uint64_t id = 0;
    size_t i = 0;
    while (name[i] != '\0') {
        if (i >= 6) {
            throw std::length_error{"CPP.lang.OutOfMemoryException"};
        }
        signed char c = internal::toLower_(name[i]);
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            throw std::invalid_argument{"CPP.lang.InvalidParameterException"};
        }
        id <<= 8;
        id += c;
        i++;
    }
    return id;
}

template <unsigned m> struct Fib {
    template <typename ValueType, unsigned n> struct Impl {
        static constexpr ValueType value =
            (Impl<ValueType, n - 1>::value) + (Impl<ValueType, n - 2>::value);
    };

    template <typename ValueType> struct Impl<ValueType, 0> {
        static constexpr ValueType value = 0;
    };

    template <typename ValueType> struct Impl<ValueType, 1> {
        static constexpr ValueType value = 1;
    };

    template <typename ValueType>
    using result =
        internal::LiteralValue<internal::Number<ValueType, Impl<ValueType, m>::value>>;
};

template <typename ValueType> class Fibin {
  public:
    template <typename Expr, typename V = ValueType,
              typename = typename std::enable_if<std::is_integral<V>::value>::type>
    static constexpr ValueType eval() {
        using LitT = ER<Expr, internal::Empty>;
        using NumT = typename LitT::value;
        return NumT::value;
    }

    template <typename Expr, typename V = ValueType,
              typename = typename std::enable_if<!std::is_integral<V>::value>::type>
    static constexpr void eval() {
        std::cout << "Fibin doesn't support: " << typeid(ValueType).name() << "\n";
    }

  private:
    template <typename Expr, typename Env> struct Eval {};

    template <typename Expr, typename Env> using ER = typename Eval<Expr, Env>::result;

    template <typename T, typename Env> struct Eval<Lit<T>, Env> {
        using result = internal::LiteralValue<T>;
    };

    template <typename T, typename Env> struct Eval<internal::LiteralValue<T>, Env> {
        using result = internal::LiteralValue<T>;
    };

    template <typename Env, unsigned n> struct Eval<Lit<Fib<n>>, Env> {
        using result = typename Fib<n>::template result<ValueType>;
    };

    template <typename Condition, typename IfTrue, typename IfFalse, typename Env>
    struct Eval<If<Condition, IfTrue, IfFalse>, Env> {
      private:
        using EvaledCondition = ER<Condition, Env>;
        using EvaledIf = If<EvaledCondition, IfTrue, IfFalse>;

      public:
        using result = ER<EvaledIf, Env>;
    };

    template <typename IfTrue, typename IfFalse, typename Env>
    struct Eval<If<internal::LiteralValue<True>, IfTrue, IfFalse>, Env> {
        using result = ER<IfTrue, Env>;
    };

    template <typename IfTrue, typename IfFalse, typename Env>
    struct Eval<If<internal::LiteralValue<False>, IfTrue, IfFalse>, Env> {
        using result = ER<IfFalse, Env>;
    };

    template <typename Left, typename Right, typename Env>
    struct Eval<Eq<Left, Right>, Env> {
      private:
        using EvaledLeft = ER<Left, Env>;
        using EvaledRight = ER<Right, Env>;

      public:
        using result =
            std::conditional_t<EvaledLeft::value::value == EvaledRight::value::value,
                               Lit<True>, Lit<False>>;
    };

    template <typename Env, typename T, typename... Ts> struct Eval<Sum<T, Ts...>, Env> {
      private:
        using EvaledLit = ER<T, Env>;
        using EvaledNum = typename EvaledLit::value;
        static constexpr ValueType v = EvaledNum::value;

        using RestLit = ER<Sum<Ts...>, Env>;
        using RestNum = typename RestLit::value;
        static constexpr ValueType rest_v = RestNum::value;

      public:
        using result = internal::LiteralValue<internal::Number<ValueType, v + rest_v>>;
    };

    template <typename Env, typename T1, typename T2> struct Eval<Sum<T1, T2>, Env> {
      private:
        using EvaledLit1 = ER<T1, Env>;
        using EvaledNum1 = typename EvaledLit1::value;
        static constexpr ValueType t1 = EvaledNum1::value;

        using EvaledLit2 = ER<T2, Env>;
        using EvaledNum2 = typename EvaledLit2::value;
        static constexpr ValueType t2 = EvaledNum2::value;

      public:
        using result = internal::LiteralValue<internal::Number<ValueType, t1 + t2>>;
    };

    template <typename Env, typename T> struct Eval<Inc1<T>, Env> {
        using L1 = Lit<Fib<1>>;

      public:
        using result = ER<Sum<L1, T>, Env>;
    };

    template <typename Env, typename T> struct Eval<Inc10<T>, Env> {
      private:
        using L10 = Lit<Fib<10>>;

      public:
        using result = ER<Sum<L10, T>, Env>;
    };

    template <typename Env, uint64_t id> struct Eval<Ref<id>, Env> {
        using result = typename internal::SearchList<id, Env>::result;
    };

    template <typename Env, uint64_t id, typename Body>
    struct Eval<Lambda<id, Body>, Env> {
        using result = internal::LiteralValue<internal::Function<id, Body, Env>>;
    };

    template <typename Env, typename Value, typename Expr>
    struct Eval<Invoke<Expr, Value>, Env> {
      private:
        using Fun = ER<Expr, Env>;

      public:
        using result = ER<Invoke<Fun, Value>, Env>;
    };

    template <typename Env, uint64_t id, typename ParamValue, typename Body,
              typename FunctionEnv>
    struct Eval<Invoke<internal::LiteralValue<internal::Function<id, Body, FunctionEnv>>,
                       ParamValue>,
                Env> {
      private:
        using EvaledParamValue = ER<ParamValue, Env>;
        using UpdatedEnv = internal::List<id, EvaledParamValue, FunctionEnv>;

      public:
        using result = ER<Body, UpdatedEnv>;
    };

    template <typename Env, uint64_t id, typename Value, typename Body>
    struct Eval<Let<id, Value, Body>, Env> {
      private:
        using Fn = ER<Lambda<id, Body>, Env>;

      public:
        using result = ER<Invoke<Fn, Value>, Env>;
    };
};

#endif
