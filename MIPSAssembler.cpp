#include "MIPSAssembler.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <boost/optional.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

// good ol' MIPS instruction
struct Instruction {
    std::string name;
};

// assembler directives. things like .asciiz
struct Directive {
    std::string name;
    std::string argument;
};

// Address labels
struct Label {
    std::string name;
};

// A line. Contains an optional label and either an instruction or a directive.
struct Line {
    /*
    boost::optional<Label> label;
    boost::variant<Instruction, Directive> IoD;
    */
    std::string help_me;
};

BOOST_FUSION_ADAPT_STRUCT(
        Line,
        (std::string, help_me)
)

// The root of the AST. Basically an array of lines
struct MipsAsm {
    std::vector<Line> lines;
};

BOOST_FUSION_ADAPT_STRUCT(
    MipsAsm,
    (std::vector<Line>, lines)
)

template <typename Iterator>
struct skipper : qi::grammar<Iterator>
{
    skipper() : skipper::base_type(start)
    {
        qi::char_type char_;
        ascii::space_type space;

        start =
            (space - qi::eol) | '#' >> *(char_ - qi::eol);
    }

    qi::rule<Iterator> start;
};

template <typename Iterator, typename Skipper = skipper<Iterator>>
struct AsmGrammar : qi::grammar<Iterator, MipsAsm(), Skipper> {
    AsmGrammar() : AsmGrammar::base_type(root)
    {
        using qi::lit;
        using ascii::char_;
        using qi::_1;
        using boost::spirit::no_skip;
        using phoenix::push_back;
        using phoenix::at_c;
        using namespace qi::labels;

        line = *(char_ - qi::eol) >> qi::eol;

        root =
            *(line) [push_back(at_c<0>(_val), _1)];
    }
    qi::rule<Iterator, Line(), Skipper> line;
    qi::rule<Iterator, MipsAsm(), Skipper> root;
};

int main()
{
    using grammar = AsmGrammar<std::string::iterator>;
    using Skipper = skipper<std::string::iterator>;

    MipsAsm asm_;

    grammar g;
    Skipper sp;

    std::string asd = "helicoptero #comentario\n"
        "li $t0, 10 #comentario 2\n";
    auto res2 = qi::phrase_parse(asd.begin(), asd.end(), g, sp, asm_);


    return 0;
}

