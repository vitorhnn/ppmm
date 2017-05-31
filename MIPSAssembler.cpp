#include "MIPSAssembler.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

#include <boost/optional.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

// good ol' MIPS instruction
struct Instruction {
    std::string name;
    std::vector<std::string> arguments;
};

BOOST_FUSION_ADAPT_STRUCT(
        Instruction,
        (std::string, name),
        (std::vector<std::string>, arguments)
)

// assembler directives. things like .asciiz
struct Directive {
    std::string name;
    std::string argument;
};

BOOST_FUSION_ADAPT_STRUCT(
        Directive,
        (std::string, name),
        (std::string, argument)
)

// Address labels
struct Label {
    std::string name;
};

BOOST_FUSION_ADAPT_STRUCT(
        Label,
        (std::string, name)
)

using Instruction_or_Directive = boost::variant<Instruction, Directive>;
// A line. Contains an optional label and either an instruction or a directive.
struct Line {
    boost::optional<Label> label;
    Instruction_or_Directive IoD;
};

BOOST_FUSION_ADAPT_STRUCT(
        Line,
        (boost::optional<Label>, label),
        (Instruction_or_Directive, IoD)
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
            (space - qi::eol) | ('#' >> *(char_ - qi::eol));
    }

    qi::rule<Iterator> start;
};

template <typename Iterator, typename Skipper = skipper<Iterator>>
struct AsmGrammar : qi::grammar<Iterator, MipsAsm(), Skipper> {
    AsmGrammar() : AsmGrammar::base_type(root)
    {
        using ascii::char_;
        using qi::_1;
        using qi::lexeme;
        using phoenix::push_back;
        using phoenix::at_c;
        using qi::as_string;
        using phoenix::val;
        using namespace qi::labels;

        directive = 
            '.' 
            >> lexeme 
            [ 
                as_string
                [
                    (
                        +(
                            ~char_(' ')
                        )
                    )
                ] [at_c<0>(_val) += _1]
                >> ' '
            ]
            >> +(char_ - qi::eol) [at_c<1>(_val) += _1];

        instruction = 
            lexeme 
            [
                as_string
                [
                    (
                        +(
                            ~char_(' ')
                        )
                    )
                ] [at_c<0>(_val) = _1]
                >> ' '
            ]
            >> 
            *(
                as_string
                [
                    (
                        +(
                            ~char_(',')
                        )
                    )
                ] [push_back(at_c<1>(_val), _1)]
                >> -char_(',')
            );

        label =
            *(
                ~char_(':')
            )
            >> ':';

        line = 
            -label [at_c<0>(_val) = _1]
            >> (directive | instruction) [at_c<1>(_val) = _1]
            >> -qi::eol;

        root =
            +(line) [push_back(at_c<0>(_val), _1)];

        /*
         * DEBUG GARBO
         *
        root.name("root");
        line.name("line");
        label.name("label");
        directive.name("directive");
        instruction.name("instruction");

        debug(root);
        debug(line);
        debug(label);
        debug(directive);
        debug(instruction);
        */
    }

    qi::rule<Iterator, Directive(), Skipper> directive;
    qi::rule<Iterator, Instruction(), Skipper> instruction;
    qi::rule<Iterator, Label(), Skipper> label;
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

    std::string asd = "teste: .helicoptero voando\nsuamae aquela,puta";
    auto res2 = qi::phrase_parse(asd.begin(), asd.end(), g, sp, asm_);


    return 0;
}
