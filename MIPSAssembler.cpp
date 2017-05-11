#include "MIPSAssembler.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

template <typename Iterator>
bool parse_label(Iterator first, Iterator last, std::string& out)
{
    using qi::char_;
    using qi::phrase_parse;
    using qi::_1;
    using ascii::space;

    bool r = phrase_parse(
        first,
        last,
        (+~char_(':') >> ':'),
        space
    );

    if (first != last) {
        return false;
    }

    return r;
}

int main()
{
    std::string out;
    std::string input = "youcantseehim:";

    auto res = parse_label(input.begin(), input.end(), out);


    return 0;
}