#include "Objects.hpp"
#include <iostream>

namespace qi = boost::spirit::qi;
namespace karma = boost::spirit::karma;

int main() {
  std::cin >> std::noskipws;
  Object obj;
  boost::spirit::istream_iterator begin(std::cin);
  boost::spirit::istream_iterator end;
  Object_grammar<boost::spirit::istream_iterator> grammar;
  bool res = qi::phrase_parse(begin, end, grammar, qi::ascii::space, obj);
  //std::cout << res << std::endl;
  Object_generator<std::ostream_iterator<char> > gen;
  karma::generate_delimited(std::ostream_iterator<char>(std::cout), gen, karma::ascii::space, obj);
  return 0;
}
