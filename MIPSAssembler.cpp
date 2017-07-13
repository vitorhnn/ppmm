#include "MIPSAssembler.hpp"

#include <unordered_map>

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
         */
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
    }

    qi::rule<Iterator, Directive(), Skipper> directive;
    qi::rule<Iterator, Instruction(), Skipper> instruction;
    qi::rule<Iterator, Label(), Skipper> label;
    qi::rule<Iterator, Line(), Skipper> line;
    qi::rule<Iterator, MipsAsm(), Skipper> root;
};

static std::unordered_map<std::string, uint32_t> registerMap = {
    {"$zero", 0},
    {"$at", 1},
    {"$v0", 2},
    {"$v1", 3},
    {"$a0", 4},
    {"$a1", 5},
    {"$a2", 6},
    {"$a3", 7},
    {"$t0", 8},
    {"$t1", 9},
    {"$t2", 10},
    {"$t3", 11},
    {"$t4", 12},
    {"$t5", 13},
    {"$t6", 14},
    {"$t7", 15},
    {"$s0", 16},
    {"$s1", 17},
    {"$s2", 18},
    {"$s3", 19},
    {"$s4", 20},
    {"$s5", 21},
    {"$s6", 22},
    {"$s7", 23},
    {"$t8", 24},
    {"$t9", 25},
    {"$k0", 26},
    {"$k1", 27},
    {"$gp", 28},
    {"$sp", 29},
    {"$fp", 30},
    {"$ra", 31}
};

static std::unordered_map<std::string, uint32_t> opcodeMap = {
    {"beq", 4},
    {"bne", 5},
    {"blez", 6},
    {"bgtz", 7},
    {"addi", 8},
    {"addiu", 9},
    {"slti", 10},
    {"sltiu", 11},
    {"andi", 12},
    {"ori", 13},
    {"xori", 14},
    {"lui", 15},
    {"lb", 32},
    {"lh", 33},
    {"lw", 35},
    {"lbu", 36},
    {"lhu", 37},
    {"sb", 40},
    {"sh", 41},
    {"sw", 43}
};

static std::unordered_map<std::string, uint32_t> functMap = {
    {"sll", 0},
    {"srl", 2},
    {"sra", 3},
    {"sra", 3},
    {"sllv", 4},
    {"srlv", 6},
    {"srav", 7},
    {"jr", 8},
    {"jalr", 9},
    {"jalr", 9},
    {"syscall", 12},
    {"mfhi", 16},
    {"mthi", 17},
    {"mflo", 18},
    {"mtlo", 19},
    {"mult", 24},
    {"multu", 25},
    {"div", 26},
    {"divu", 27},
    {"add", 32},
    {"addu", 33},
    {"sub", 34},
    {"subu", 35},
    {"and", 36},
    {"or", 37},
    {"xor", 38},
    {"nor", 39},
    {"slt", 42},
    {"sltu", 43}
};

uint32_t AssembleIType(const Instruction& instruction)
{
    if (instruction.name == "blez" || instruction.name == "bgtz") {
        uint32_t opcode = opcodeMap[instruction.name] << 26;
        uint32_t rs = registerMap[instruction.arguments[0]] << 21;
        uint32_t imm = std::stoi(instruction.arguments[1]);

        return opcode | rs | imm;
    }

    if (instruction.name == "lui") {
        uint32_t opcode = opcodeMap[instruction.name] << 26;
        uint32_t rt = registerMap[instruction.arguments[0]] << 16;
        uint32_t imm = std::stoi(instruction.arguments[1]);

        return opcode | rt | imm;
    }


    uint32_t opcode = opcodeMap[instruction.name] << 26;

    if ((opcode >> 26) >= 32) { // loads and stores.

        auto split = [](const std::string& argument) {
            std::string rs, imm;
            size_t i;

            for (i = 0; i < argument.length() && argument[i] != '('; ++i) imm.push_back(argument[i]);

            for (i++; i < argument.length() && argument[i] != ')'; ++i) rs.push_back(argument[i]);

            return std::make_pair(imm, rs);
        };

        uint32_t rt = registerMap[instruction.arguments[0]] << 16;

        // split the offset($register) argument

        auto pair = split(instruction.arguments[1]);

        uint32_t rs = registerMap[pair.second] << 21;

        uint32_t imm = std::stoi(pair.first);

        return opcode | rs | rt | imm;
    }

    uint32_t rs = registerMap[instruction.arguments[0]] << 21;
    uint32_t rt = registerMap[instruction.arguments[1]] << 16;
    uint32_t imm = std::stoi(instruction.arguments[2]);

    return opcode | rs | rt | imm;
}

uint32_t AssembleRType(const Instruction& instruction)
{
    if (instruction.name == "sll" ||
            instruction.name == "srl" ||
            instruction.name == "sra")
    {
        uint32_t opcode = 0;

        uint32_t rd = registerMap[instruction.arguments[0]] << 11;
        uint32_t rt = registerMap[instruction.arguments[1]] << 16;

        uint32_t shamt = instruction.arguments[2] << 6;

        return opcode | rd | rt | shamt;
    }

    if (instruction.name == "jr") {
        uint32_t opcode = 0;

        uint32_t rs = registerMap[instruction.arguments[0]] << 21;

        uint32_t funct = functMap[instruction.name];

        return opcode | rs | funct;
    }

    if (instruction.name == "jalr") {
        if (instruction.arguments.size() > 1) {
            // non-implicit jalr

            uint32_t opcode = 0;

            uint32_t rd = registerMap[instruction.arguments[0]] << 11;
            uint32_t rs = registerMap[instruction.arguments[1]] << 21;

            uint32_t funct = functMap[instruction.name];

            return opcode | rs | rd | funct;
        }

        uint32_t opcode = 0;

        uint32_t rd = 31 << 11;
        uint32_t rs = registerMap[instruction.arguments[0]] << 21;

        uint32_t funct = functMap[instruction.name];

        return opcode | rs | rd | funct;
    }

    if (instruction.name == "syscall") {
        uint32_t opcode = 0;
        uint32_t funct = functMap[instruction.name];

        return opcode | funct;
    }

    if (instruction.name == "mfhi" || instruction.name == "mflo") {
        uint32_t opcode = 0;
        uint32_t rd = registerMap[instruction.arguments[0]] << 11;
        uint32_t funct = functMap[instruction.name];

        return opcode | rd | funct;
    }

    if (instruction.name == "mult" ||
            instruction.name == "multu" ||
            instruction.name == "div" ||
            instruction.name == "divu")
    {
        uint32_t opcode = 0;

        uint32_t rs = registerMap[instruction.arguments[0]] << 21;
        uint32_t rt = registerMap[instruction.arguments[1]] << 16;

        uint32_t funct = functMap[instruction.name];

        return opcode | rs | rt | funct;
    }

    uint32_t opcode = 0;
    uint32_t rd = registerMap[instruction.arguments[0]] << 11;
    uint32_t rt = registerMap[instruction.arguments[1]] << 16;
    uint32_t rs = registerMap[instruction.arguments[2]] << 21;

    uint32_t funct = functMap[instruction.name];

    return opcode | rs | rt | rd | funct;
}

std::vector<uint32_t> Assemble(std::string assembly)
{
    using grammar = AsmGrammar<std::string::iterator>;
    using Skipper = skipper<std::string::iterator>;

    MipsAsm asm_;

    grammar g;
    Skipper sp;

    auto res = qi::phrase_parse(assembly.begin(), assembly.end(), g, sp, asm_);

    if (!res) {
        throw std::exception();
    }

    size_t linec = 0;

    std::unordered_map<std::string, size_t> labelMap;

    for (const auto& line : asm_.lines) {
        if (line.label) {
            labelMap[line.label.get().name] = linec;
        }

        linec++;
    }

    for (const auto& line : asm_.lines) {

    }

    return std::vector<uint32_t>();
}

int main()
{
    Instruction ins;
    ins.name = "lw";
    ins.arguments.push_back("$a0");
    ins.arguments.push_back("0($v0)");

    return AssembleIType(ins);
}
