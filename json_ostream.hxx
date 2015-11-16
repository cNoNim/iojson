#ifndef __JSON_OSTREAM__
#define __JSON_OSTREAM__

#if __cplusplus > 199711L || _MSC_VER > 1800
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
#else
namespace json {

template <bool, typename I, typename> struct conditional { typedef I type; };
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
#endif

namespace json {

enum null_type { null = 0 };
template<typename T, typename Enabled = void>
struct value_type;

#if __cplusplus > 199711L
inline
#endif
namespace _v1 {

struct closed_tag {} close;
struct array_tag  {} array;
struct object_tag {} object;
struct value_tag  {} value;

template<typename T> struct is_tag { static const bool value = false; };
template<> struct is_tag<array_tag> { static const bool value = true; };
template<> struct is_tag<object_tag> { static const bool value = true; };
template<> struct is_tag<value_tag> { static const bool value = true; };

template<typename OStream, typename Parent = closed_tag> class __array_proxy;
template<typename OStream, typename Parent = closed_tag> class __object_proxy;
template<typename OStream, typename Parent = closed_tag> class __value_proxy;

template<typename Tag, typename OStream, typename Parent = closed_tag>
struct __tag_proxy_map;

template<typename Tag, typename OStream>
typename __tag_proxy_map<Tag, OStream>::type
operator<<(OStream & os, Tag const &);

class api_tag {
  api_tag() {}
  static api_tag const & get() { static api_tag tag; return tag; }

  template<typename OStream, typename Parent>
  friend class __base_value_proxy;

  template<typename Tag, typename OStream>
  friend typename __tag_proxy_map<Tag, OStream>::type
  operator<<(OStream &, Tag const &);
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

  api_tag const &
  tag() const { return api_tag::get(); }
  stream_type &
  stream() const { return os; }
  parent_type const &
  close() const { return parent; }

private:
  stream_type & os;
  parent_type const & parent;
};

template<typename OStream, typename Parent,
  typename OStream::char_type oc, typename OStream::char_type dc, typename OStream::char_type cc>
class __base_collection_proxy
  : __base_value_proxy<OStream, Parent> {
  typedef __base_collection_proxy             this_type;
  typedef __base_value_proxy<OStream, Parent> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;

protected:
  __base_collection_proxy(stream_type & os, parent_type const & parent)
    : base_type(os, parent), state(Empty) {}

  using base_type::tag;
  using base_type::stream;
  parent_type const &
  close() const {
    if (is_open()) {
      if (is_empty()) stream() << oc;
      state = Closed;
      stream() << cc;
    }
    return base_type::close();
  }

  void
  open() const {
    if (!is_open()) return;
    stream() << (is_empty() ? oc : dc);
    state = Opened;
  }

private:
  bool is_open() const { return state != Closed; }
  bool is_empty() const { return state == Empty; }
  enum { Empty, Opened, Closed } mutable state;
};

template<typename OStream, typename Parent>
class __value_proxy
  : public __base_value_proxy<OStream, Parent> {
  typedef __value_proxy                            this_type;
  typedef __base_value_proxy<OStream, Parent> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;

  __value_proxy(api_tag const &, stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

  template<typename T>
  parent_type const &
  write(T const & v) const { stream() << v; return close(); }

  template<typename Iterator>
  typename enable_if<is_same<typename Iterator::value_type, char_type>::value, parent_type const &>::type
  write_quoted(Iterator begin, Iterator end) const {
    typedef typename Iterator::value_type char_type;
    static char_type hex[]  = {
      '0', '1', '2', '3',
      '4', '5', '6', '7',
      '8', '9', 'a', 'b',
      'c', 'd', 'e', 'f'
    };
    stream_type & os = stream();
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
    return close();
  }

private:
  using base_type::tag;
  using base_type::stream;
  using base_type::close;

  template<typename T>
  friend typename enable_if<!is_tag<T>::value, parent_type const &>::type
  operator<<(this_type const & proxy, T const & v) {
    value_type<T>()(__value_proxy<stream_type>(proxy.tag(), proxy.stream(), json::_v1::close), v);
    return proxy.close();
  }

  template<typename Tag>
  friend typename __tag_proxy_map<Tag, stream_type, parent_type>::type
  operator<<(this_type const & proxy, Tag const &) {
    typedef typename __tag_proxy_map<Tag, stream_type, parent_type>::type proxy_type;
    return proxy_type(proxy.tag(), proxy.stream(), proxy.close());
  }
};

template<typename OStream, typename Parent>
class __array_proxy
  : public __base_collection_proxy<OStream, Parent, '[', ',', ']'> {
  typedef __array_proxy                                           this_type;
  typedef __base_collection_proxy<OStream, Parent, '[', ',', ']'> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;

  __array_proxy(api_tag const &, stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

private:
  using base_type::tag;
  using base_type::stream;
  using base_type::close;

  template<typename T>
  friend typename enable_if<!is_tag<T>::value, this_type const &>::type
  operator<<(this_type const & proxy, T const & v) {
    return proxy << value << v;
  }


  template<typename Tag>
  friend typename __tag_proxy_map<Tag, stream_type, this_type>::type
  operator<<(this_type const & proxy, Tag const &) {
    proxy.open();
    typedef typename __tag_proxy_map<Tag, stream_type, this_type>::type proxy_type;
    return proxy_type(proxy.tag(), proxy.stream(), proxy);
  }

  friend parent_type const &
  operator<<(this_type const & proxy, closed_tag const &) {
    return proxy.close();
  }
};

template<typename OStream, typename Parent>
class __object_proxy
  : public __base_collection_proxy<OStream, Parent, '{', ',', '}'> {
  typedef __object_proxy                                          this_type;
  typedef __base_collection_proxy<OStream, Parent, '{', ',', '}'> base_type;

public:
  typedef typename base_type::stream_type stream_type;
  typedef typename base_type::parent_type parent_type;
  typedef typename base_type::char_type   char_type;

  __object_proxy(api_tag const &, stream_type & os, parent_type const & parent)
    : base_type(os, parent) {}

private:
  using base_type::tag;
  using base_type::stream;
  using base_type::close;

  template<typename String>
  friend typename enable_if<value_type<String>::is_string,
    __value_proxy<stream_type, this_type> >::type
  operator<<(this_type const & proxy, String const & str) {
    proxy.open();
    __value_proxy<stream_type, this_type> value(proxy.tag(), proxy.stream(), proxy);
     value << str;
    value.write(':');
    return value;
  }

  friend parent_type const &
  operator<<(this_type const & proxy, closed_tag const &) {
    return proxy.close();
  }
};

template<typename OStream, typename Parent>
struct __tag_proxy_map<array_tag, OStream, Parent> { typedef __array_proxy<OStream, Parent> type; };
template<typename OStream, typename Parent>
struct __tag_proxy_map<object_tag, OStream, Parent> { typedef __object_proxy<OStream, Parent> type; };
template<typename OStream, typename Parent>
struct __tag_proxy_map<value_tag, OStream, Parent> { typedef __value_proxy<OStream, Parent> type; };

template<typename Tag, typename OStream>
typename __tag_proxy_map<Tag, OStream>::type
operator<<(OStream & os, Tag const &) { return typename __tag_proxy_map<Tag, OStream>::type(api_tag::get(), os, close); }

} // namespace json::_v1

#if __cplusplus <= 199711L
using _v1::close;
using _v1::closed_tag;

using _v1::array;
using _v1::object;
using _v1::value;
#endif

} // namespace json

#include <cmath>

namespace json {

template<>
struct value_type<null_type> {
  template<typename Json>
  closed_tag
  operator()(Json const & json, null_type) const { return json.write("null"); }
};

#if __cplusplus > 199711L
template<>
struct value_type<std::nullptr_t> : public value_type<null_type> { };
#endif

template<>
struct value_type<bool> {
  template<typename Json>
  closed_tag
  operator()(Json const & json, bool value) const { return json.write(value ? "true" : "false"); }
};

template<typename Integral>
struct value_type<Integral,
  typename enable_if<is_integral<Integral>::value && !is_same<Integral, bool>::value>::type> {
  template<typename Json>
  closed_tag
  operator()(Json const & json, Integral value) const { return json.write(+value); }
};

template<typename FloatingPoint>
struct value_type<FloatingPoint,
  typename enable_if<is_floating_point<FloatingPoint>::value>::type> {
  template<typename Json>
  closed_tag
  operator()(Json const & json, FloatingPoint value) const {
    if (std::isfinite(value)) { return json.write(value); }
    else return json << null;
  }
};

} // namespace json

#include <string>

namespace json {

template<typename String, typename Char>
struct string_type
{
  static const bool is_string = true;

  template<typename Json>
  typename enable_if<is_same<typename Json::char_type, Char>::value, closed_tag>::type
  operator()(Json const & json, String const & value) const {
    std::basic_string<Char> str(value);
    return json.write_quoted(str.begin(), str.end());
  }
};

template<typename Char> struct value_type<Char *> : public string_type<Char *, Char> {};
template<typename Char> struct value_type<Char const *> : public string_type<Char const *, Char> {};
template<typename Char, unsigned N> struct value_type<Char [N]> : public string_type<Char [N], Char> {};
template<typename Char, unsigned N> struct value_type<Char const [N]> : public string_type<Char const [N], Char> {};
template<typename Char> struct value_type<std::basic_string<Char> > : public string_type<std::basic_string<Char>, Char > {};

} // namespace json

#endif // __JSON_OSTREAM__
