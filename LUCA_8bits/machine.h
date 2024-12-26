#ifndef MACHINE_H
#define MACHINE_H

#include <iostream>
#include "ALU.h"

enum registers { // register file alias
    D0, D1, D2, D3, D4, D5, D6, D7,
    PC, IX, RT, SP, BE, MDR, MAR
};

class LUCA {
    size_t cycles; // machine cycles
    double clock; // clock rate (millisecond)

    int registerFile[15]; // visuable regesters
    std::bitset<16> IR; // instruction register
    std::bitset<8> SR; // status register
    int IODR; // I/O data register

    /*      memory array
        2 bytes per instruction */
    std::bitset<16> instructionMem[256]; // 512 byte instruction memory
    int dataMem[256]; // 256 byte data memory

    /* control signals */
    std::bitset<4> ALUop;
    bool RegWR;
    bool memWR;	
    bool memRD;	
    std::bitset<2> WBsrc;
    std::bitset<2> branch;
    bool jump;
    bool in;
    bool out;
    bool halt;

    /* Operation signals */
    operand literal; // immd or [Addr] or [disp]
    operand ALUresult;
    std::bitset<3> Ra; // operand A (register)
    std::bitset<3> Rb; // operand B (register)
    std::bitset<3> Rc; // operand C (write register)

    /* stage functions */
    void fetch();
    void decode();
    void excute();
    void memoryAccess();
    void WriteBack();
    
public:
    LUCA(double clock_=0);
    void run();
    void loader(int program[], size_t lines);

    friend std::ostream& operator<<(std::ostream& out, const LUCA &luca) {
    out << "cycles: " << luca.cycles << "\n"
        << "D0: " << std::bitset<8>(luca.registerFile[D0]) << "\t"
        << "D1: " << std::bitset<8>(luca.registerFile[D1]) << "\t"
        << "D2: " << std::bitset<8>(luca.registerFile[D2]) << "\n"
        << "D3: " << std::bitset<8>(luca.registerFile[D3]) << "\t"
        << "D4: " << std::bitset<8>(luca.registerFile[D4]) << "\t"
        << "D5: " << std::bitset<8>(luca.registerFile[D5]) << "\n"
        << "D6: " << std::bitset<8>(luca.registerFile[D6]) << "\t"
        << "D7: " << std::bitset<8>(luca.registerFile[D7]) << "\t"
        << "PC: " << std::bitset<8>(luca.registerFile[PC]) << "\n"
        << "MDR: " << std::bitset<8>(luca.registerFile[MDR]) << "\t"
        << "MAR: " << std::bitset<8>(luca.registerFile[MAR]) << "\t"
        << "IR: " << std::bitset<16>(luca.IR) << "\n";

    return out;
}
};

#endif // MACHINE_H