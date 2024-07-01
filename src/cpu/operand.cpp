#include <Arduino.h>
#include "peripherals.hpp"
#include "registers.hpp"
#include "cpu.hpp"


// 8bitレジスタのRW
// サイクル消費はしない
bool Cpu::read8(Peripherals &bus, Reg8 src, uint8_t &val){
    switch(src){
        case Reg8::A: val = this->regs.a; return true;
        case Reg8::B: val = this->regs.b; return true;
        case Reg8::C: val = this->regs.c; return true;
        case Reg8::D: val = this->regs.d; return true;
        case Reg8::E: val = this->regs.e; return true;
        case Reg8::H: val = this->regs.h; return true;
        case Reg8::L: val = this->regs.l; return true;
    };
    return false;
}
bool Cpu::write8(Peripherals &bus, Reg8 dst, uint8_t val){
    switch(dst){
        case Reg8::A: this->regs.a = val; return true;
        case Reg8::B: this->regs.b = val; return true;
        case Reg8::C: this->regs.c = val; return true;
        case Reg8::D: this->regs.d = val; return true;
        case Reg8::E: this->regs.e = val; return true;
        case Reg8::H: this->regs.h = val; return true;
        case Reg8::L: this->regs.l = val; return true;
    };
    return false;
}

// 16bitレジスタのRW
// サイクル消費はしない
bool Cpu::read16(Peripherals &bus, Reg16 src, uint16_t &val){
    switch(src){
        case Reg16::AF: val = this->regs.af(); return true;
        case Reg16::BC: val = this->regs.bc(); return true;
        case Reg16::DE: val = this->regs.de(); return true;
        case Reg16::HL: val = this->regs.hl(); return true;
        case Reg16::SP: val = this->regs.sp; return true;
    };
    return false;
}
bool Cpu::write16(Peripherals &bus, Reg16 src, uint16_t val){
    switch(src){
        case Reg16::AF: this->regs.write_af(val); return true;
        case Reg16::BC: this->regs.write_bc(val); return true;
        case Reg16::DE: this->regs.write_de(val); return true;
        case Reg16::HL: this->regs.write_hl(val); return true;
        case Reg16::SP: this->regs.sp = val; return true;
    };
    return false;
}

// プログラムカウンタが指す場所から読み取られる8bitのRW
// サイクル1消費
bool Cpu::read8(Peripherals &bus, Imm8 src, uint8_t &val){
    static uint8_t _step = 0;

    switch(_step){
        case 0:
            _step = 1;
            return false;
        case 1:
            val = bus.read(this->regs.pc);
            this->regs.pc += 1;
            _step = 0;
            return true;
    };

    return false;
}

// プログラムカウンタが指す場所から読み取られる16bitのRW
// サイクル2消費
bool Cpu::read16(Peripherals &bus, Imm16 src, uint16_t &val){
    static uint8_t _step = 0;
    uint8_t _tmp = 0;

    RE_ACTION:
    switch(_step){
        case 0:
            if(this->read8(bus, this->imm8, _tmp)){
                val = 0;
                val |= _tmp;
                _step = 1;
                goto RE_ACTION;
            }
            return false;
        case 1:
            if(this->read8(bus, this->imm8, _tmp)){
                val |= _tmp << 8;
                _step = 2;
                goto RE_ACTION;
            }
            return false;
        case 2:
            _step = 0;
            return true;
    };

    return false;
}


// 16bitレジスタ、もしくは2つの8bitレジスタからなる16bitが指す場所の8bitを読み取る
// サイクル1消費
bool Cpu::read8(Peripherals &bus, Indirect src, uint8_t &val){
    static uint8_t _step = 0;

    switch(_step){
        case 0:
            _step = 1;
            return false;
        case 1:
            uint16_t addr = 0;
            switch(src){
                case Indirect::BC: val = bus.read(this->regs.bc()); break;
                case Indirect::DE: val = bus.read(this->regs.de()); break;
                case Indirect::HL: val = bus.read(this->regs.hl()); break;
                case Indirect::HLD:
                    // HLの値を読んだ後にデクリメントする
                    addr = this->regs.hl();
                    val = bus.read(addr);
                    addr -= 1;
                    this->regs.write_hl(addr); 
                    break;
                case Indirect::HLI:
                    // HLの値を読んだ後にインクリメントする
                    addr = this->regs.hl();
                    val = bus.read(addr);
                    addr += 1;
                    this->regs.write_hl(addr); 
                    break;
            };
            _step = 0;

            return true;
    }
    return false;
}

bool Cpu::write8(Peripherals &bus, Indirect dst, uint8_t val){
    static uint8_t _step = 0;

    switch(_step){
        case 0:
            _step = 1;
            return false;
        case 1:
            uint16_t addr = 0;
            switch(dst){
                case Indirect::BC: bus.write(this->regs.bc(), val); break;
                case Indirect::DE: bus.write(this->regs.de(), val); break;
                case Indirect::HL: bus.write(this->regs.hl(), val); break;
                case Indirect::HLD:
                    // HLの値を書いた後にデクリメントする
                    addr = this->regs.hl();
                    bus.write(addr, val);
                    addr -= 1;
                    this->regs.write_hl(addr); 
                    break;
                case Indirect::HLI:
                    // HLの値を書いた後にインクリメントする
                    addr = this->regs.hl();
                    bus.write(addr, val);
                    addr += 1;
                    this->regs.write_hl(addr); 
                    break;
            };

            _step = 0;
            return true;
    }
    return false;
}


// プログラムカウンタが指す場所から読み取られる16bitが指す場所から読み取られる8bit
// Dだと3サイクル、DEFは2サイクル
bool Cpu::read8(Peripherals &bus, Direct8 src, uint8_t &val){
    static uint8_t _step = 0;
    static uint8_t _val8 = 0;
    static uint16_t _val16 = 0;
    uint8_t _tmp = 0;

    switch(_step){
        case 0:
            if(this->read8(bus, this->imm8, _val8)){
                _step = 1;
                return false;
            }
            return false;
        case 1:
            if(src == Direct8::DFF) {
                _val16 = 0xFF00 | _val8;
                _step = 2;                              // DEFの場合はメモリアクセスが1回少ない
                return false;
            }
            if(this->read8(bus, this->imm8, _tmp)){
                _val16 = _tmp << 8 | _val8;
                _step = 2;
                return false;
            }
            return false;
        case 2:
            val = bus.read(_val16);                      // 作成したアドレスの値を読む
            _step = 0;
            return true;
    };

    return false;
}


bool Cpu::write8(Peripherals &bus, Direct8 dst, uint8_t val){
    static uint8_t _step = 0;
    static uint8_t _val8 = 0;
    static uint16_t _val16 = 0;
    uint8_t _tmp = 0;

    switch(_step){
        case 0:
            if(this->read8(bus, this->imm8, _val8)){
                _step = 1;
                return false;
            }
            return false;
        case 1:
            if(dst == Direct8::DFF) {
                _val16 = 0xFF00 | _val8;
                _step = 2;                              // DEFの場合はメモリアクセスが1回少ない
                return false;
            }
            if(this->read8(bus, this->imm8, _tmp)){
                _val16 = _tmp << 8 | _val8;
                _step = 2;
                return false;
            }
            return false;
        case 2:
            bus.write(_val16, val);                      // 作成したアドレスの値書く
            _step = 0;
            return true;
    };
    
    return false;
}