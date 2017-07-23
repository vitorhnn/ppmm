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

using Instruction_or_Directive = boost::variant<boost::blank, Instruction, Directive>;
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
    std::list<Line> lines;
};

BOOST_FUSION_ADAPT_STRUCT(
    MipsAsm,
    (std::list<Line>, lines)
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
                            char_ - (char_(' ') | qi::eol)
                        )
                    )
                ] [at_c<0>(_val) += _1]
                >> -char_(' ')
            ]
            >> *(char_ - qi::eol) [at_c<1>(_val) += _1];

        instruction =
            lexeme
            [
                as_string
                [
                    (
                        +(
                            char_ - (char_(' ') | qi::eol)
                         )
                    )
                ] [at_c<0>(_val) = _1]
                >> -char_(' ')
            ]
            >>
            *(
                as_string
                [
                    (
                        +(
                            char_ - (char_(',') | qi::eol)
                        )
                    )
                ] [push_back(at_c<1>(_val), _1)]
                >> -char_(',')
            );

        label =
            *(
                char_ - (char_(':') | qi::eol)
            )
            >> ':';

        line =
            -label [at_c<0>(_val) = _1]
            >> -(directive | instruction) [at_c<1>(_val) = _1]
            >> qi::eol;

        root =
            +(line) [push_back(at_c<0>(_val), _1)];

        /*
         * DEBUG GARBO
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
//        */
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
    {"fakeori", 13},
    {"xori", 14},
    {"lui", 15},
    {"fakelui", 15},
    {"lb", 32},
    {"lh", 33},
    {"lw", 35},
    {"sb", 40},
    {"sh", 41},
    {"sw", 43}
};

static std::unordered_map<std::string, uint32_t> functMap = {
    {"sll", 0},
    {"srl", 2},
    {"jr", 8},
    {"syscall", 12},
    {"mfhi", 16},
    {"mflo", 18},
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
};

enum class Section {
    DATA,
    TEXT
};

class Assembler {
    std::unordered_map<std::string, size_t> labelMap;

    Section currentSection = Section::TEXT;

    const uint32_t baseTextAddr = 0x400000 / 4;

    uint32_t currentTextAddr = baseTextAddr;

    const uint32_t baseDataAddr = 0x10010000 / 4;

    uint32_t currentDataAddr = baseDataAddr;

    uint32_t ResolveLabel(const std::string& argument)
    {
        auto search = labelMap.find(argument);

        if (search != labelMap.end()) {
            return search->second * 4;
        }

        uint32_t maybeAddr = std::stoul(argument);

        return maybeAddr;
    }

    uint32_t Label2AddressRelative(const std::string& argument)
    {
        // this is an awful hack, I'm aware.

        uint32_t absolute = ResolveLabel(argument) / 4;

        return absolute - currentTextAddr - 1;
    }

    uint32_t ResolveRegister(const std::string& argument)
    {
        auto search = registerMap.find(argument);

        if (search != registerMap.end()) {
            return search->second;
        }

        if (argument[0] == '$') {
            auto numStr = argument.substr(1);
            uint32_t maybeNumber = std::stoi(numStr);

            if (maybeNumber > 32) {
                throw std::invalid_argument(numStr + " is not a valid MIPS register number.");
            }

            return maybeNumber;
        }

        throw std::invalid_argument("Couldn't resolve " + argument);
    }

    uint32_t AssembleIType(const Instruction& instruction)
    {
        if (instruction.name == "blez" || instruction.name == "bgtz") {
            uint32_t opcode = opcodeMap[instruction.name] << 26;
            uint32_t rs = ResolveRegister(instruction.arguments[0]) << 21;
            uint16_t imm = Label2AddressRelative(instruction.arguments[1]);

            return opcode | rs | imm;
        }

        if (instruction.name == "beq" || instruction.name == "bne") {
            uint32_t opcode = opcodeMap[instruction.name] << 26;
            uint32_t rs = ResolveRegister(instruction.arguments[0]) << 21;
            uint32_t rt = ResolveRegister(instruction.arguments[1]) << 16;
            uint16_t imm = Label2AddressRelative(instruction.arguments[2]);

            return opcode | rs | rt | imm;
        }

        if (instruction.name == "lui") {
            uint32_t opcode = opcodeMap[instruction.name] << 26;
            uint32_t rt = ResolveRegister(instruction.arguments[0]) << 16;
            uint16_t imm = std::stoi(instruction.arguments[1]);

            return opcode | rt | imm;
        }

        // partially resolved lui, generated by la and li
        if (instruction.name == "fakelui") {
            uint32_t opcode = opcodeMap[instruction.name] << 26;
            uint32_t rt = ResolveRegister(instruction.arguments[0]) << 16;
            uint16_t imm = (ResolveLabel(instruction.arguments[1]) >> 16);

            return opcode | rt | imm;
        }

        // see fakelui
        if (instruction.name == "fakeori") {
            uint32_t opcode = opcodeMap[instruction.name] << 26;
            uint32_t rs = ResolveRegister(instruction.arguments[0]) << 21;
            uint32_t rt = ResolveRegister(instruction.arguments[1]) << 16;
            uint16_t imm = (ResolveLabel(instruction.arguments[2]) & 0xFFFF);

            return opcode | rs | rt | imm;
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

            uint32_t rt = ResolveRegister(instruction.arguments[0]) << 16;

            // split the offset($register) argument

            auto pair = split(instruction.arguments[1]);

            uint32_t rs = ResolveRegister(pair.second) << 21;

            uint16_t imm = std::stoi(pair.first);

            return opcode | rs | rt | imm;
        }

        uint32_t rt = ResolveRegister(instruction.arguments[0]) << 16;
        uint32_t rs = ResolveRegister(instruction.arguments[1]) << 21;
        uint16_t imm = std::stoul(instruction.arguments[2]);

        return opcode | rs | rt | imm;
    }

    uint32_t AssembleRType(const Instruction& instruction)
    {
        if (instruction.name == "sll" ||
                instruction.name == "srl" ||
                instruction.name == "sra")
        {
            uint32_t opcode = 0;

            uint32_t rd = ResolveRegister(instruction.arguments[0]) << 11;
            uint32_t rt = ResolveRegister(instruction.arguments[1]) << 16;

            uint32_t shamt = std::stoi(instruction.arguments[2]) << 6;

            uint32_t funct = functMap[instruction.name];

            return opcode | rd | rt | shamt | funct;
        }

        if (instruction.name == "jr") {
            uint32_t opcode = 0;

            uint32_t rs = ResolveRegister(instruction.arguments[0]) << 21;

            uint32_t funct = functMap[instruction.name];

            return opcode | rs | funct;
        }

        if (instruction.name == "jalr") {
            uint32_t opcode = 0;

            uint32_t rd = 31 << 11;

            if (instruction.arguments.size() > 1) {
                rd = ResolveRegister(instruction.arguments[0]) << 11;
            }

            uint32_t rs = ResolveRegister(instruction.arguments[0]) << 21;

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
            uint32_t rd = ResolveRegister(instruction.arguments[0]) << 11;
            uint32_t funct = functMap[instruction.name];

            return opcode | rd | funct;
        }

        if (instruction.name == "mult" ||
                instruction.name == "multu" ||
                instruction.name == "div" ||
                instruction.name == "divu")
        {
            uint32_t opcode = 0;

            uint32_t rs = ResolveRegister(instruction.arguments[0]) << 21;
            uint32_t rt = ResolveRegister(instruction.arguments[1]) << 16;

            uint32_t funct = functMap[instruction.name];

            return opcode | rs | rt | funct;
        }

        uint32_t opcode = 0;
        uint32_t rd = ResolveRegister(instruction.arguments[0]) << 11;
        uint32_t rs = ResolveRegister(instruction.arguments[1]) << 21;
        uint32_t rt = ResolveRegister(instruction.arguments[2]) << 16;

        uint32_t funct = functMap[instruction.name];

        return opcode | rs | rt | rd | funct;
    }

    uint32_t AssembleJType(const Instruction& instruction)
    {
        uint32_t opcode = (instruction.name == "j" ? 2 : 3) << 26;

        uint32_t labelAddr = ResolveLabel(instruction.arguments[0]);

        return opcode | (labelAddr >> 2);
    }

    uint32_t DispatchInstruction(const Instruction& instruction)
    {
        if (opcodeMap.find(instruction.name) != opcodeMap.end()) {
            return AssembleIType(instruction);
        }

        if (functMap.find(instruction.name) != functMap.end()) {
            return AssembleRType(instruction);
        }

        if (instruction.name == "j" || instruction.name == "jal") {
            return AssembleJType(instruction);
        }

        throw std::invalid_argument(instruction.name + " is not supported by this assembler");
    }

    bool MaybeSwitchSection(const Directive& directive)
    {
        if (directive.name == "data") {
            currentSection = Section::DATA;
            return true;
        }

        if (directive.name == "text") {
            currentSection = Section::TEXT;
            return true;
        }

        return false;
    }

    std::vector<uint32_t> DispatchDirective(const Directive& directive)
    {
        if (MaybeSwitchSection(directive)) {
            return std::vector<uint32_t>();
        }

        if (directive.name == "space") {
            if (currentSection == Section::TEXT) {
                throw std::invalid_argument("Can't use space while in text section");
            }

            uint32_t bytes = std::stoi(directive.argument);

            return std::vector<uint32_t>(bytes / 4 + 1, 1);
        }

        throw std::invalid_argument("Directive " + directive.name + " is not supported by this assembler");
    }

    void AppendMem(std::unordered_map<uint32_t, uint32_t>& memory, const std::vector<uint32_t>& words)
    {
        // this not the best thing, because I'm padding where I don't really need to
        // but all of this is a giant hack anyway
        if (currentSection == Section::DATA) {
            for (std::size_t i = 0; i < words.size(); ++i, ++currentDataAddr) {
                memory[currentDataAddr] = words[i];
            }
        }
    }

    boost::optional<std::list<Line>> TranslatePseudoInstruction(const Instruction& ins, const boost::optional<Label> label)
    {
        if (ins.name == "move") {
            Instruction addiu = {
                "addu",
                {
                    ins.arguments[0],
                    "$zero",
                    ins.arguments[1]
                }
            };

            Line l = {label, addiu};

            return std::list<Line>{l};
        }

        if (ins.name == "nop") {
            Instruction sll = {
                "sll",
                {
                    "$0",
                    "$0",
                    "0"
                }
            };

            Line l = {label, sll};

            return std::list<Line>{l};
        }

        if (ins.name == "bge") {
            Instruction slt = {
                "slt",
                {
                    "$at",
                    ins.arguments[0],
                    ins.arguments[1]
                }
            };

            Instruction beq = {
                "beq",
                {
                    "$at",
                    "$zero",
                    ins.arguments[2]
                }
            };

            return std::list<Line>{{label, slt}, {boost::optional<Label>(), beq}};
        }

        if (ins.name == "ble") {
            Instruction slt = {
                "slt",
                {
                    "$at",
                    ins.arguments[1],
                    ins.arguments[0]
                }
            };

            Instruction beq = {
                "beq",
                {
                    "$at",
                    "$zero",
                    ins.arguments[2]
                }
            };

            return std::list<Line>{{label, slt}, {boost::optional<Label>(), beq}};
        }

        if (ins.name == "li") {
            // no support for li with large numbers because I'm lazy
            Instruction addiu = {
                "addiu",
                {
                    ins.arguments[0],
                    "$0",
                    ins.arguments[1]
                }
            };

            return std::list<Line>{{label, addiu}};
        }

        if (ins.name == "la") {
            // I should be incarcerated for this

            Instruction lui = {
                "fakelui",
                {
                    ins.arguments[0],
                    ins.arguments[1]
                }
            };

            Instruction ori = {
                "fakeori",
                {
                    ins.arguments[0],
                    ins.arguments[0],
                    ins.arguments[1]
                }
            };

            return std::list<Line>{{label, lui}, {boost::optional<Label>(), ori}};
        }

        return boost::optional<std::list<Line>>();
    }

public:
    std::unordered_map<uint32_t, uint32_t> Assemble(std::string assembly)
    {
        using grammar = AsmGrammar<std::string::iterator>;
        using Skipper = skipper<std::string::iterator>;

        MipsAsm asm_;

        grammar g;
        Skipper sp;

        auto res = qi::phrase_parse(assembly.begin(), assembly.end(), g, sp, asm_);

        if (!res) {
            throw std::invalid_argument("Unable to parse the input.");
        }

        for (auto it = asm_.lines.begin(); it != asm_.lines.end();) {
            if (const Instruction* ins = boost::get<Instruction>(&it->IoD)) {
                auto maybeTranslated = TranslatePseudoInstruction(*ins, it->label);
                if (maybeTranslated) {
                    // insert after current it

                    auto next = it;
                    ++next;

                    asm_.lines.splice(next, maybeTranslated.get());
                    asm_.lines.erase(it);
                    it = next;
                    continue;
                }
            }
            it++;
        }

        for (const auto& line : asm_.lines) {
            bool switched = false;
            if (const Directive* dir = boost::get<Directive>(&line.IoD)) {
                switched = MaybeSwitchSection(*dir);
            }

            if (line.label) {
                labelMap[line.label.get().name] = currentSection == Section::DATA ? currentDataAddr : currentTextAddr;
            }

            if (line.IoD.which() != 0 && !switched) {
                if (currentSection == Section::DATA) {
                    currentDataAddr++;
                } else if (currentSection == Section::TEXT) {
                    currentTextAddr++;
                }
            }
        }


        currentTextAddr = baseTextAddr;
        currentDataAddr = baseDataAddr;

        auto output = std::unordered_map<uint32_t, uint32_t>();
        for (const auto& line : asm_.lines) {
            if (const Directive* dir = boost::get<Directive>(&line.IoD)) {
                AppendMem(output, DispatchDirective(*dir));
            } else if (const Instruction* ins = boost::get<Instruction>(&line.IoD)) {
                output[currentTextAddr] = DispatchInstruction(*ins);
                currentTextAddr++;
            }
        }

        return output;
    }
};

std::unordered_map<uint32_t, uint32_t> Assemble(std::string assembly)
{
    // shitty wrapper function because I'm lazy

    Assembler asmer;

    return asmer.Assemble(assembly);
}
