#ifndef __JSON_OSTREAM__
#define __JSON_OSTREAM__

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
} // namespace json
#else
#include <cstddef>
#include <type_traits>
namespace json {
using std::conditional;
using std::enable_if;
using std::is_floating_point;
using std::is_integral;
using std::is_same;
using std::nullptr_t;
} // namespace json
#endif

namespace json {

enum null_type { null = 0 };
template<typename T, typename Enabled = void>
struct value_type;

#if __cplusplus > 199711L
inline
#endif
namespace _v1 {

template<typename OStream, typename Parent> class __value_proxy;
template<typename OStream, typename Parent> class __array_proxy;
template<typename OStream, typename Parent> class __object_proxy;

struct value_tag  { constexpr value_tag() {} }  constexpr value;
struct array_tag  { constexpr array_tag() {} }  constexpr array;
struct object_tag { constexpr object_tag() {} } constexpr object;
struct close_tag  { constexpr close_tag() {} }  constexpr close;

class api_tag {
  api_tag() {}
  static api_tag const & get() { static api_tag tag; return tag; }

  template<typename OStream, typename Parent> friend class __base_value_proxy;

  template<typename OS> friend __object_proxy<OS, OS>
  operator<<(OS & os, object_tag const &);

  template<typename OS> friend __value_proxy<OS, OS>
  operator<<(OS & os, value_tag const &);

  template<typename OS> friend __array_proxy<OS, OS>
  operator<<(OS & os, array_tag const &);
};

template<typename OStream, typename Parent>
class __base_value_proxy {
  typedef __base_value_proxy this_type;

public:
  typedef OStream                       stream_type;
  typedef Parent                        parent_type;
  typedef typename OStream::char_type   char_type;

protected:
  __base_value_proxy(stream_type & os, parent_type const & parent)
    : os(os), parent(parent) {}

  stream_type & get_stream() const { return os; }
  parent_type const & get_parent() const { return parent; }
  api_tag const & get_tag() const { return api_tag::get(); }

private:
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

protected:
  __base_value_proxy_spec(stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

  using base_type::get_stream;
  using base_type::get_parent;
  using base_type::get_tag;
};

template<typename OStream>
class __base_value_proxy_spec<OStream, OStream>
  : public __base_value_proxy<OStream, OStream> {
  typedef __base_value_proxy_spec              this_type;
  typedef __base_value_proxy<OStream, OStream> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;

protected:
  __base_value_proxy_spec(stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

  using base_type::get_stream;
  parent_type & get_parent() const { return get_stream(); }
  using base_type::get_tag;
};

template<typename OStream, typename Parent,
  typename OStream::char_type oc, typename OStream::char_type dc, typename OStream::char_type cc>
class __base_collection_proxy
  : __base_value_proxy_spec<OStream, Parent> {
  typedef __base_collection_proxy                  this_type;
  typedef __base_value_proxy_spec<OStream, Parent> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;

protected:
  __base_collection_proxy(stream_type & os, parent_type const & parent)
    : base_type(os, parent), state(Empty) {}

  using base_type::get_stream;
  using base_type::get_parent;
  using base_type::get_tag;

  void open() const {
    if (!is_open()) return;
    get_stream() << (state == Empty ? oc : dc);
    state = Opened;
  }

  void close() const {
    if (!is_open()) return;
    if (state == Empty) get_stream() << oc;
    state = Closed;
    get_stream() << cc;
  }

private:
  bool is_open() const { return state != Closed; }
  enum { Empty, Opened, Closed } mutable state;
};

template<typename OStream, typename Parent = OStream>
class __value_proxy
  : public __base_value_proxy_spec<OStream, Parent> {
  typedef __value_proxy                            this_type;
  typedef __base_value_proxy_spec<OStream, Parent> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;
  typedef typename conditional<is_same<stream_type, parent_type>::value,
    stream_type &, parent_type const &>::type return_type;

  explicit
  __value_proxy(api_tag const &, stream_type & os)
    : base_type(os, os) {}

  __value_proxy(api_tag const &, stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

  template<typename T>
  return_type
  write(T const & v) const { get_stream() << v; return get_parent(); }

  template<typename Iterator>
  typename enable_if<is_same<typename Iterator::value_type, char_type>::value, return_type>::type
  write_quoted(Iterator begin, Iterator end) const {
    typedef typename Iterator::value_type char_type;
    static char_type hex[]  = {
      '0', '1', '2', '3',
      '4', '5', '6', '7',
      '8', '9', 'a', 'b',
      'c', 'd', 'e', 'f'
    };
    stream_type & os = get_stream();
    os.put('"');
    for(; begin != end; ++begin) {
      char_type c = *begin;
      switch(c) {
      case '\b': os.put('\\'); os.put('b'); continue;
      case '\t': os.put('\\'); os.put('t'); continue;
      case '\n': os.put('\\'); os.put('n'); continue;
      case '\f': os.put('\\'); os.put('f'); continue;
      case '\r': os.put('\\'); os.put('r'); continue;
      case '"': case '\\': os.put('\\');
      default:
        if((0 < c && c < 31) || c == 127) {
          os.put('\\'); os.put('u'); os.put('0'); os.put('0');
          os.put(hex[(c & 0xf0) >> 4]); os.put(hex[c & 0xf]);
        }
        else os.put(c);
      };
    }
    os.put('"');
    return get_parent();
  }

private:
  using base_type::get_stream;
  using base_type::get_parent;
  using base_type::get_tag;

  template<typename T>
  friend return_type
  operator<<(this_type const & proxy, T const & v) {
    return value_type<T>().template apply<return_type>(proxy, v);
  }

  friend this_type
  operator<<(this_type const & proxy, value_tag const &) {
    return proxy;
  }

  friend __object_proxy<stream_type, parent_type>
  operator<<(this_type const & proxy, object_tag const &) {
    return __object_proxy<stream_type, parent_type>(proxy.get_tag(), proxy.get_stream(), proxy.get_parent());
  }

  friend __array_proxy<stream_type, parent_type>
  operator<<(this_type const & proxy, array_tag const &) {
    return __array_proxy<stream_type, parent_type>(proxy.get_tag(), proxy.get_stream(), proxy.get_parent());
  }
};

template<typename OStream, typename Parent = OStream>
class __array_proxy
  : public __base_collection_proxy<OStream, Parent, '[', ',', ']'> {
  typedef __array_proxy                                           this_type;
  typedef __base_collection_proxy<OStream, Parent, '[', ',', ']'> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;
  typedef typename conditional<is_same<stream_type, parent_type>::value,
    stream_type &, parent_type const &>::type return_type;

  explicit
  __array_proxy(api_tag const &, stream_type & os)
    : base_type(os, os) {}

  __array_proxy(api_tag const &, stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

private:
  using base_type::get_stream;
  using base_type::get_parent;
  using base_type::get_tag;

  template<typename T>
  friend this_type const &
  operator<<(this_type const & proxy, T const & v) {
    return proxy << value << v;
  }

  friend __value_proxy<stream_type, this_type>
  operator<<(this_type const & proxy, value_tag const &) {
    proxy.open();
    return __value_proxy<stream_type, this_type>(proxy.get_tag(), proxy.get_stream(), proxy);
  }

  friend __array_proxy<stream_type, this_type>
  operator<<(this_type const & proxy, array_tag const &) {
    proxy.open();
    return __array_proxy<stream_type, this_type>(proxy.get_tag(), proxy.get_stream(), proxy);
  }

  friend __object_proxy<stream_type, this_type>
  operator<<(this_type const & proxy, object_tag const &) {
    proxy.open();
    return __object_proxy<stream_type, this_type>(proxy.get_tag(), proxy.get_stream(), proxy);
  }

  friend return_type
  operator<<(this_type const & proxy, close_tag const &) {
    proxy.close();
    return proxy.get_parent();
  }
};

template<typename OStream, typename Parent = OStream>
class __object_proxy
  : public __base_collection_proxy<OStream, Parent, '{', ',', '}'> {
  typedef __object_proxy                                          this_type;
  typedef __base_collection_proxy<OStream, Parent, '{', ',', '}'> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;
  typedef typename conditional<is_same<stream_type, parent_type>::value,
    stream_type &, parent_type const &>::type return_type;

  explicit
  __object_proxy(api_tag const &, stream_type & os)
    : base_type(os, os) {}

  __object_proxy(api_tag const &, stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

private:
  using base_type::get_stream;
  using base_type::get_parent;
  using base_type::get_tag;

  template<typename String>
  friend typename enable_if<value_type<String>::is_string,
    __value_proxy<stream_type, this_type> >::type
  operator<<(this_type const & proxy, String const & str) {
    proxy.open();
    stream_type & os = (__value_proxy<stream_type, this_type>(proxy.get_tag(), proxy.get_stream(), proxy) << str).get_stream();
    os << ':';
    return __value_proxy<stream_type, this_type>(proxy.get_tag(), os, proxy);
  }

  friend return_type
  operator<<(this_type const & proxy, close_tag const &) {
    proxy.close();
    return proxy.get_parent();
  }
};

template<typename OStream>
__value_proxy<OStream>
operator<<(OStream & os, value_tag const &) { return __value_proxy<OStream>(api_tag::get(), os); }

template<typename OStream>
__array_proxy<OStream>
operator<<(OStream & os, array_tag const &) { return __array_proxy<OStream>(api_tag::get(), os); }

template<typename OStream>
__object_proxy<OStream>
operator<<(OStream & os, object_tag const &) { return __object_proxy<OStream>(api_tag::get(), os); }

} // namespace json::_v1
#if __cplusplus <= 199711L
using _v1::value;
using _v1::array;
using _v1::object;
using _v1::close;
#endif
} // namespace json

#include <cmath>

namespace json {
template<>
struct value_type<null_type> {
  template<typename Return, typename Json>
  Return
  apply(Json const & json, null_type) const { return json.write("null"); }
};

#if __cplusplus > 199711L
template<>
struct value_type<std::nullptr_t> : public value_type<null_type> { };
#endif

template<>
struct value_type<bool> {
  template<typename Return, typename Json>
  Return
  apply(Json const & json, bool value) const { return json.write(value ? "true" : "false"); }
};

template<typename Integral>
struct value_type<Integral,
  typename enable_if<is_integral<Integral>::value
    && !is_same<Integral, bool>::value>::type> {
  template<typename Return, typename Json>
  Return
  apply(Json const & json, Integral value) const { return json.write(+value); }
};

template<typename FloatingPoint>
struct value_type<FloatingPoint,
  typename enable_if<is_floating_point<FloatingPoint>::value>::type> {
  template<typename Return, typename Json>
  Return
  apply(Json const & json, FloatingPoint value) const {
    if (std::isfinite(value)) { return json.write(value); }
    else return value_type<null_type>().template apply<Return>(json, null);
  }
};

} // namespace json

#include <string>

namespace json {

template<typename String, typename Char>
struct string_type
{
  static const bool is_string = true;

  template<typename Return, typename Json>
  typename enable_if<is_same<typename Json::char_type, Char>::value,
    Return>::type
  apply(Json const & json, String const & value) const {
    std::basic_string<Char> str(value);
    return json.write_quoted(str.begin(), str.end());
  }
};

template<typename Char>
struct value_type<Char *> : public string_type<Char *, Char> {};
template<typename Char>
struct value_type<Char const *> : public string_type<Char const *, Char> {};
template<typename Char, unsigned N>
struct value_type<Char [N]> : public string_type<Char [N], Char> {};
template<typename Char, unsigned N>
struct value_type<Char const [N]> : public string_type<Char const [N], Char> {};
template<typename Char>
struct value_type<std::basic_string<Char> >
  : public string_type<std::basic_string<Char>, Char > {};

} // namespace json

#endif // __JSON_OSTREAM__
