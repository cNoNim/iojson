#include <cmath>
#include <cstddef>
#include <iomanip>
#include <ostream>
#include <string>
#include <utility>

#if __cplusplus <= 199711L
#define constexpr
namespace json {

template <bool, typename I, typename> struct conditional { typedef I type;};
template <typename I, typename T> struct conditional<false, I, T> { typedef T type; };
template <bool, typename T = void> struct enable_if {};
template <typename T> struct enable_if<true, T> { typedef T type; };
template <typename> struct is_floating_point { static const bool value = false; };
template <> struct is_floating_point<float> { static const bool value = true; };
template <> struct is_floating_point<double> { static const bool value = true; };
template <> struct is_floating_point<long double> { static const bool value = true; };
template <typename> struct is_integral { static const bool value = false; };
template <> struct is_integral<bool> { static const bool value = true; };
template <> struct is_integral<char> { static const bool value = true; };
template <> struct is_integral<signed char> { static const bool value = true; };
template <> struct is_integral<unsigned char> { static const bool value = true; };
template <> struct is_integral<wchar_t> { static const bool value = true; };
template <> struct is_integral<short> { static const bool value = true; };
template <> struct is_integral<unsigned short> { static const bool value = true; };
template <> struct is_integral<int> { static const bool value = true; };
template <> struct is_integral<unsigned int> { static const bool value = true; };
template <> struct is_integral<long> { static const bool value = true; };
template <> struct is_integral<unsigned long> { static const bool value = true; };
template <> struct is_integral<long long> { static const bool value = true; };
template <> struct is_integral<unsigned long long> { static const bool value = true; };
template <typename, typename> struct is_same { static const bool value = false; };
template <typename T> struct is_same<T, T> { static const bool value = true; };
}
#else
#include <type_traits>
namespace json {
using std::conditional;
using std::enable_if;
using std::is_floating_point;
using std::is_integral;
using std::is_same;
}
#endif

namespace json {

template<typename T, typename Enabled = void>
struct value_type;

#if __cplusplus > 199711L
inline namespace _v1 {
#endif

enum null_type { null = 0 };
struct quoting_tag { constexpr quoting_tag() {} } constexpr quote;
struct value_tag   { constexpr value_tag() {} }   constexpr value;
struct array_tag   { constexpr array_tag() {} }   constexpr array;
struct object_tag  { constexpr object_tag() {} }  constexpr object;
struct close_tag   { constexpr close_tag() {} }   constexpr close;

template<typename OStream, typename Parent> class __value_proxy;
template<typename OStream, typename Parent> class __array_proxy;
template<typename OStream, typename Parent> class __object_proxy;

template<typename OStream>
class __quoting_proxy {
  typedef __quoting_proxy this_type;
  OStream & os;

public:
  typedef OStream                              stream_type;
  typedef typename OStream::char_type          char_type;
  typedef typename OStream::traits_type        traits_type;
  typedef std::basic_string<char_type>         string_type;
  typedef typename string_type::const_iterator const_iterator;

  explicit
  __quoting_proxy(stream_type & os) : os(os) { }

  stream_type & get_stream() const { return os; }

  friend stream_type &
  operator<<(this_type const & proxy, string_type const & str) {
    using namespace std;
    stream_type & os = proxy.get_stream();
    os << '"';
    std::ios_base::fmtflags flags = os.flags(ios::hex | ios::right);
    char_type fill = os.fill('0');
    string_type const & __range = str;
    for(const_iterator __begin = __range.begin(),
        __end = __range.end();
      __begin != __end; ++__begin) {
      char_type c = *__begin;
      switch(c) {
      case '\b': os.put('\\'); os.put('b'); continue;
      case '\t': os.put('\\'); os.put('t'); continue;
      case '\n': os.put('\\'); os.put('n'); continue;
      case '\f': os.put('\\'); os.put('f'); continue;
      case '\r': os.put('\\'); os.put('r'); continue;
      case '"': case '\\': os.put('\\');
      default:
        if((0 < c && c < 31) || c == 127)
          os << "\\u" << setw(4) << traits_type::to_int_type(c);
        else os.put(c);
      };
    }
    os.fill(fill);
    os.flags(flags);
    os << '"';
    return os;
  }

  template<typename OS>
  friend __quoting_proxy<OS>
  operator<<(OS & os, quoting_tag const &);
};

template<typename OStream, typename Parent>
class __base_value_proxy {
  typedef __base_value_proxy this_type;

public:
  typedef OStream                       stream_type;
  typedef Parent                        parent_type;
  typedef typename OStream::char_type   char_type;
  typedef typename OStream::traits_type traits_type;

protected:
  __base_value_proxy(stream_type & os, parent_type const & parent)
    : os(os), parent(parent) {}

public:
  stream_type & get_stream() const { return os; }
  parent_type const & get_parent() const { return parent; }

protected:
  stream_type & os;
  parent_type const & parent;
};

template<typename OStream, typename Parent>
class __base_value_proxy_spec
  : public __base_value_proxy<OStream, Parent> {
  typedef  __base_value_proxy_spec             this_type;
  typedef  __base_value_proxy<OStream, Parent> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;
  typedef typename base_type::traits_type traits_type;

protected:
  __base_value_proxy_spec(stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

public:
  using base_type::get_stream;
  using base_type::get_parent;
};

template<typename OStream>
class __base_value_proxy_spec<OStream, OStream>
  : public __base_value_proxy<OStream, OStream> {
  typedef __base_value_proxy_spec              this_type;
  typedef __base_value_proxy<OStream, OStream> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;
  typedef typename base_type::traits_type traits_type;

protected:
  __base_value_proxy_spec(stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

public:
  using base_type::get_stream;
  parent_type & get_parent() const { return get_stream(); }
};

template<typename OStream, typename Parent,
  typename OStream::char_type oc, typename OStream::char_type dc, typename OStream::char_type cc>
class __base_collection_proxy
  : __base_value_proxy_spec<OStream, Parent> {
  typedef __base_collection_proxy                   this_type;
  typedef __base_value_proxy_spec<OStream, Parent> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;
  typedef typename base_type::traits_type traits_type;

protected:
  __base_collection_proxy(stream_type & os, parent_type const & parent)
    : base_type(os, parent), state(Empty) {}

  bool open() const {
    if (!is_open()) return false;
    stream_type & os = get_stream();
    os << (state == Empty ? oc : dc);
    state = Opened;
    return true;
  }

  void close() const {
    if (!is_open()) return;
    stream_type & os = get_stream();
    if (state == Empty) os << oc;
    state = Closed;
    os << cc;
  }

public:
  bool is_open() const { return state != Closed; }
  using base_type::get_stream;
  using base_type::get_parent;


protected:
  enum { Empty, Opened, Closed } mutable state;
};

template<typename OStream, typename Parent = OStream>
class __value_proxy
  : public __base_value_proxy_spec<OStream, Parent> {
  template<typename T, typename U> friend class __value_proxy;
  template<typename T, typename U> friend class __array_proxy;
  template<typename T, typename U> friend class __object_proxy;
  typedef __value_proxy                            this_type;
  typedef __base_value_proxy_spec<OStream, Parent> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;
  typedef typename base_type::traits_type traits_type;
  typedef typename conditional<is_same<stream_type, parent_type>::value,
    stream_type, parent_type const >::type auto_parent_type;

  explicit
  __value_proxy(OStream & os)
    : base_type(os, os) {}

  __value_proxy(OStream & os, Parent const & parent)
    : base_type(os, parent) {}

  using base_type::get_stream;
  using base_type::get_parent;

  template<typename T>
  friend auto_parent_type &
  operator<<(this_type const & proxy, T const & v) {
    value_type<T>()(proxy << value, v);
    return proxy.get_parent();
  }

  friend this_type
  operator<<(this_type const & proxy, value_tag const &) {
    return proxy;
  }

  friend __object_proxy<stream_type, parent_type>
  operator<<(this_type const & proxy, object_tag const &) {
    return __object_proxy<stream_type, parent_type>(proxy.get_stream(), proxy.get_parent());
  }

  friend __array_proxy<stream_type, parent_type>
  operator<<(this_type const & proxy, array_tag const &) {
    return __array_proxy<stream_type, parent_type>(proxy.get_stream(), proxy.get_parent());
  }
};

template<typename OStream, typename Parent = OStream>
class __array_proxy
  : public __base_collection_proxy<OStream, Parent, '[', ',', ']'> {
  template<typename T, typename U> friend class __value_proxy;
  template<typename T, typename U> friend class __array_proxy;
  template<typename T, typename U> friend class __object_proxy;
  typedef __array_proxy                                           this_type;
  typedef __base_collection_proxy<OStream, Parent, '[', ',', ']'> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;
  typedef typename base_type::traits_type traits_type;
  typedef typename conditional<is_same<stream_type, parent_type>::value,
    stream_type, parent_type const >::type auto_parent_type;

  explicit
  __array_proxy(stream_type & os)
    : base_type(os, os) {}

  __array_proxy(stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

  using base_type::get_stream;
  using base_type::get_parent;

  template<typename T>
  friend this_type const &
  operator<<(this_type const & proxy, T const & v) {
    value_type<T>()(proxy << value, v);
    return proxy;
  }

  friend __value_proxy<stream_type, this_type>
  operator<<(this_type const & proxy, value_tag const &) {
    proxy.open();
    return __value_proxy<stream_type, this_type>(proxy.get_stream(), proxy);
  }

  friend __array_proxy<stream_type, this_type>
  operator<<(this_type const & proxy, array_tag const &) {
    proxy.open();
    return __array_proxy<stream_type, this_type>(proxy.get_stream(), proxy);
  }

  friend __object_proxy<stream_type, this_type>
  operator<<(this_type const & proxy, object_tag const &) {
    proxy.open();
    return __object_proxy<stream_type, this_type>(proxy.get_stream(), proxy);
  }

  friend auto_parent_type &
  operator<<(this_type const & proxy, close_tag const &) {
    proxy.close();
    return proxy.get_parent();
  }

  template<typename OS>
  friend __array_proxy<OS, OS>
  operator<<(OS & os, object_tag const &);
};

template<typename OStream, typename Parent = OStream>
class __object_proxy
  : public __base_collection_proxy<OStream, Parent, '{', ',', '}'> {
  template<typename T, typename U> friend class __value_proxy;
  template<typename T, typename U> friend class __array_proxy;
  template<typename T, typename U> friend class __object_proxy;
  typedef __object_proxy                                          this_type;
  typedef __base_collection_proxy<OStream, Parent, '{', ',', '}'> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;
  typedef typename base_type::traits_type traits_type;
  typedef typename conditional<is_same<stream_type, parent_type>::value,
    stream_type, parent_type const >::type auto_parent_type;

  explicit
  __object_proxy(stream_type & os)
    : base_type(os, os) {}

  __object_proxy(stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

  using base_type::get_stream;
  using base_type::get_parent;

  friend __value_proxy<stream_type, this_type>
  operator<<(this_type const & proxy, std::basic_string<char_type> const & str) {
    if (proxy.open()) proxy.get_stream() << quote << str << ':';
    return __value_proxy<stream_type, this_type>(proxy.get_stream(), proxy);
  }

  friend auto_parent_type &
  operator<<(this_type const & proxy, close_tag const &) {
    proxy.close();
    return proxy.get_parent();
  }

  template<typename OS>
  friend __object_proxy<OS, OS>
  operator<<(OS & os, object_tag const &);
};

template<typename OStream>
__quoting_proxy<OStream>
operator<<(OStream & os, quoting_tag const &) {
  return __quoting_proxy<OStream>(os);
}

template<typename OStream>
__value_proxy<OStream>
operator<<(OStream & os, value_tag const &) {
  return __value_proxy<OStream>(os);
}

template<typename OStream>
__array_proxy<OStream>
operator<<(OStream & os, array_tag const &) {
  return __array_proxy<OStream>(os);
}

template<typename OStream>
__object_proxy<OStream>
operator<<(OStream & os, object_tag const &) {
  return __object_proxy<OStream>(os);
}

#if __cplusplus > 199711L
}
#endif

template<>
struct value_type<null_type> {
  template<typename Json>
  void
  operator()(Json const & json, null_type) const {
    json.get_stream() << "null";
  }
};

#if __cplusplus > 199711L
template<>
struct value_type<std::nullptr_t> : public value_type<null_type> { };
#endif

template<>
struct value_type<bool> {
  template<typename Json>
  void
  operator()(Json const & json, bool value) const { json.get_stream() << (value ? "true" : "false"); }
};

template<typename Integral>
struct value_type<Integral,
  typename enable_if<is_integral<Integral>::value
    && !is_same<Integral, bool>::value>::type> {
  template<typename Json>
  void
  operator()(Json const & json, Integral value) const { json.get_stream() << +value; }
};

template<typename FloatingPoint>
struct value_type<FloatingPoint,
  typename enable_if<is_floating_point<FloatingPoint>::value>::type> {
  template<typename Json>
  void
  operator()(Json const & json, FloatingPoint value) const {
    if (std::isfinite(value)) json.get_stream() << value;
    else json::value_type<null_type>()(json, null);
  }
};

template<typename Char>
struct value_type<std::basic_string<Char> > {
  template<typename Json>
  typename enable_if<is_same<typename Json::char_type, Char>::value>::type
  operator()(Json const & json, std::basic_string<Char> const & value) const {
    json.get_stream() << quote << value;
  }
};

template<typename Char>
struct value_type<Char *,
  typename std::char_traits<Char>::char_type> {
  template<typename Json>
  typename enable_if<is_same<typename Json::char_type, Char>::value>::type
  operator()(Json const & json, Char const * value) const {
    json.get_stream() << quote << value;
  }
};

template<typename Char>
struct value_type<Char const *> : public value_type<Char *, Char> {};
template<typename Char, size_t N>
struct value_type<Char [N]> : public value_type<Char *, Char> {};
template<typename Char, size_t N>
struct value_type<Char const [N]> : public value_type<Char *, Char> {};

}
