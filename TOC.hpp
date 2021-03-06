#include <string>
#include <vector>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace spirit = boost::spirit;
namespace qi = spirit::qi;

struct TOC {
  std::string name;
  int pages;
  std::vector<TOC> children;
};

BOOST_FUSION_ADAPT_STRUCT(
  TOC,
  (std::string, name)
  (int, pages)
  (std::vector<TOC>, children)
)

template <typename Iterator>
struct TOC_grammar : qi::grammar<Iterator, TOC(), qi::locals<int> >
{
  TOC_grammar() : TOC_grammar::base_type(toc) {
    using qi::eps;
    using qi::lit;
    using qi::int_;
    using spirit::ascii::char_;
    using qi::eol;
    using qi::repeat;
    using qi::inf;
    using qi::omit;
    using namespace qi::labels;

    count %= eps[_val = 0] >> *(lit(_r1)[_val += 1]);

    name %= +(char_ - '\t');

    toc %= omit[count('\t')[_a = _1]]
        >> name
         > +lit('\t')
         > int_
         > eol
         > children(_a+1);

    children %= *(&repeat(_r1,inf)[lit('\t')] >> toc);
  }

  qi::rule<Iterator, TOC(), qi::locals<int> > toc;
  qi::rule<Iterator, std::vector<TOC>(int)> children;
  qi::rule<Iterator, std::string()> name;
  qi::rule<Iterator, int(char)> count;
};
