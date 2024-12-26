#include <iostream>

#include "machine.h"

using namespace std;

int main() {
    int program[] 
    {0b0111100000000000,    // in
    0b0101000000000000,     // st R0, 0
    0b0100000000000100,     // ld R4, 0
    0b0100100000000000,     // ldi R0, 0
    0b0100100000001001,     // ldi R1, 1
    0b0100100000001010,     // ldi R2, 1
    0b0000100100010100,     // add R4, R4, R2
    0b0000100000001000,     // add R0, R0, R1
    0b0000100001010001,     // add R1, R1, R2
    0b1000000001100000,     // cmp R1, R4
    0b1000111111101000,     // blt -3
    0b1111000000000000,     // out
    0b1111100000000000};    // halt

    LUCA com(500);
    com.loader(program, 13);
    com.run();
}