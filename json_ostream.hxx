#include <cmath>
#include <cstddef>
#include <iomanip>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

namespace json {

template<typename T, typename Enabled = void>
struct value;

inline namespace _v1 {

struct quoting_tag { constexpr quoting_tag() {} } constexpr quote;
struct array_tag   { constexpr array_tag() {} }   constexpr array;
struct object_tag  { constexpr object_tag() {} }  constexpr object;
struct close_tag   { constexpr close_tag() {} }   constexpr close;

template<typename OStream>
class __quoting_proxy {
  OStream & os;

  explicit
  __quoting_proxy(OStream & os) : os(os) { }

  using char_type = typename OStream::char_type;
  using traits_type = typename OStream::traits_type;

  friend auto &
  operator<<(__quoting_proxy const & proxy, std::basic_string<char_type> const & str) {
    using namespace std;
    auto & os = proxy.os;
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

template<typename OStream, typename Parent = OStream>
class __object_proxy;
template<typename OStream, typename Parent = OStream>
class __array_proxy;

template<typename ProxyStream>
class __value_proxy {
  ProxyStream const & proxy;

  explicit
  __value_proxy(ProxyStream const & proxy) : proxy(proxy) {}

  using stream_type = typename ProxyStream::stream_type;
  using char_type = typename ProxyStream::char_type;
  using traits_type = typename ProxyStream::traits_type;

  template<typename T>
  friend auto &
  operator<<(__value_proxy const & proxy, T const & v) {
    if (proxy.proxy.state != ProxyStream::__state::Closed) {
      auto & os = proxy.proxy.os;
      value<T>()(os, v);
    }
    return proxy.proxy;
  }

  friend auto
  operator<<(__value_proxy const & proxy, object_tag const &) {
    return __object_proxy<stream_type, ProxyStream>(proxy.proxy.os, proxy.proxy);
  }

  friend auto
  operator<<(__value_proxy const & proxy, array_tag const &) {
    return __array_proxy<stream_type, ProxyStream>(proxy.proxy.os, proxy.proxy);
  }

  friend ProxyStream;
};

template<typename OStream, typename Parent,
  typename OStream::char_type oc,
  typename OStream::char_type dc,
  typename OStream::char_type cc>
class __base_proxy {
protected:
  using stream_type = OStream;
  using char_type   = typename stream_type::char_type;
  using traits_type = typename stream_type::traits_type;

  enum class __state { Empty, Opened, Closed } mutable state;
  OStream & os;
  Parent const & parent;

  __base_proxy(OStream & os, Parent const & parent)
    : state(__state::Empty), os(os), parent(parent) {}

  bool open() const {
    if (state == __state::Closed) return false;
    os << (state == __state::Empty ? oc : dc);
    state = __state::Opened;
    return true;
  }

  void __close() const {
    if (state != __state::Closed) {
      if (state == __state::Empty) os << oc;
      state = __state::Closed;
      os << cc;
    }
  }
};

template<typename OStream, typename Parent,
  typename OStream::char_type oc,
  typename OStream::char_type dc,
  typename OStream::char_type cc>
class __base_proxy_spec
  : protected __base_proxy<OStream, Parent, oc, dc, cc> {
  using base_type   = __base_proxy<OStream, Parent, oc, dc, cc>;
protected:
  __base_proxy_spec(OStream & os, Parent const & parent) : base_type(os, parent) {}
  Parent const &
  close() const { this->__close(); return this->parent; }
};

template<typename OStream,
  typename OStream::char_type oc,
  typename OStream::char_type dc,
  typename OStream::char_type cc>
class __base_proxy_spec<OStream, OStream, oc, dc, cc>
  : protected __base_proxy<OStream, OStream, oc, dc, cc> {
  using base_type   = __base_proxy<OStream, OStream, oc, dc, cc>;
protected:
  __base_proxy_spec(OStream & os, OStream const & parent) : base_type(os, parent) {}
  OStream &
  close() const { this->__close(); this->os.flush(); return this->os; }
};

template<typename OStream, typename Parent>
class __array_proxy : private __base_proxy_spec<OStream, Parent, '[', ',', ']'> {
  template<typename T> friend class __value_proxy;
  template<typename T, typename U> friend class __array_proxy;
  template<typename T, typename U> friend class __object_proxy;

  using base_type   = __base_proxy_spec<OStream, Parent, '[', ',', ']'>;
  using stream_type = typename base_type::stream_type;
  using char_type   = typename base_type::char_type;
  using traits_type = typename base_type::traits_type;

  explicit
  __array_proxy(OStream & os)
    : base_type(os, os) {}

  __array_proxy(OStream & os, Parent const & parent)
    : base_type(os, parent) {}

  template<typename T>
  friend auto &
  operator<<(__array_proxy const & proxy, T const & v) {
    if (proxy.open()) value<T>()(proxy.os, v);
    return proxy;
  }

  friend auto
  operator<<(__array_proxy const & proxy, object_tag const &) {
    proxy.open();
    return __object_proxy<stream_type, __array_proxy>(proxy.os, proxy);
  }

  friend auto
  operator<<(__array_proxy const & proxy, array_tag const &) {
    proxy.open();
    return __array_proxy<stream_type, __array_proxy>(proxy.os, proxy);
  }

  friend auto &
  operator<<(__array_proxy const & proxy, close_tag const &) {
    return proxy.close();
  }

  template<typename OS>
  friend __array_proxy<OS>
  operator<<(OS & os, array_tag const &);
};

template<typename OStream, typename Parent>
class __object_proxy : private __base_proxy_spec<OStream, Parent, '{', ',', '}'> {
  template<typename T> friend class __value_proxy;
  template<typename T, typename U> friend class __array_proxy;
  template<typename T, typename U> friend class __object_proxy;

  using base_type   = __base_proxy_spec<OStream, Parent, '{', ',', '}'>;
  using stream_type = typename base_type::stream_type;
  using char_type   = typename base_type::char_type;
  using traits_type = typename base_type::traits_type;

  explicit
  __object_proxy(OStream & os)
    : base_type(os, os) {}

  __object_proxy(OStream & os, Parent const & parent)
    : base_type(os, parent) {}

  friend auto
  operator<<(__object_proxy const & proxy, std::basic_string<char_type> const & str) {
    if (proxy.open()) proxy.os << quote << str << ':';
    return __value_proxy<__object_proxy>(proxy);
  }

  friend auto &
  operator<<(__object_proxy const & proxy, close_tag const &) {
    return proxy.close();
  }

  template<typename OS>
  friend __object_proxy<OS>
  operator<<(OS & os, object_tag const &);
};

template<typename OStream>
__quoting_proxy<OStream>
operator<<(OStream & os, quoting_tag const &) {
  return __quoting_proxy<OStream>(os);
}

template<typename OStream>
__object_proxy<OStream>
operator<<(OStream & os, object_tag const &) {
  return __object_proxy<OStream>(os);
}
}

template<>
struct value<std::nullptr_t> {
  template<typename OStream>
  void
  operator()(OStream& os, std::nullptr_t) const {
    os << "null";
  }
};

template<>
struct value<bool> {
  template<typename OStream>
  void
  operator()(OStream& os, bool value) const { os << (value ? "true" : "false"); }
};

template<typename Integral>
struct value<Integral, typename std::enable_if<std::is_integral<Integral>::value
    && !std::is_same<Integral, bool>::value>::type> {
  template<typename OStream>
  void
  operator()(OStream& os, Integral value) const { os << +value; }
};

template<typename FloatingPoint>
struct value<FloatingPoint, typename std::enable_if<std::is_floating_point<FloatingPoint>::value>::type> {
  template<typename OStream>
  void
  operator()(OStream& os, FloatingPoint value) const {
    if (std::isfinite(value)) os << value;
    else json::value<std::nullptr_t>()(os, nullptr);
  }
};

template<typename Char>
struct value<std::basic_string<Char>> {
  template<typename OStream>
  void
  operator()(OStream& os, std::basic_string<Char> const & value) const {
    __write(os, value, std::is_same<typename OStream::char_type, Char>());
  }
private:
  template<typename OStream>
  static void
  __write(OStream& os, std::basic_string<Char> const & value, std::true_type const &) {
    os << quote << value;
  }
};

template<typename Char>
struct value<Char *,
  typename std::char_traits<Char>::char_type> {
  template<typename OStream>
  void
  operator()(OStream& os, Char const * value) const {
    __write(os, value, std::is_same<typename OStream::char_type, Char>());
  }
private:
  template<typename OStream>
  static void
  __write(OStream& os, Char const * value, std::true_type const &) {
    os << quote << value;
  }
};

template<typename Char>
struct value<Char const *> : public value<Char *, Char> {};
template<typename Char, size_t N>
struct value<Char [N]> : public value<Char *, Char> {};
template<typename Char, size_t N>
struct value<Char const [N]> : public value<Char *, Char> {};

}
