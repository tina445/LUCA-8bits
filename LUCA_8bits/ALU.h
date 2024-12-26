#ifndef ALU_H
#define ALU_H
#include <bitset>

typedef std::bitset<8> operand;

enum SRstatus {
    C, Z, N, O // Carry, Zero, Negative, Overflow
};

int binToDec(operand bin, bool sign=true);
operand adder(operand x, operand y, std::bitset<8> &SR, bool sub=false);
operand logicUnit(int opt, std::bitset<8> &SR, operand x, operand y=0);
int calculate(operand x, operand y, std::bitset<4> ALUop, std::bitset<8> &SR);

#endif // ALU_H