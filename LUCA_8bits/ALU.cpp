#include <bitset>
#include <iostream>

#include "ALU.h"

// change Binary to Decimal
int binToDec(operand bin, bool sign) {
    if (sign && bin[7] == 1) { // if signed int and MSB = 1 then negative
        bin.flip();
        return -(bin.to_ulong() + 1);
    }

    return bin.to_ulong();
}

operand adder(operand x, operand y, std::bitset<8> &SR, bool sub) {
    operand ret;
    int carry[9]{0,};

    // subtractor
    if (sub) {
        carry[0] = 1;
        y.flip();
    }
    
    // ripple carry adder
    for (int i = 0; i < 8; i++) {
        if ((x[i] && y[i]) || (x[i] && carry[i]) || (y[i] && carry[i])) {
            carry[i+1] = 1;
        }
        else carry[i+1] = 0;

        if (x[i] ^ y[i] ^ carry[i]) {
            ret[i] = 1;
        }
        else ret[i] = 0;
    }

    // set Status Register
    if (carry[8]) SR[C] = 1;
    else SR[C] = 0;

    if (ret.count() == 0) SR[Z] = 1;
    else SR[Z] = 0;

    if (ret[7] == 1) SR[N] = 1;
    else SR[N] = 0; 

    if (carry[7] ^ carry[8]) SR[O] = 1;
    else SR[O] = 0;

    return ret;
}

operand logicUnit(int opt, std::bitset<8> &SR, operand x, operand y) {
    operand ret;
    switch (opt) {
        case 0b1: // and
            for (int i = 0; i < 8; i++) ret[i] = x[i] & y[i];
            break;
        case 0b10: // or
            for (int i = 0; i < 8; i++) ret[i] = x[i] | y[i];
            break;
        case 0b11: // xor
            for (int i = 0; i < 8; i++) ret[i] = x[i] ^ y[i];
            break;
        case 0b100: // inv
            ret = x.flip();
            break;
        case 0b101: // neg
            ret = x.flip().to_ulong() + 1;
            break;
        default:
            break;
    }

    if (ret.count() == 0) SR[Z] = 1;
    else SR[Z] = 0;

    if (ret[7]) SR[N] = 1;
    else SR[N] = 0; 

    return ret;
}

int calculate(operand x, operand y, std::bitset<4> ALUop, std::bitset<8> &SR) {
    int ret;
    
    if (ALUop == 0b1000) { // add
        ret = binToDec(adder(x, y, SR));
    }
    else if (ALUop == 0b1001) { // sub
        ret = binToDec(adder(x, y, SR, 1));
    }
    else if (ALUop == 0b0001) { // and
        ret = binToDec(logicUnit(ALUop.to_ulong(), SR, x, y));
    }
    else if (ALUop == 0b0010) { // or
        ret = binToDec(logicUnit(ALUop.to_ulong(), SR, x, y));
    }
    else if (ALUop == 0b0011) { // xor
        ret = binToDec(logicUnit(ALUop.to_ulong(), SR, x, y));
    }
    else if (ALUop == 0b0100) { // inv
        ret = binToDec(logicUnit(ALUop.to_ulong(), SR, x));
    }
    else if (ALUop == 0b0101) { // neg
        ret = binToDec(logicUnit(ALUop.to_ulong(), SR, x));
    }

    return ret;
}