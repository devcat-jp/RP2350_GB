#include <Arduino.h>
#include "bootrom.hpp"


BootRom::BootRom(){
    this->active = true;
}

bool BootRom::isActive() {
    return this->active;
}

uint8_t BootRom::read(uint16_t addr){
    return pgm_read_byte(&rom[addr]);
}

void BootRom::write(uint16_t addr, uint8_t val){
    if (val != 0) this->active = false;
}
