#include <Arduino.h>
#include "peripherals.hpp"
#include "registers.hpp"
#include "cpu.hpp"


Cpu::Cpu(){
    this->cycle = 0;
}

// 0xCBの場合は16bit命令
void Cpu::cb_prefixed(Peripherals &bus) {
    uint8_t _val = 0;
    // プログラムカウンタの値を読む
    if (this->read8(bus, this->imm8, _val)) {
        this->ctx.opecode = _val;
        this->ctx.cb = true;
        this->cb_decode(bus);
    }
}


void Cpu::fetch(Peripherals &bus){
    this->ctx.opecode = bus.read(this->regs.pc);
    this->regs.pc += 1;
    this->ctx.cb = false;

    this->cycle = 0;
}


void Cpu::decode(Peripherals &bus){
    switch(this->ctx.opecode){

        case 0x00: this->nop(bus); break;
        case 0x1A: this->ld(bus, Reg8::A, Indirect::DE); break;
        case 0x3E: this->ld(bus, Reg8::A, this->imm8); break;           // 2サイクル
        case 0x47: this->ld(bus, Reg8::B, Reg8::A); break;              // 1サイクル
        case 0x22: this->ld(bus, Indirect::HLI, Reg8::A); break;        // 2サイクル
        
        case 0xE0: this->ld(bus, Direct8::DFF, Reg8::A); break;         // 2サイクル
        case 0x11: this->ld16(bus, Reg16::DE, this->imm16); break;
        case 0x21: this->ld16(bus, Reg16::HL, this->imm16); break;      // 3サイクル
        case 0x31: this->ld16(bus, Reg16::SP, this->imm16); break;      // 3サイクル

        case 0x28: this->jr_c(bus, Cond::Z); break;                     // 2-3サイクル
        case 0xCB: this->cb_prefixed(bus);

    }
}

void Cpu::cb_decode(Peripherals &bus){
    switch(this->ctx.opecode){
        case 0x6C: this->chkbit(bus, 5, Reg8::H); break;                // 2サイクル（cb確認を含む）
    }
}



void Cpu::emulate_cycle(Peripherals &bus){
    // 16bit命令
    if (this->ctx.cb) {
        this->cb_decode(bus);
        this->cycle += 1;
        return;
    }
    // 8bit命令
    this->decode(bus);
    this->cycle += 1;
}

