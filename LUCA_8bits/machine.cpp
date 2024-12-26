#include <iostream>
#include <bitset>
#include <array>
#include <vector>
#include <unistd.h>

#include "machine.h"

template <size_t ns, size_t is>
std::bitset<ns> slice(std::bitset<is> input, int start) {
    std::bitset<ns> ret;

    if (ns + start > is) return ret;

    for (int i = 0; i < ns; i++) {
        ret[i] = input[start + i];
    }

    return ret;
}

template <size_t n>
bool minterm(const std::bitset<n> &lhs, const std::array<int, n> &rhs) {
    for (size_t i = 0; i < n; i++) {
        if (rhs[i] != 2 && lhs[n-i-1] != rhs[i]) return false;
    }

    return true;
}

template <size_t n>
bool sop(const std::bitset<n> &lhs, const std::vector<std::array<int, n>> &terms) {
    for (const auto& term : terms) {
        if (minterm(lhs, term)) {
            return true;
        }
    }
    return false;
}

LUCA::LUCA(double clock_) {
    for (int i = 0; i < 15; i++) registerFile[i] = 0;
    for (int i = 0; i < 256; i++) {
        dataMem[i] = 0;
        instructionMem[i] = 0;
    }

    clock = clock_;
    cycles = 0;
    halt = false;

    IR = 0;
    SR = 0;
    IODR = 0;

    ALUop = 0;
    RegWR = 0;
    memWR = 0;
    memRD = 0;
    WBsrc = 0;
    branch = 0;
    jump = 0;
    in = 0;
    out = 0;

    literal = 0;
    ALUresult = 0;
    Ra = 0;
    Rb = 0;
    Rc = 0;
}

void LUCA::fetch() {
    registerFile[MAR] = registerFile[PC];
    IR = instructionMem[registerFile[MAR]];
}

void LUCA::decode() {
    std::bitset<5> opCode = slice<5>(IR, IR.size()-5);
    std::bitset<7> ALUCode = slice<7>(IR, IR.size()-7);
    
    literal = slice<8>(IR, 3);
    Ra = slice<3>(IR, 6);
    Rb = slice<3>(IR, 3);
    Rc = slice<3>(IR, 0);

    /* Set ALUop */
    if (sop(ALUCode, {{0,0,0,0,1,0,0}})) {
        ALUop = 0b1000; // add
    }
    else if (sop(ALUCode, {{0,0,0,0,1,0,1}, {1,0,0,0,0,2,2}})) {
        ALUop = 0b1001; // sub
    }
    else if (sop(ALUCode, {{0,0,0,1,0,0,0}})) {
        ALUop = 0b0001; // and
    }
    else if (sop(ALUCode, {{0,0,0,1,0,0,1}})) {
        ALUop = 0b0010; // or
    }
    else if (sop(ALUCode, {{0,0,0,1,0,1,0}})) {
        ALUop = 0b0011; // xor
    }
    else if (sop(ALUCode, {{0,0,0,1,1,0,0}})) {
        ALUop = 0b0100; // inv
    }
    else if (sop(ALUCode, {{0,0,0,1,1,0,1}})) {
        ALUop = 0b0101; // neg
    }
    else ALUop = 0;

    /* Set Reg Write Back */
    if (sop(opCode, {{0,2,0,0,1}, {0,1,0,0,2}, {0,1,1,1,1}})) RegWR = true;
    else RegWR = false;

    /* Set Mem Write */
    if (sop(opCode, {{0,1,0,1,0}})) memWR = true;
    else memWR = false;

    /* Set Mem Read */
    if (sop(opCode, {{0,1,0,0,0}})) memRD = true;
    else memRD = false;

    /* Set Write Back source */
    if (sop(opCode, {{0,1,0,0,0}})) WBsrc = 01; // memory
    else if (sop(opCode, {{0,1,0,0,1}})) WBsrc = 10; // immediate
    else if (sop(opCode, {{0,1,1,1,1}})) WBsrc = 11; // IO
    else WBsrc = 0;

    /* Set Branch */
    if (sop(opCode, {{1,0,0,0,1}})) branch = 0b01; // branch less then
    else if (sop(opCode, {{1,0,0,1,0}})) branch = 0b10; // branch equal
    else branch = 0;

    /* Set Jump */
    if (sop(opCode, {{1,1,0,0,0}})) jump = true;
    else jump = false;

    /* Special Features */
    if (sop(opCode, {{0,1,1,1,1}})) in = true;
    else in = false;
    
    if (sop(opCode, {{1,1,1,1,0}})) out = true;
    else out = false;
    
    if (sop(opCode, {{1,1,1,1,1}})) halt = true;
    else halt = false;
}

void LUCA::excute() {
    operand srcA = registerFile[Ra.to_ulong()];
    operand srcB = registerFile[Rb.to_ulong()];
    ALUresult = calculate(srcA, srcB, ALUop, SR);

    if (branch == 0b01 && (SR[N] || (SR[O] && !SR[N]))) { // branch less then
        registerFile[PC] += binToDec(literal);
    } 
    else if (branch == 0b10 && SR[Z]) { // branch equal
        registerFile[PC] += binToDec(literal);
    }
    else if (jump) registerFile[PC] = literal.to_ulong();
    else registerFile[PC]++;
}

void LUCA::memoryAccess() {
    registerFile[MAR] = literal.to_ulong();

    if (memRD) {
        registerFile[MDR] = dataMem[registerFile[MAR]];
    }

    if (memWR) {
        registerFile[MDR] = registerFile[Rc.to_ullong()];
        dataMem[registerFile[MAR]] = registerFile[MDR];
    }
}

void LUCA::WriteBack() {
    if (in) {
        std::cout << "input: ";
        std::cin >> IODR;
    }

    if (out) {
        IODR = registerFile[D0];
        std::cout << "output: " << binToDec(IODR) << "\n";
        sleep(3);
    }

    if (RegWR) {
        if (WBsrc == 0) {
            registerFile[Rc.to_ulong()] = ALUresult.to_ulong();
        }
        if (WBsrc == 0b01) {
            registerFile[Rc.to_ullong()] = registerFile[MDR];
        }
        else if (WBsrc == 0b10) {
            registerFile[Rc.to_ullong()] = literal.to_ullong();
        }
        else if (WBsrc == 0b11) {
            registerFile[D0] = IODR; 
        }
    }

}

void LUCA::run() {
    while (!halt) {
        cycles++;
        
        fetch();
        std::cout << "\033[2J\033[1;1H";
        std::cout << *this;
        decode();
        std::cout << "\033[2J\033[1;1H";
        std::cout << *this;
        excute();
        std::cout << "\033[2J\033[1;1H";
        std::cout << *this;
        memoryAccess();
        std::cout << "\033[2J\033[1;1H";
        std::cout << *this;
        WriteBack();
        std::cout << "\033[2J\033[1;1H";
        std::cout << *this;

        usleep(clock * 1000);
    }

    std::cout << "machine halted\n";
    sleep(3);
}

void LUCA::loader(int program[],size_t lines) {
    for (size_t i = 0; i < lines; i++) instructionMem[i] = program[i];
}