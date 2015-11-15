#include <cmath>
#include <cstddef>
#include <iomanip>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

namespace json {

template<typename T, typename Enabled = void>
struct value_type;

inline namespace _v1 {

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
  using this_type = __quoting_proxy;
  OStream & os;

public:
  using stream_type = OStream;
  using char_type   = typename OStream::char_type;
  using traits_type = typename OStream::traits_type;

  explicit
  __quoting_proxy(stream_type & os) : os(os) { }

  stream_type & get_stream() const { return os; }

  friend stream_type &
  operator<<(this_type const & proxy, std::basic_string<char_type> const & str) {
    using namespace std;
    auto & os = proxy.get_stream();
    os << '"';
    auto flags = os.flags(ios::hex | ios::right);
    auto fill = os.fill('0');
    for(auto && c : str)
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
  using this_type = __base_value_proxy;

public:
  using stream_type = OStream;
  using parent_type = Parent;
  using char_type   = typename OStream::char_type;
  using traits_type = typename OStream::traits_type;

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
  using this_type = __base_value_proxy_spec;
  using base_type = __base_value_proxy<OStream, Parent>;

public:
  using stream_type = typename base_type::stream_type;
  using parent_type = typename base_type::parent_type;
  using char_type   = typename base_type::char_type;
  using traits_type = typename base_type::traits_type;

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
  using this_type = __base_value_proxy_spec;
  using base_type = __base_value_proxy<OStream, OStream>;

public:
  using stream_type = typename base_type::stream_type;
  using parent_type = typename base_type::parent_type;
  using char_type   = typename base_type::char_type;
  using traits_type = typename base_type::traits_type;

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
  using this_type = __base_collection_proxy;
  using base_type = __base_value_proxy_spec<OStream, Parent>;

public:
  using stream_type = typename base_type::stream_type;
  using parent_type = typename base_type::parent_type;
  using char_type   = typename base_type::char_type;
  using traits_type = typename base_type::traits_type;

protected:
  __base_collection_proxy(stream_type & os, parent_type const & parent)
    : base_type(os, parent), state(__state::Empty) {}

  bool open() const {
    if (!is_open()) return false;
    auto & os = get_stream();
    os << (state == __state::Empty ? oc : dc);
    state = __state::Opened;
    return true;
  }

  void close() const {
    if (!is_open()) return;
    auto & os = get_stream();
    if (state == __state::Empty) os << oc;
    state = __state::Closed;
    os << cc;
  }

public:
  bool is_open() const { return state != __state::Closed; }
  using base_type::get_stream;
  using base_type::get_parent;


protected:
  enum class __state { Empty, Opened, Closed } mutable state;
};

template<typename OStream, typename Parent = OStream>
class __value_proxy
  : public __base_value_proxy_spec<OStream, Parent> {
  template<typename T, typename U> friend class __value_proxy;
  template<typename T, typename U> friend class __array_proxy;
  template<typename T, typename U> friend class __object_proxy;
  using this_type = __value_proxy;
  using base_type = __base_value_proxy_spec<OStream, Parent>;

public:
  using stream_type = typename base_type::stream_type;
  using parent_type = typename base_type::parent_type;
  using char_type   = typename base_type::char_type;
  using traits_type = typename base_type::traits_type;
  using auto_parent_type = typename std::conditional<
    std::is_same<stream_type, parent_type>::value, stream_type, parent_type const >::type;

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
  using this_type = __array_proxy;
  using base_type = __base_collection_proxy<OStream, Parent, '[', ',', ']'>;

public:
  using stream_type = typename base_type::stream_type;
  using parent_type = typename base_type::parent_type;
  using char_type   = typename base_type::char_type;
  using traits_type = typename base_type::traits_type;
  using auto_parent_type = typename std::conditional<
    std::is_same<stream_type, parent_type>::value, stream_type, parent_type const >::type;

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
  using this_type = __object_proxy;
  using base_type = __base_collection_proxy<OStream, Parent, '{', ',', '}'>;

public:
  using stream_type = typename base_type::stream_type;
  using parent_type = typename base_type::parent_type;
  using char_type   = typename base_type::char_type;
  using traits_type = typename base_type::traits_type;
  using auto_parent_type = typename std::conditional<
    std::is_same<stream_type, parent_type>::value, stream_type, parent_type const >::type;

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
}

template<>
struct value_type<std::nullptr_t> {
  template<typename Json>
  void
  operator()(Json const & json, std::nullptr_t) const {
    json.get_stream() << "null";
  }
};

template<>
struct value_type<bool> {
  template<typename Json>
  void
  operator()(Json const & json, bool value) const { json.get_stream() << (value ? "true" : "false"); }
};

template<typename Integral>
struct value_type<Integral, typename std::enable_if<std::is_integral<Integral>::value
    && !std::is_same<Integral, bool>::value>::type> {
  template<typename Json>
  void
  operator()(Json const & json, Integral value) const { json.get_stream() << +value; }
};

template<typename FloatingPoint>
struct value_type<FloatingPoint, typename std::enable_if<std::is_floating_point<FloatingPoint>::value>::type> {
  template<typename Json>
  void
  operator()(Json const & json, FloatingPoint value) const {
    if (std::isfinite(value)) json.get_stream() << value;
    else json::value_type<std::nullptr_t>()(json, nullptr);
  }
};

template<typename Char>
struct value_type<std::basic_string<Char>> {
  template<typename Json>
  typename std::enable_if<std::is_same<typename Json::char_type, Char>::value>::type
  operator()(Json const & json, std::basic_string<Char> const & value) const {
    json.get_stream() << quote << value;
  }
};

template<typename Char>
struct value_type<Char *,
  typename std::char_traits<Char>::char_type> {
  template<typename Json>
  typename std::enable_if<std::is_same<typename Json::char_type, Char>::value>::type
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
