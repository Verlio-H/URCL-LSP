#ifndef DEFINES_H
#define DEFINES_H

#include <unordered_set>

namespace urcl::defines {
    const std::unordered_set<std::string> HEADERS = {
        "BITS", "MINREG", "MINHEAP", "MINSTACK", "RUN"
    };

    const std::unordered_set<std::string> CORE_INSTRUCTIONS = {
        "IMM", "ADD", "NOR", "RSH", "LOD", "STR", "BGE"
    };

    const std::unordered_set<std::string> BASIC_INSTRUCTIONS = {
        "IMM", "ADD", "NOR", "RSH", "LOD", "STR", "BGE", "MOV", "SUB", "NEG", "DEC", "INC", "NOT", "AND", "NAND", "OR", "NOR", "XOR", "XNOR", "LSH", "CPY", "JMP", "BRE", "BNE", "BLE", "BRL", "BRG", "BRP", "BRN", "BRZ", "BNZ", "BOD", "BEV", "BRC", "BNC", "NOP", "HLT"
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
}

#endif