#ifndef TYPE_TRAITS_HH_INCLUDED_20181221
#define TYPE_TRAITS_HH_INCLUDED_20181221

#include "compiler.hh"

#if PUFFIN_HAS_CONSTEXPR
#define PUFFIN_DECLARE_COMPILE_TIME_INT(type, name, value) \
                static constexpr type name = value;
#else
#define PUFFIN_DECLARE_COMPILE_TIME_INT(type, name, value) \
                enum { name = value };
#endif

namespace puffin { namespace impl {

// __ enable_if ________________________________________________________________
template<bool Cond, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };


// __ type _____________________________________________________________________
// template <typename T> struct type  { typedef T type; };


// __ integral_constant ________________________________________________________
// integral_constant is basically a C++03 backport of C++14' integral_constant.
template <typename T, T v>
struct integral_constant {
        PUFFIN_DECLARE_COMPILE_TIME_INT(T, value, v)

        typedef T value_type;
        typedef integral_constant  type;

        PUFFIN_CONSTEXPR   operator T  () const PUFFIN_NOEXCEPT { return v; }
        PUFFIN_CONSTEXPR T operator () () const PUFFIN_NOEXCEPT { return v; }
};


// __ bool_constant ____________________________________________________________
#if PUFFIN_HAS_ALIAS_TEMPLATE
template <bool v> using bool_constant = integral_constant<bool, v>;
#else
template <bool v> struct bool_constant : integral_constant<bool, v> {};
#endif


// __ true_type/false_type _____________________________________________________
typedef bool_constant<true > true_type;
typedef bool_constant<false> false_type;


// __ is_same<T, U> ____________________________________________________________
template <typename T, typename U> struct is_same      : false_type {};
template <typename T>             struct is_same<T,T> : true_type {};


// __ conditional<Cond, TrueT, FalseT> _________________________________________
template<bool Cond, typename TrueT, typename FalseT>
struct conditional { typedef TrueT type; };

template<typename TrueT, typename FalseT>
struct conditional<false, TrueT, FalseT> { typedef FalseT type; };


// __ conjunction<> ____________________________________________________________
template <typename = void,
          typename = void,
          typename = void,
          typename = void,
          typename = void,
          typename = void,
          typename = void,
          typename = void,
          typename = void,
          typename = void>
struct conjunction : true_type {};

template <typename HeadT>
struct conjunction<HeadT> : HeadT {};

template <typename HeadT,
          typename Tail0>
struct conjunction<HeadT, Tail0> :
        conditional<
                bool(HeadT::value),
                conjunction<Tail0>,
                HeadT
        >::type
{};

template <typename HeadT,
          typename Tail0,
          typename Tail1>
struct conjunction<HeadT, Tail0, Tail1> :
        conditional<
                bool(HeadT::value),
                conjunction<Tail0, Tail1>,
                HeadT >::type
{};

template <typename HeadT,
          typename Tail0,
          typename Tail1,
          typename Tail2>
struct conjunction<HeadT, Tail0, Tail1, Tail2> :
        conditional<
                bool(HeadT::value),
                conjunction<Tail0, Tail1, Tail2>,
                HeadT
        >::type
{};

template <typename HeadT,
          typename Tail0,
          typename Tail1,
          typename Tail2,
          typename Tail3>
struct conjunction<HeadT, Tail0, Tail1, Tail2, Tail3> :
        conditional<
                bool(HeadT::value),
                conjunction<Tail0, Tail1, Tail2, Tail3>,
                HeadT
        >::type
{};

template <typename HeadT,
          typename Tail0,
          typename Tail1,
          typename Tail2,
          typename Tail3,
          typename Tail4>
struct conjunction<HeadT, Tail0, Tail1, Tail2, Tail3, Tail4> :
        conditional<
                bool(HeadT::value),
                conjunction<Tail0, Tail1, Tail2, Tail3, Tail4>,
                HeadT
        >::type
{};


// __ remove_cv / remove_const / remove_volatile _______________________________
template <typename T> struct remove_const          { typedef T type; };
template <typename T> struct remove_const<const T> { typedef T type; };

template <typename T> struct remove_volatile             { typedef T type; };
template <typename T> struct remove_volatile<volatile T> { typedef T type; };

template <typename T>
struct remove_cv {
private:
        typedef typename remove_const<T>::type const_removed_type;
public:
        typedef typename remove_volatile<const_removed_type>::type type;
};



// __ is_floating_point ________________________________________________________
template <typename T>
struct is_floating_point :
        integral_constant<
                bool,
                is_same<float,       typename remove_cv<T>::type>::value ||
                is_same<double,      typename remove_cv<T>::type>::value ||
                is_same<long double, typename remove_cv<T>::type>::value
        >
{};

// __ is_unsigned_integer ______________________________________________________
template <typename T>
struct is_unsigned_integer :
        integral_constant<
                bool,
                is_same<unsigned char,   typename remove_cv<T>::type>::value ||
                is_same<unsigned short,  typename remove_cv<T>::type>::value ||
                is_same<unsigned int,    typename remove_cv<T>::type>::value ||
                is_same<unsigned long,   typename remove_cv<T>::type>::value ||
                is_same<unsigned long long, typename remove_cv<T>::type>::value
        >
{};


} }

#endif //TYPE_TRAITS_HH_INCLUDED_20181221
