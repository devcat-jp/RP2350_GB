#include <Arduino.h>
#include "hram.hpp"

uint8_t HRam::read(uint16_t addr){
    return this->hram[addr & 0x7f];
}

void HRam::write(uint16_t addr, uint8_t val){
    this->hram[addr & 0x7f] = val;
}

