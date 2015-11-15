#include "json_ostream.hxx"
#include <iostream>

struct vec2i {
  int x, y;
};

namespace json {
  template<>
  struct value<vec2i> {
    template<typename OStream>
    void
    operator()(OStream& os, vec2i const & value) const {
      os << json::object <<
        "x" << value.x <<
        "y" << value.y <<
        json::close;
    }
  };
}

int main() {
  using namespace std;
  cout << json::object <<
    "null" << nullptr <<
    "true" << true <<
    "false" << false <<
    "int" << 1 <<
    "float" << 0.1 <<
    "string" << "simple" <<
    "escaped_string" << "\b\t\n\f\r\"\\\1" <<
    "nested" << json::object <<
        "empty" << json::object << json::close <<
        "foo" << "bar" <<
      json::close <<
    "object" << vec2i{ 1, 2 } <<
    "empty_array" << json::array << json::close <<
    "array" << json::array << 1 << 2.2 << std::to_string(3) << json::close <<
    "array_of_object_1" << json::array <<
      vec2i{ 3, 4 } <<
      json::object << json::close <<
      json::close <<
    "array_of_object_2" << json::array <<
      json::object << json::close <<
      vec2i{ 3, 4 } <<
      json::close <<
    "array_of_array" << json::array <<
      json::array << 1 << 2 << 3 << json::close <<
      json::array << 4 << 5 << 6 << json::close <<
      json::close <<
    json::close << endl;
  return 0;
}

/* output (pretty):
{
  "null": null,
  "true": true,
  "false": false,
  "int": 1,
  "float": 0.1,
  "string": "simple",
  "escaped_string": "\b\t\n\f\r\"\\\u0001",
  "nested": {
    "empty": {},
    "foo": "bar"
  },
  "object": { "x": 1, "y": 2 },
  "empty_array": [],
  "array": [ 1, 2.2, "3" ],
  "array_of_object_1": [
    { "x": 3, "y": 4 },
    {}
  ],
  "array_of_object_2": [
    {},
    { "x": 3, "y": 4 }
  ],
  "array_of_array": [
    [ 1, 2, 3 ],
    [ 4, 5, 6 ]
  ]
}
*/
