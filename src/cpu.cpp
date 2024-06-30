#include <Arduino.h>
#include "peripherals.hpp"
#include "registers.hpp"
#include "cpu.hpp"
//#include "ope.hpp"


Cpu::Cpu(){
    this->cycle = 0;
}

void Cpu::emulate_cycle(Peripherals &bus){
    // 16bit命令
    if (this->ctx.cb) {
        this->cb_decode(bus);
        return;
    }
    // 8bit命令
    this->decode(bus);
    
}


// 0xCBの場合は16bit命令
void Cpu::cb_prefixed(Peripherals &bus) {
    static uint8_t _val = 0;
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

    //if(this->ctx.opecode != pgm_read_byte(&(OpeData[this->cycle]))){
    //    delay(10000);
    //}
    this->cycle += 1;

    //delay(1000);
}


void Cpu::decode(Peripherals &bus){
    switch(this->ctx.opecode){

        case 0x00: this->nop(bus); break;
        
        
        case 0x1A: this->ld(bus, Reg8::A, Indirect::DE); break;         // 2サイクル
        case 0x3E: this->ld(bus, Reg8::A, this->imm8); break;           // 2サイクル
        case 0x06: this->ld(bus, Reg8::B, this->imm8); break;
        case 0x0E: this->ld(bus, Reg8::C, this->imm8); break;           // 2サイクル
        case 0x2E: this->ld(bus, Reg8::L, this->imm8); break;
        case 0x47: this->ld(bus, Reg8::B, Reg8::A); break;              // 1サイクル
        case 0x79: this->ld(bus, Reg8::A, Reg8::C); break;              // 1サイクル
        case 0x7B: this->ld(bus, Reg8::A, Reg8::E); break;
        case 0x7A: this->ld(bus, Reg8::A, Reg8::D); break;
        case 0x57: this->ld(bus, Reg8::D, Reg8::A); break;
        case 0x22: this->ld(bus, Indirect::HLI, Reg8::A); break;        // 2サイクル
        case 0x32: this->ld(bus, Indirect::HLD, Reg8::A); break;
        case 0xEA: this->ld(bus, Direct8::D, Reg8::A); break;
        case 0xE0: this->ld(bus, Direct8::DFF, Reg8::A); break;         // 2サイクル
        
        case 0x11: this->ld16(bus, Reg16::DE, this->imm16); break;
        case 0x21: this->ld16(bus, Reg16::HL, this->imm16); break;      // 3サイクル
        case 0x31: this->ld16(bus, Reg16::SP, this->imm16); break;      // 3サイクル

        case 0x3D: this->dec(bus, Reg8::A); break;                      // 1サイクル
        case 0x05: this->dec(bus, Reg8::B); break;
        case 0x0D: this->dec(bus, Reg8::C); break;
        case 0x15: this->dec(bus, Reg8::D); break;
        
        case 0x23: this->inc16(bus, Reg16::HL); break;
        case 0x13: this->inc16(bus, Reg16::DE); break;

        case 0xC5: this->push(bus, Reg16::BC); break;
        case 0xF5: this->push(bus, Reg16::AF); break;                   // 4サイクル
        
        case 0xF1: this->pop(bus, Reg16::AF); break;                    // 3サイクル
        case 0xC1: this->pop(bus, Reg16::BC); break;

        case 0x18: this->jr(bus); break;
        case 0x28: this->jr_c(bus, Cond::Z); break;                     // 2-3サイクル
        case 0x20: this->jr_c(bus, Cond::NZ); break;                    // 2-3サイクル
        case 0xCD: this->call(bus); break;                              // 
        case 0xC9: this->ret(bus); break;
        case 0xCB: this->cb_prefixed(bus); break;

        case 0xFE: this->cp(bus, this->imm8); break;

    }
}

void Cpu::cb_decode(Peripherals &bus){
    switch(this->ctx.opecode){
        case 0x10: this->rl(bus, Reg8::B); break;                       // 2サイクル
        case 0x11: this->rl(bus, Reg8::C); break;                       // 2サイクル
        case 0x6C: this->chkbit(bus, 5, Reg8::H); break;                // 2サイクル
    }
}





