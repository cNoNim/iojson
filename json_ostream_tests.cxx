#include "json_ostream.hxx"
#include <iostream>

struct vec2i {
  int x, y;
  vec2i(int x, int y) : x(x), y(y) {}
};

namespace json {
template<>
struct value_type<vec2i> {
  template<typename Json>
  closed_tag
  operator()(Json const & json, vec2i const & value) const {
    return json << object << "x" << value.x << "y" << value.y << close;
  }
};
}

int main() {
  using namespace std;

  char const * hello = "hello";
  char world[] = "world";

  cout << json::array <<
    json::value << vec2i(3, 4) << 'c' << hello << world <<
    json::close;
  cout << endl;
  cout << json::value << json::value << json::value << json::value << vec2i(1, 2);
  cout << endl;
  cout << json::array << 1 << 2 << 3 << json::close;
  cout << endl;
  cout << json::object <<
    "null" << json::null <<
    "true" << true <<
    "false" << false <<
    "int" << json::value << 1 <<
    "float" << 0.1 <<
    "string" << "simple" <<
    "escaped_string" << "\b\t\n\f\r\"\\\x7f" <<
    "nested" << json::object <<
        "empty" << json::object << json::close <<
        "foo" << "bar" <<
      json::close <<
    "object" << vec2i(1, 2) <<
    "empty_array" << json::array << json::close <<
    "array" << json::array << 1 << 2.2 << "3" << json::close <<
    "array_of_object_1" << json::array <<
      json::value << vec2i(3, 4) <<
      json::object << json::close <<
      json::close <<
    "array_of_object_2" << json::array <<
      json::object << json::close <<
      vec2i(3, 4) <<
      json::close <<
    "array_of_array" << json::array <<
      json::value << json::array << 1 << 2 << 3 << json::close <<
      json::array << 4 << 5 << 6 << json::close <<
      json::close <<
    "object_with_array" << json::object <<
      "array" << json::array << 1 << 2 << 3 << json::close <<
      json::close <<
    json::close;
  cout << endl;
  return 0;
}
