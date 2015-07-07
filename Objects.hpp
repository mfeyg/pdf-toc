#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/stl/container.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <string>
#include <map>

namespace spirit = boost::spirit;
namespace qi = spirit::qi;
namespace karma = spirit::karma;

struct Boolean {
  bool value;
};

struct Integer_ {
  int value;
};

struct Real {
  double value;
};

struct String {
  std::string value;
};

struct Name {
  std::string value;
};

bool operator<(const Name& a, const Name& b) {
  return a.value < b.value;
}

BOOST_FUSION_ADAPT_STRUCT(Boolean, (bool, value));
BOOST_FUSION_ADAPT_STRUCT(Integer_, (int, value));
BOOST_FUSION_ADAPT_STRUCT(Real, (double, value));
BOOST_FUSION_ADAPT_STRUCT(String, (std::string, value));
BOOST_FUSION_ADAPT_STRUCT(Name, (std::string, value));

struct Null {};

struct Array;
struct Dictionary;

typedef boost::variant<Boolean,
                       Integer_,
                       Real,
                       String,
                       Name,
                       boost::recursive_wrapper<Array>,
                       boost::recursive_wrapper<Dictionary>,
                       Null
                      > Object;

struct Array {
  std::vector<Object> value;
};
BOOST_FUSION_ADAPT_STRUCT(Array, (std::vector<Object>, value));

struct Dictionary {
  typedef std::map<Name,Object> dict_type;
  dict_type value;
};
BOOST_FUSION_ADAPT_STRUCT(Dictionary, (Dictionary::dict_type, value));

template <typename Iterator>
struct Object_grammar : qi::grammar<Iterator, Object()>
{
  Object_grammar() : Object_grammar::base_type(object) {
    using qi::double_;
    using qi::int_;
    using qi::bool_;
    using qi::char_;
    using qi::lit;
    using namespace qi::labels;
    using qi::repeat;
    using boost::phoenix::bind;
    using qi::ascii::xdigit;
    using qi::attr;
    using qi::ascii::space;
    using qi::omit;

    object %= boolean | integer | real | string | name | array | dictionary | null;

    boolean %= bool_;

    integer %= int_ >> &(space | delim);

    real %= double_;

    string %= '(' >> lit_string >> ')' | '<' >> hex_string >> '>';
    lit_string %= *(plain_string | substring | escape_code);
    substring %= char_('(') >> lit_string >> char_(')');
    plain_string %= +(char_ - char_("()\\"));
    escape_code = lit("\\n")[_val+="\n"]
                | lit("\\r")[_val+="\r"]
                | lit("\\t")[_val+="\t"]
                | lit("\\b")[_val+="\b"]
                | lit("\\f")[_val+="\f"]
                | lit("\\(")[_val+="("]
                | lit("\\)")[_val+=")"]
                | lit("\\\\")[_val+="\\"]
                | lit("\\\n")
                | octal_escape [_val += bind(&Object_grammar::parse_base, 8, _1)];
    octal_escape = '\\' >> repeat(1,3)[char_('0','7')];
    hex_char = xdigit >> xdigit | xdigit >> attr('0');
    hex_string = *(hex_char[_val += bind(&Object_grammar::parse_base, 16, _1)]);

    delim %= char_("()<>[]{}/%");

    name %= '/' >> *(/*name_escaped_char |*/ char_ - delim - space);
    name_escaped_char = name_escape_seq [_val = bind(&Object_grammar::parse_base, 16, _1)];
    name_escape_seq = '#' >> xdigit >> xdigit;

    array %= '[' >> omit[*space] >> *(object >> *space) >> ']';

    dictionary %= "<<" >> omit[*space] >> dict >> ">>";
    dict = *dict_entry;
    dict_entry %= name >> omit[*space] >> object >> omit[*space];

    null = lit("null")[_val = Null()];
  }

  qi::rule<Iterator, Boolean()> boolean;

  qi::rule<Iterator, Integer_()> integer;

  qi::rule<Iterator, Real()> real;

  qi::rule<Iterator, String()> string;
  qi::rule<Iterator, std::string()> lit_string;
  qi::rule<Iterator, std::string()> plain_string;
  qi::rule<Iterator, std::string()> escape_code;
  qi::rule<Iterator, std::string()> octal_escape;
  qi::rule<Iterator, std::string()> substring;
  qi::rule<Iterator, std::string()> hex_string;
  qi::rule<Iterator, std::string()> hex_char;

  qi::rule<Iterator, Name()> name;
  qi::rule<Iterator, std::string()> name_escape_seq;
  qi::rule<Iterator, char()> name_escaped_char;

  qi::rule<Iterator, char()> delim;

  qi::rule<Iterator, Array()> array;

  qi::rule<Iterator, Dictionary()> dictionary;
  qi::rule<Iterator, Dictionary::dict_type()> dict;
  qi::rule<Iterator, std::pair<Name,Object>() > dict_entry;

  qi::rule<Iterator, Null()> null;

  qi::rule<Iterator, Object()> object;

  static char parse_base(int base, const std::string& digs) {
    char res = 0;
    char mul = 1;
    for (std::string::const_reverse_iterator it = digs.rbegin(); it != digs.rend(); ++it) {
      char val;
      if ('0' <= *it && *it <= '9') val = *it - '0';
      else if ('a' <= *it && *it <= 'z') val = 10 + *it - 'a';
      else if ('A' <= *it && *it <= 'Z') val = 10 + *it - 'A';
      res += val * mul;
      mul *= base;
    }
    return res;
  }
};

template <typename OutputIterator>
struct Object_generator : karma::grammar<OutputIterator, Object()>
{
  Object_generator() : Object_generator::base_type(object) {
    using karma::int_;
    using karma::double_;
    using karma::bool_;
    using karma::char_;
    using karma::duplicate;
    using karma::string;
    using karma::ascii::space;
    using karma::lit;
    using namespace karma::labels;

    integer = int_;

    real = double_;

    boolean = bool_;

    string_ = '(' << *escape << ')';
    escape = plain_char | '\\' << char_;
    plain_char = duplicate[!char_("\\()") << char_];

    name = '/' << string;

    array = '[' << -(object % space) << ']';

    dict_entry = name << '\t' << object;
    dict = -(dict_entry % '\n');
    dictionary = "<<" << dict << ">>";

    null = lit("null")[_1 = Null()];

    object = boolean | integer | real | string_ | name | array | dictionary | null;
  }

  karma::rule<OutputIterator, Boolean()> boolean;

  karma::rule<OutputIterator, Integer_()> integer;
  karma::rule<OutputIterator, Real()> real;

  karma::rule<OutputIterator, String()> string_;
  karma::rule<OutputIterator, char()> escape;
  karma::rule<OutputIterator, char()> plain_char;

  karma::rule<OutputIterator, Name()> name;

  karma::rule<OutputIterator, Array()> array;

  karma::rule<OutputIterator, Dictionary()> dictionary;
  karma::rule<OutputIterator, std::map<Name, Object>()> dict;
  karma::rule<OutputIterator, std::pair<const Name,Object>()> dict_entry;

  karma::rule<OutputIterator, Null()> null;

  karma::rule<OutputIterator, Object()> object;
};
