#ifndef DEFINES_H
#define DEFINES_H

#include <unordered_set>

namespace urcl::defines {
    using description = std::string;

    enum op_type {
        imm,
        reg,
        basicval,
        val,
        array,
        port,
        comparison,
        inst
    };

    const std::unordered_set<std::string> HEADERS = {
        "BITS", "MINREG", "MINHEAP", "MINSTACK", "RUN"
    };

    const std::unordered_set<std::string> CORE_INSTRUCTIONS = {
        "IMM", "ADD", "NOR", "RSH", "LOD", "STR", "BGE"
    };

    const std::unordered_set<std::string> BASIC_INSTRUCTIONS = {
        "IMM", "ADD", "NOR", "RSH", "LOD", "STR", "BGE", "MOV", "SUB", "NEG", "DEC", "INC", "NOT", "AND", "NAND", "OR", "XOR", "XNOR", "LSH", "CPY", "JMP", "BRE", "BNE", "BLE", "BRL", "BRG", "BRP", "BRN", "BRZ", "BNZ", "BOD", "BEV", "BRC", "BNC", "NOP", "HLT"
    };

    const std::unordered_set<std::string> STACK_INSTRUCTIONS = {
        "PSH", "POP", "CAL", "RET" // basic
    };

    const std::unordered_set<std::string> COMPLEX_INSTRUCTIONS = {
        "MLT", "UMLT", "SUMLT", "DIV", "SDIV", "MOD", "ABS", "SRS", "BSR", "BSS", "BSL", "LLOD", "LSTR", "SBLE", "SBRL", "SBGE", "SBRG", "SETE", "SETNE", "SETLE", "SSETLE", "SETL", "SSETL", "SETGE", "SSETGE", "SETG", "SSETG", "SETC", "SETNC"
    };

    const std::unordered_set<std::string> OTHER_INSTRUCTIONS = {
        "DW", // data
        "IN", "OUT" // i/o
    };

    const std::unordered_set<std::string> IRIS_INSTRUCTIONS = {
        "FTOI", "ITOF", "FADD", "FSUB", "FMLT", "FDIV", "FABS", "FSQRT", // floats
        "HPSH", "HPOP", "HCAL", "HRET", // hardware stack
        "RW" // ROM
    };

    const std::unordered_set<std::string> URCX_INSTRUCTIONS = {
        "__ASSERT", "__ASSERT0", "__ASSERT_EQ", "__ASSERT_NEQ"
    };

    const std::unordered_set<std::string> IRIX_INSTRUCTIONS = {
        "HSAV", "HRSR"
    };

    const std::unordered_set<std::string> URCX_MACROS = {
        "@DEBUG", "@ASSERT", "@ASSERT0", "@ASSERT_EQ", "@ASSERT_NEQ"
    };

    const std::unordered_set<std::string> RUN_MODES = {
        "ROM", "RAM"
    };

    const std::unordered_set<std::string> DEBUG_MODES = {
        "ONREAD", "ONWRITE"
    };

    const std::unordered_set<std::string> STD_PORTS = {
        "CPUBUS", "TEXT", "NUMB", "SUPPORTED", "SPECIAL", "PROFILE",
        "X", "Y", "COLOR", "BUFFER", "G_SPECIAL",
        "ASCII", "CHAR5", "CHAR6", "ASCII7", "UTF8", "UTF16", "UTF32", "T_SPECIAL",
        "INT", "UINT", "BIN", "HEX", "FLOAT", "FIXED", "N_SPECIAL",
        "ADDR", "BUS", "PAGE", "S_SPECIAL",
        "RNG", "NOTE", "INSTR", "NLEG", "WAIT", "NADDR", "DATA", "M_SPECIAL",
        "UD1", "UD2", "UD3", "UD4", "UD5", "UD6", "UD7", "UD8",
        "UD9", "UD10", "UD11", "UD12", "UD13", "UD14", "UD15", "UD16"
    };

    const std::unordered_set<std::string> URCX_PORTS = {
        "GAMEPAD", "AXIS", "GAMEPAD_INFO", "KEY",
        "MOUSE_X", "MOUSE_Y", "MOUSE_DX", "MOUSE_DY", "MOUSE_DWHEEL", "MOUSE_BUTTONS",
        "FILE", "DBG_INT", "BENCHMARK", "TIME", "FAIL_ASSERT"
    };

    const std::unordered_set<std::string> IRIS_PORTS = {
        "CLEAR_SCREEN", "TOGGLE_BUFFER", "COLORX", "COLORY", "TIMER", "TIMER_RESET",
        "X1", "Y1", "X2", "Y2", "LINE", "TILE"
    };

    const std::unordered_set<std::string> STD_CONSTS = {
        "BITS", "HEAP", "MSB", "MAX", "SMSB", "SMAX", "UHALF", "LHALF", 
        "MINREG", "MINHEAP", "MINSTACK"
    };

    const std::unordered_set<std::string> URCX_CONSTS = {
        "A", "B", "SELECT", "START", "LEFT", "RIGHT", "UP", "DOWN",
        "Y", "X", "LB", "RB", "LEFT2", "RIGHT2", "UP2", "DOWN2",
        "RT", "LT",
        "LEFT_X", "LEFT_Y", "RIGHT_X", "RIGHT_Y" // controller axis
    };

    const std::unordered_map<std::string, std::pair<description, std::vector<op_type>>> INST_INFO = {
        {
            "IMM", {
                "Immediate (2 Operands) (core)\\\nLoads an immediate value\\\nOp1 = Op2",
                {op_type::reg, op_type::imm}
            }
        },{
            "ADD", {
                "Add (3 Operands) (core/basic)\\\nPerforms an integer addition\\\nOp1 = Op2 + Op3",
                {op_type::reg, op_type::basicval, op_type::basicval}
            }
        },{
            "NOR", {
                "Bitwise NOR (3 Operands) (core/basic)\\\nPerforms a bitwise NOR operation\\\nOp1 = ~(Op2 | Op3)",
                {op_type::reg, op_type::basicval, op_type::basicval}
            }
        },{
            "RSH", {
                "Right Shift (2 Operands) (core/basic)\\\nPerforms a one bit logical right shift operation\\\nOp1 = Op2 >> 1",
                {op_type::reg, op_type::basicval}
            }
        },{
            "LOD", {
                "Load (2 Operands) (core/basic)\\\nLoads a value from memory\\\nOp1 = mem[Op2]",
                {op_type::reg, op_type::basicval}
            }
        },{
            "STR", {
                "Store (2 Operands) (core/basic)\\\nStores a value into memory\\\nmem[Op1] = Op2",
                {op_type::basicval, op_type::basicval}
            }
        },{
            "BGE", {
                "Branch if Greater than or Equal to (3 Operands) (core/basic)\\\nPerforms a branch if Op3 is greater than or equal to Op2 (unsigned)\\\vif (Op2 >= Op3) goto Op1 (unsigned)",
                {op_type::basicval, op_type::basicval, op_type::basicval}
            }
        },{
            "MOV", {
                "Move (2 Operands) (basic)\\\nCopies a value between operands\\\nOp1 = Op2",
                {op_type::reg, op_type::val}
            }
        },{
            "SUB", {
                "Subtract (3 Operands) (basic)\\\nPerforms a subtraction\\\nOp1 = Op2 - Op3",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "NEG", {
                "Negate (2 Operands) (basic)\\\nPerforms a negation\\\nOp1 = -Op2",
                {op_type::reg, op_type::val}
            }
        },{
            "DEC", {
                "Decrement (2 Operands) (basic)\\\nSubtracts one\\\nOp1 = Op2 - 1",
                {op_type::reg, op_type::val}
            }
        },{
            "INC", {
                "Increment (2 Operands) (basic)\\\nPerforms an incrementation\\\nOp1 = Op2 + 1",
                {op_type::reg, op_type::val}
            }
        },{
            "NOT", {
                "Bitwise NOT (2 Operands) (basic)\\\nPerforms a bitwise NOT operation\\\nOp1 = ~Op2",
                {op_type::reg, op_type::val}
            }
        },{
            "AND", {
                "Bitwise AND (3 Operands) (basic)\\\nPerforms a bitwise AND operation\\\nOp1 = Op2 & Op3",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "NAND",  {
                "Bitwise NAND (3 Operands) (basic)\\\nPerforms a bitwise NAND operation\\\nOp1 = ~(Op2 & Op3)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "OR", {
                "Bitwise OR (3 Operands) (basic)\\\nPerforms a bitwise OR operation\\\nOp1 = Op2 | Op3",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "XOR", {
                "Bitwise XOR (3 Operands) (basic)\\\nPerforms a bitwise XOR operation\\\nOp1 = Op2 ^ Op3",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "XNOR", {
                "Bitwise XNOR (3 Operands) (basic)\\\nPerforms a bitwise XNOR operation\\\nOp1 = ~(Op2 ^ Op3)",
                {op_type::reg, op_type::val, op_type::val}
            
            }
        },{
            "LSH", {
                "Left Shift (2 Operands) (basic)\\\nPerforms a one bit logical left shift operation\\\nOp1 = Op2 << 1",
                {op_type::reg, op_type::val}
            }
        },{
            "CPY", {
                "Copy (3 Operands) (basic)\\\nCopies a value between the memory pointed to by operands\\\nmem[Op1] = mem[Op2]",
                {op_type::val, op_type::val}
            }
        },{
            "JMP", {
                "Jump (1 Operand) (basic)\\\nPerforms an unconditional jump\\\ngoto Op1",
                {op_type::val}
            }
        },{
            "BRE", {
                "Branch if Equal to (3 Operands) (basic)\\\nPerforms a branch if equal\\\nif (Op2 == Op3) goto Op1",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "BNE", {
                "Branch if Not Equal to (3 Operands) (basic)\\\nPerforms a branch if not equal\\\nif (Op2 != Op3) goto Op1",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "BLE", {
                "Branch if Less than or Equal to (3 Operands) (basic)\\\nPerforms a branch if Op2 is less than or equal to Op3 (unsigned)\\\nif (Op2 <= Op3) goto Op1 (unsigned)",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "BRL", {
                "Branch if Less than (3 Operands) (basic)\\\nPerforms a branch if Op2 is less than Op3 (unsigned)\\\nif (Op2 < Op3) goto Op1 (unsigned)",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "BRG", {
                "Branch if Greater than (3 Operands) (basic)\\\nPerforms a branch if Op2 greater than Op3 (unsigned)\\\nif (Op2 > Op3) goto Op1 (unsigned)",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "BRP", {
                "Branch if Positive (2 Operands) (basic)\\\nPerforms a branch if positive (or zero) (signed)\\\nif (Op2 >= 0) goto Op1 (signed)",
                {op_type::val, op_type::val}
            }
        },{
            "BRN", {
                "Branch if Negative (2 Operands) (basic)\\\nPerforms a branch if negative\\\nif (Op2 < 0) goto Op1 (signed)",
                {op_type::val, op_type::val}
            }
        },{
            "BRZ", {
                "Branch if Zero (2 Operands) (basic)\\\nPerforms a branch if zero\\\nif (Op2 == 0) goto Op1",
                {op_type::val, op_type::val}
            }
        },{
            "BNZ", {
                "Branch if Not Zero (2 Operands) (basic)\\\nPerforms a branch if not zero\\\nif (Op2 != 0) goto Op1",
                {op_type::val, op_type::val}
            }
        },{
            "BOD", {
                "Branch if Odd (2 Operands) (basic)\\\nPerforms a branch if odd\\\nif (Op2 & 1) goto Op1",
                {op_type::val, op_type::val}
            }
        },{
            "BEV", {
                "Branch if Even (2 Operands) (basic)\\\nPerforms a branch if even\\\nif (!(Op & 1)) goto Op1",
                {op_type::val, op_type::val}
            }
        },{
            "BRC", {
                "Branch if Carry (3 Operands) (basic)\\\nPerforms a branch if Op2 added to Op3 results in a carry (unsigned)\\\nif (Op2 + Op3 < Op2) goto Op1 (unsigned)",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "BNC", {
                "Branch if Not Carry (3 Operands) (basic)\\\nPerforms a branch if Op2 added to Op2 does not result in a carry (unsigned)\\\nif (Op2 + Op3 >= Op2) goto Op1 (unsigned)",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "NOP", {
                "No Operation (0 Operands) (basic)\\\nDoes nothing",
                {}
            }
        },{
            "HLT", {
                "Halt (0 Operands) (basic)\\\nStops execution of the program",
                {}
            }
        },{
            "PSH", {
                "Push (1 Operand) (basic)\\\nPushes a value to the stack\\\nstack[--sp] = Op1",
                {op_type::val}
            }
        },{
            "POP", {
                "Pop (1 Operand) (basic)\\\nPops a value from the stack\\\nOp1 = stack[sp++]",
                {op_type::reg}
            }
        },{
            "CAL", {
                "Call (1 Operand) (basic)\\\nPerforms a call, pushing the return address to the stack\\\nstack[--sp] = ~+1; goto Op1",
                {op_type::val}
            }
        },{
            "RET", {
                "Return (0 Operands) (basic)\\\nReturns from a previous call\\\ngoto stack[sp++]",
                {}
            }
        },{
            "MLT", {
                "Multiply (3 Operands) (complex)\\\nPerforms an integer multiplication, returning the lower part\\\nOp1 = Op2 * Op3",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "UMLT", {
                "Upper Multiply (3 Operands) (complex)\\\nPerforms an integer multiplication, returning the upper part (unsigned)\\\nOp1 = (Op2 * Op3) >> @BITS (unsigned)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SUMLT", {
                "Signed Upper Multiply (3 Operands) (complex)\\\nPerforms an integer multiplication, returning the upper part (signed)\\\nOp1 = (Op2 * Op3) >> @BITS (signed)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "DIV", {
                "Divide (3 Operands) (complex)\\\nPerforms an integer division (unsigned)\\\nOp1 = Op2 / Op3 (unsigned)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SDIV", {
                "Signed Divide (3 Operands) (complex)\\\nPerforms an integer division (signed)\\\nOp1 = Op2 / Op3 (signed)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "MOD", {
                "Modulo (3 Operands) (complex)\\\nPerforms an integer modulo operation (unsigned)\\\nOp1 = Op2 % Op3 (unsigned)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "ABS", {
                "Absolute Value (2 Operands) (complex)\\\nPerforms an integer absolute value operation\\\nOp1 = abs(Op2)",
                {op_type::reg, op_type::val}
            }
        },{
            "SRS", {
                "Signed Right Shift (2 Operands) (complex)\\\nPerforms a one bit arithmetic right shift (signed)\\\nOp1 = Op2 >> 1 (signed)",
                {op_type::reg, op_type::val}
            }
        },{
            "BSR", {
                "Barrel Shift Right (3 Operands) (complex)\\\nPerforms a logical barrel shift right (unsigned)\\\nOp1 = Op2 >> Op3 (unsigned)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "BSS", {
                "Barrel Shift Signed (3 Operands) (complex)\\\nPerforms an arithmetic barrel shift right (signed)\\\nOp1 = Op2 >> Op3 (signed)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "BSL", {
                "Barrel Shift Left (3 Operands) (complex)\\\nPerforms a barrel shift left\\\nOp1 = Op2 << Op3",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "LLOD", {
                "List Load (3 Operands) (complex)\\\nLoads a value from memory with offset\\\nOp1 = mem[Op2 + Op3]",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "LSTR", {
                "List Store (3 Operands) (complex)\\\nStores a value to memory with offset\\\nmem[Op1 + Op2] = Op3",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "SBLE", {
                "Signed Branch if Less than or Equal to (3 Operands) (complex)\\\nPerforms a branch if Op2 is less than or equal to Op3 (signed)\\\nif (Op2 <= Op3) goto Op1 (signed)",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "SBRL", {
                "Signed Branch if Less than (3 Operands) (complex)\\\nPerforms a branch if Op2 is less than Op3 (signed)\\\nif (Op2 < Op3) goto Op1 (signed)",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "SBGE", {
                "Signed Branch if Greater than or Equal to (3 Operands) (complex)\\\nPerforms a branch if Op2 is greater than or equal to Op3 (signed)\\\nif (Op2 >= Op3) goto Op1 (signed)",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "SBRG", {
                "Signed Branch if Greater than (3 Operands) (complex)\\\nPerforms a branch if Op2 is greater than Op3 (signed)\\\nif (Op2 > Op3) goto Op1 (signed)",
                {op_type::val, op_type::val, op_type::val}
            }
        },{
            "SETE", {
                "Set if Equal to (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 is equal to Op3, and sets Op1 to 0 if not equal\\\nOp1 = (Op2 == Op3) ? -1 : 0",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SETNE", {
                "Set if Not Equal to (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 is not equal to Op3, and sets Op1 to 0 if equal\\\nOp1 = (Op2 != Op3) ? -1 : 0",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SETLE", {
                "Set if Less than or Equal to (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 is less than or equal to Op3, and sets Op1 to 0 if not (unsigned)\\\nOp1 = (Op2 <= Op3) ? -1 : 0 (unsigned)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SSETLE", {
                "Signed Set if Less than or Equal to (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 is less than or equal to Op3, and sets Op1 to 0 if not (signed)\\\nOp1 = (Op2 <= Op3) ? -1 : 0 (signed)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SETL", {
                "Set if Less than (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 is less than Op3, and sets Op1 to 0 if not (unsigned)\\\nOp1 = (Op2 < Op3) ? -1 : 0 (unsigned)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SSETL", {
                "Signed Set if Less than (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 is less than Op3, and sets Op1 to 0 if not (signed)\\\nOp1 = (Op2 < Op3) ? -1 : 0 (signed)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SETGE", {
                "Set if Greater than or Equal to (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 is greater than or equal to Op3, and sets Op1 to 0 if not (unsigned)\\\nOp1 = (Op2 >= Op3) ? -1 : 0 (unsigned)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SSETGE", {
                "Signed Set if Greater than or Equal to (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 is greater than or equal to Op3, and sets Op1 to 0 if not (signed)\\\nOp1 = (Op2 >= Op3) ? -1 : 0 (signed)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SETG", {
                "Set if Greater than (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 is greater than Op3, and sets Op1 to 0 if not (unsigned)\\\nOp1 = (Op2 > Op3) ? -1 : 0 (unsigned)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SSETG", {
                "Signed Set if Greater than (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 is greater than Op3, and sets Op1 to 0 if not (signed)\\\nOp1 = (Op2 > Op3) ? -1 : 0 (signed)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SETC", {
                "Set if Carry (3 Operands) (complex)\\\nSets Op1 to -1 if Op2 added to Op3 results in a carry, and sets Op1 to 0 if not (unsigned)\\\nOp1 = (Op2 + Op3 < Op2) ? -1 : 0",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "SETNC", {
                "Set if Not Carry (3 Operands) (complex)\\\nSets Op1 to 0 if Op2 added to Op3 results in a carry, and sets Op1 to -1 if not (unsigned)\\\nOp1 = (Op2 + Op3 <= Op2) ? -1 : 0",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "DW", {
                "Define Word (1 Operand) (data)\\\nInitializes data in memory prior to runtime",
                {op_type::array}
            }
        },{
            "IN", {
                "Input (1/2 Operand(s)) (I/O)\\\nReads data from an I/O device",
                {op_type::reg, op_type::port}
            }
        },{
            "OUT", {
                "Output (1/2 Operand(s)) (I/O)\\\nWrites data to an I/O device",
                {op_type::port, op_type::val}
            }
        },{
            "FTOI", {
                "Float to Integer (2 Operands) (iris)\\\nPerforms a conversion from a float to an integer (signed, rounded)\\\nOp1 = (int)Op2",
                {op_type::reg, op_type::val}
            }
        },{
            "ITOF", {
                "Integer to Float (2 Operands) (iris)\\\nPerforms a conversion from an integer to a float (signed)\\\nOp1 = (float)Op2",
                {op_type::reg, op_type::val}
            }
        },{
            "FADD", {
                "Floating Point Add (3 Operands) (iris)\\\nPerforms a floating point addition\\\nOp1 = Op2 + Op3 (float)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "FSUB", {
                "Floating Point Subtract (3 Operands) (iris)\\\nPerforms a floating point subtraction\\\nOp1 = Op2 - Op3 (float)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "FMLT", {
                "Floating Point Multiply (3 Operands) (iris)\\\nPerforms a floating point multiplication\\\nOp1 = Op2 * Op3 (float)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "FDIV", {
                "Floating Point Divide (3 Operands) (iris)\\\nPerforms a floating point division\\\nOp1 = Op2 / Op3 (float)",
                {op_type::reg, op_type::val, op_type::val}
            }
        },{
            "FABS", {
                "Floating Point Absolute Value (2 Operands) (iris)\\\nPerforms a floating point absolute value operation\\\nOp1 = fabs(Op2)",
                {op_type::reg, op_type::val}
            }
        },{
            "FSQRT", {
                "Floating Point Square Root (2 Operands) (iris)\\\nPerforms a floating point square root operation\\\nOp1 = sqrt(Op2)",
                {op_type::reg, op_type::val}
            }
        },{
            "HPSH", {
                "Hardware Push (1 Operand) (iris)\\\nPushes a value to the hardware data stack\\\ndstack[--dsp] = Op1",
                {op_type::val}
            }
        },{
            "HPOP", {
                "Hardware Pop (1 Operand) (iris)\\\nPops a value from the hardware data stack\\\nOp1 = dstack[dsp++]",
                {op_type::reg}
            }
        },{
            "HCAL", {
                "Hardware Call (1 Operand) (iris)\\\nPerforms a call using the hardware call stack\\\ncstack[--csp] = ~+1; goto Op1",
                {op_type::val}
            }
        },{
            "HRET", {
                "Hardware Return (1 Operand) (iris)\\\nReturns from a call using the hardware call stack\\\ngoto cstack[csp++]",
                {}
            }
        },{
            "RW", {
                "Read Only Word (1 Operand) (iris)\\\nInitializes read only data into ROM prior to runtime",
                {op_type::array}
            }
        },{
            "__ASSERT", {
                "Assert (1 Operand) (urcx)\\\nSends an error message if Op1 is zero",
                {op_type::val}
            }
        },{
            "__ASSERT0", {
                "Assert Zero (1 Operand) (urcx)\\\nSends an error message if Op1 is not zero",
                {op_type::val}
            }
        },{
            "__ASSERT_EQ", {
                "Assert Equal (2 Operands) (urcx)\\\nSends an error message if Op1 and Op2 are not equal",
                {op_type::val, op_type::val}
            }
        },{
            "__ASSERT_NEQ", {
                "Assert Not Equal (2 Operands) (urcx)\\\nSends an error message if Op1 and Op2 are equal",
                {op_type::val, op_type::val}
            }
        },{
            "HSAV", {
                "Hardware Save (1 Operand) (irix)\\\nPushes a value to the hardware save stack\\\nsstack[--ssp] = Op1",
                {op_type::val}
            }
        },{
            "HRSR", {
                "Hardware Restore (1 Operand) (irix)\\\nPops a value from the hardware save stack\\\nOp1 = sstack[ssp++]",
                {op_type::reg}
            }
        },{
            "BITS", {
                "Bit Width (1/2 Operands) (header)\\\nSpecifies the number of bits the program expects the processor to have",
                {op_type::comparison, op_type::imm}
            }
        },{
            "MINREG", {
                "Minimum Registers (1 Operand) (header)\\\nSpecifies the number of registers used by the program",
                {op_type::imm}
            }
        },{
            "MINHEAP", {
                "Minimum Heap (1 Operand) (header)\\\nSpecifies the minimum heap size necessary to run the program",
                {op_type::imm}
            }
        },{
            "MINSTACK", {
                "Minimum Stack (1 Operand) (header)\\\nSpecifies the minimum stack size necessary to run the program",
                {op_type::imm}
            }
        },{
            "RUN", {
                "Run (1 Operand) (header)\\\nSpecifies whether the program can be located in a read only memory space",
                {op_type::inst}
            }
        },{
            "RAM", {
                "Random Access Memory (run mode)\\\nSpecifies that the program must be located in a space writeable by the program",
                {}
            }
        },{
            "ROM", {
                "Read Only Memory (run mode)\\\nSpecifies that the program should be located in a space not writeable by the program",
                {}
            }
        },{
            "@DEFINE", {
                "Define (2 Operands) (macro)\\\nDefines a new constant\\\nOp1 = Op2",
                {op_type::val, op_type::val}
            }
        },{
            "@DEBUG", {
                "Debug (0/1/2 Operands) (macro) (urcx)\\\nPauses execution on certain conditions",
                {}
            }
        },{
            "@ASSERT", {
                "Assert (1 Operand) (macro) (urcx)\\\nSends an error message if Op1 is zero",
                {op_type::val}
            }
        },{
            "@ASSERT0", {
                "Assert Zero (1 Operand) (macro) (urcx)\\\nSends an error message if Op1 is not zero",
                {op_type::val}
            }
        },{
            "@ASSERT_EQ", {
                "Assert Equal (2 Operands) (macro) (urcx)\\\nSends an error message if Op1 and Op2 are not equal",
                {op_type::val, op_type::val}
            }
        },{
            "@ASSERT_NEQ", {
                "Assert Not Equal (2 Operands) (macro) (urcx)\\\nSends an error message if Op1 and Op2 are equal",
                {op_type::val, op_type::val}
            }
        },{
            "ONREAD", {
                "On Read (debug mode) (urcx)\\\nPauses execution whenever the following operand is read from",
                {op_type::val}
            }
        },{
            "ONWRITE", {
                "On Write (debug mode) (urcx)\\\nPauses execution whenever the following operand is written to",
                {op_type::val}
            }
        }
    };

    const std::vector<op_type> IN_DEFAULT = {op_type::port, op_type::reg};

    const std::unordered_map<std::string, std::pair<description, std::vector<op_type>>> IN_INFO = {
        {
            "%TEXT", {
                "Read character input from a terminal device",
                {op_type::port, op_type::reg}
            }
        },{
            "%NUMB", {
                "Read character input from a terminal device",
                {op_type::port, op_type::reg}
            }
        },{
            "%SUPPORTED", {
                "Return supported state of port specified by OUT%SUPPORTED",
                {op_type::port, op_type::reg}
            }
        },{
            "%PROFILE", {
                "Return current profile",
                {op_type::port, op_type::reg}
            }
        },{
            "%X", {
                "Return display size in pixels in x dimension",
                {op_type::port, op_type::reg}
            }
        },{
            "%Y", {
                "Return display size in pixels in y dimension",
                {op_type::port, op_type::reg}
            }
        },{
            "%COLOR", {
                "Read pixel color on display (coordinates specified by %X and %Y)",
                {op_type::port, op_type::reg}
            }
        },{
            "%BUFFER", {
                "Read current buffer state",
                {op_type::port, op_type::reg}
            }
        },{
            "%ASCII", {
                "Read ASCII character from terminal",
                {op_type::port, op_type::reg}
            }
        },{
            "%CHAR5", {
                "Read 5 bit character from terminal",
                {op_type::port, op_type::reg}
            }
        },{
            "%CHAR6", {
                "Read 6 bit character from terminal",
                {op_type::port, op_type::reg}
            }
        },{
            "%ASCII7", {
                "Read 7-bit ASCII character from terminal",
                {op_type::port, op_type::reg}
            }
        },{
            "%UTF8", {
                "Read UTF-8 byte from terminal",
                {op_type::port, op_type::reg}
            }
        },{
            "%UTF16", {
                "Read UTF-16 word from terminal",
                {op_type::port, op_type::reg}
            }
        },{
            "%UTF32", {
                "Read UTF-32 codepoint from terminal",
                {op_type::port, op_type::reg}
            }
        },{
            "%INT", {
                "Read signed integer from numeric I/O",
                {op_type::port, op_type::reg}
            }
        },{
            "%UINT", {
                "Read unsigned integer from numeric I/O",
                {op_type::port, op_type::reg}
            }
        },{
            "%BIN", {
                "Read binary integer from numeric I/O",
                {op_type::port, op_type::reg}
            }
        },{
            "%Hex", {
                "Read hexadecimal integer from numeric I/O",
                {op_type::port, op_type::reg}
            }
        },{
            "%FLOAT", {
                "Read floating point number from numeric I/O",
                {op_type::port, op_type::reg}
            }
        },{
            "%FIXED", {
                "Read fixed point number from numeric I/O",
                {op_type::port, op_type::reg}
            }
        },{
            "%ADDR", {
                "Read size of current page on storage device",
                {op_type::port, op_type::reg}
            }
        },{
            "%BUS", {
                "Read word of data from storage device (address specified by %ADDR)",
                {op_type::port, op_type::reg}
            }
        },{
            "%PAGE", {
                "Return number of pages on storage device",
                {op_type::port, op_type::reg}
            }
        },{
            "%RNG", {
                "Return random number",
                {op_type::port, op_type::reg}
            }
        },{
            "%NOTE", {
                "Read currently set pitch on audio device",
                {op_type::port, op_type::reg}
            }
        },{
            "%INSTR", {
                "Read currently set instrument on audio device",
                {op_type::port, op_type::reg}
            }
        },{
            "%NLEG", {
                "Read signed integer from numeric I/O",
                {op_type::port, op_type::reg}
            }
        },{
            "%WAIT", {
                "Wait until timer runs out",
                {op_type::port, op_type::reg}
            }
        },{
            "%NADDR", {
                "Read current network address",
                {op_type::port, op_type::reg}
            }
        },{
            "%DATA", {
                "Read data from network",
                {op_type::port, op_type::reg}
            }
        },
    };

    const std::unordered_map<std::string, std::pair<description, std::vector<op_type>>> OUT_INFO = {
        {
            "%TEXT", {
                "Write character output to a terminal device",
                {op_type::port, op_type::val}
            }
        },{
            "%NUMB", {
                "Write numeric input to a numeric output device",
                {op_type::port, op_type::val}
            }
        },{
            "%SUPPORTED", {
                "Set the port number to query support for",
                {op_type::port, op_type::val}
            }
        },{
            "%PROFILE", {
                "Set the device profile",
                {op_type::port, op_type::val}
            }
        },{
            "%X", {
                "Set the x coordinate of display I/O",
                {op_type::port, op_type::val}
            }
        },{
            "%Y", {
                "Set the y coordinate of display I/O",
                {op_type::port, op_type::val}
            }
        },{
            "%COLOR", {
                "Set the color of a pixel of the display (coordinates specified by %X and %Y)",
                {op_type::port, op_type::val}
            }
        },{
            "%BUFFER", {
                "Set the display buffer state",
                {op_type::port, op_type::val}
            }
        },{
            "%ASCII", {
                "Write character output to an ASCII terminal device",
                {op_type::port, op_type::val}
            }
        },{
            "%CHAR5", {
                "Write character output to a terminal device utilizing 5 bit characters",
                {op_type::port, op_type::val}
            }
        },{
            "%CHAR6", {
                "Write character output to a terminal device utilizing 6 bit characters",
                {op_type::port, op_type::val}
            }
        },{
            "%ASCII7", {
                "Write character output to a terminal device utilizing 7 bit ASCII characters",
                {op_type::port, op_type::val}
            }
        },{
            "%UTF8", {
                "Write character output to a terminal device utilizing UTF-8",
                {op_type::port, op_type::val}
            }
        },{
            "%UTF16", {
                "Write character output to a terminal device utilizing UTF-16",
                {op_type::port, op_type::val}
            }
        },{
            "%UTF32", {
                "Write character output to a terminal device utilizing UTF-32",
                {op_type::port, op_type::val}
            }
        },{
            "%INT", {
                "Write numeric output to a numeric device using signed decimal numbers",
                {op_type::port, op_type::val}
            }
        },{
            "%UINT", {
                "Write numeric output to a numeric device using unsigned decimal numbers",
                {op_type::port, op_type::val}
            }
        },{
            "%BIN", {
                "Write numeric output to a numeric device utilizing binary numbers",
                {op_type::port, op_type::val}
            }
        },{
            "%HEX", {
                "Write numeric output to a numeric device utilizing hexadecimal numbers",
                {op_type::port, op_type::val}
            }
        },{
            "%FLOAT", {
                "Write numeric output to a numeric device utilizing floating point numbers",
                {op_type::port, op_type::val}
            }
        },{
            "%FIXED", {
                "Write numeric output to a numeric device utilizing fixed point numbers",
                {op_type::port, op_type::val}
            }
        },{
            "%ADDR", {
                "Set storage device address",
                {op_type::port, op_type::val}
            }
        },{
            "%BUS", {
                "Write word of data to storage device (address specified by %ADDR)",
                {op_type::port, op_type::val}
            }
        },{
            "%PAGE", {
                "Set page of storage device",
                {op_type::port, op_type::val}
            }
        },{
            "%RNG", {
                "Set seed of random number generator",
                {op_type::port, op_type::val}
            }
        },{
            "%NOTE", {
                "Set output pitch of audio device",
                {op_type::port, op_type::val}
            }
        },{
            "%INSTR", {
                "Set output instrument of audio device",
                {op_type::port, op_type::val}
            }
        },{
            "%NLEG", {
                "Make sound on sound device of given length (in milliseconds)",
                {op_type::port, op_type::val}
            }
        },{
            "%WAIT", {
                "Set wait time (in milliseconds)",
                {op_type::port, op_type::val}
            }
        },{
            "%NADDR", {
                "Set network address",
                {op_type::port, op_type::val}
            }
        },{
            "%DATA", {
                "Send network data",
                {op_type::port, op_type::val}
            }
        }
    };
}

#endif