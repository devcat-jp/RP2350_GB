
#include <Arduino.h>
#include "peripherals.hpp"
#include "registers.hpp"
#include "cpu.hpp"


// NOP命令、何もしない
void Cpu::nop(Peripherals &bus){
    this->fetch(bus);
}


// ld d s ： s の値を d  に格納する
template<typename T, typename U> void Cpu::ld(Peripherals &bus, T dst, U src){
    static uint8_t _step = 0;
    static uint8_t _val8 = 0;

    switch(_step){
        case 0:
            if(this->read8(bus, src, _val8)) _step = 1;
            break;
        case 1:
            if(this->write8(bus, dst, _val8)){
                this->fetch(bus);
                _step = 0;
            }
            break;
    };
}
template<typename T, typename U> void Cpu::ld16(Peripherals &bus, T dst, U src){
    static uint8_t _step = 0;
    static uint16_t _val16 = 0;
    
    switch(_step){
        case 0:
            if(this->read16(bus, src, _val16)) _step = 1;
            break;
        case 1:
            if(this->write16(bus, dst, _val16)){
                this->fetch(bus);
                _step = 0;
            }
            break;
    };
}


// bit num s : s の num bit目が0か1かを確認する
template<typename T> void Cpu::chkbit(Peripherals &bus, uint8_t bitsize, T src){
    uint8_t val = 0;
    if(this->read8(bus, src, val)){
        val &= 1 << bitsize;
        this->regs.set_zf(val == 0);        // Zフラグ、指定bitが0の場合は1にする
        this->regs.set_nf(false);           // Nフラグ、無条件に0
        this->regs.set_hf(true);            // Hフラグ、無条件に1
        this->fetch(bus);
    }

}

// dec : sをデクリメント
// 8bitの場合
template<typename T> bool Cpu::dec(Peripherals &bus, T src){
    static uint8_t _step = 0;
    static uint8_t _val8 = 0;
    static uint8_t _result = 0;

    RE_ACTION:
    switch(_step){
        case 0:
            if(this->read8(bus, src, _val8)){
                // デクリメント
                _result = _val8 - 1;
                // フラグ操作
                this->regs.set_zf(_result == 0);        // Zフラグ、演算結果が0の場合は1
                this->regs.set_nf(false);               // Nフラグ、無条件に0
                this->regs.set_hf(_val8 & 0xF == 0);    // Hフラグ、4bit目からの繰り下がりが発生すると1
                _step = 1;
                goto RE_ACTION;
            }
            return false;
        case 1:
            if(this->write8(bus, src, _result)){
                _step = 0;
                this->fetch(bus);
                return true;
            }
            return false;
    };
    return false;
}


// INC s : sをインクリメントする
// 8bit操作の時はフラグレジスタ操作も必要
template<typename T> bool Cpu::inc(Peripherals &bus, T src){
    static uint8_t _step = 0;
    static uint8_t _val8 = 0;
    static uint8_t _result = 0;

    RE_ACTION:
    switch(_step){
        case 0:
            if(this->read8(bus, src, _val8)){
                // インクリメント
                _result = _val8 + 1;
                // フラグ操作
                this->regs.set_zf(_result == 0);            // Zフラグ、演算結果が0の場合は1
                this->regs.set_nf(false);                   // Nフラグ、無条件に0
                this->regs.set_hf(_val8 & 0xF == 0xF);      // Hフラグ、3bit目で繰り上がりが発生すると1
                _step = 1;
                goto RE_ACTION;
            }
            return false;
        case 1:
            if(this->write8(bus, src, _result)){
                _step = 0;
                this->fetch(bus);
                return true;
            }
            return false;
    };

    return false;
}

template<typename T> bool Cpu::inc16(Peripherals &bus, T src){
    static uint8_t _step = 0;
    static uint16_t _val16 = 0;

    RE_ACTION:
    switch(_step){
        case 0:
            if(this->read16(bus, src, _val16)){
                // インクリメント
                _val16 += 1;
                _step = 1;
                goto RE_ACTION;
            }
            return false;
        case 1:
            if(this->write16(bus, src, _val16)){
                _step = 0;
                this->fetch(bus);
                return true;
            }
            return false;
    };

    return false;
}



// RL s : sの値とCフラグを合わせた9bitの値を左に回転 = 1bit左シフト、Cフラグを最下位bitにセットする
template<typename T> bool Cpu::rl(Peripherals &bus, T src){
    static uint8_t _step = 0;
    static uint8_t _val8 = 0;
    static uint8_t _result = 0;

    RE_ACTION:
    switch(_step){
        case 0:
            // src の値を取得する
            if(this->read8(bus, src, _val8)){
                // Cフラグを左シフトし、srcとのorをとる
                _result = _val8 << 1 | (uint8_t)this->regs.cf();
                // フラグ操作
                this->regs.set_zf(_result == 0);        // 演算結果が0の場合は1
                this->regs.set_nf(0);                   // 無条件に0
                this->regs.set_hf(0);                   // 無条件に0
                this->regs.set_cf((_val8 & 0x80) > 0);  // 演算前のsrcで7bit目が1の場合は1
                _step = 1;
                goto RE_ACTION;
            }
            return false;
        case 1:
            // 値の書き込み
            if(this->write8(bus, src, _result)){
                _step = 0;
                this->fetch(bus);
                return true;
            }
            return false;
    };

    return false;
}




// push ：　16bitの値を、スタックポインタをデクリメントした後にスタックポインタが指すアドレスに値を格納する
// 3サイクル
bool Cpu::push16(Peripherals &bus, uint16_t val){
    static uint8_t _step = 0;

    switch(_step){
        case 0:
            // メモリアクセス回数 + 1
            _step = 1;
            return false;
        case 1:
            // SPをデクリメントし、SPが指すアドレスに値を書き込む（H）
            this->regs.sp -= 1;
            bus.write(this->regs.sp, (uint8_t)(val >> 8));
            _step = 2;
            return false;
        case 2:
            // 下位ビット
            this->regs.sp -= 1;
            bus.write(this->regs.sp, (uint8_t)(val & 0xFF));
            _step = 3;
            return false;
        case 3:
            _step = 0;
            return true;
    }

    return false;
}

// 4サイクル固定
bool Cpu::push(Peripherals &bus, Reg16 src){
    static uint8_t _step = 0;
    static uint16_t _val16 = 0;

    RE_ACTION:
    switch(_step){
        case 0:
            // 16bitの値取得
            this->read16(bus, src, _val16);
            _step = 1;
            goto RE_ACTION;
        case 1:
            // プログラムカウンタの値をpush（3サイクル）
            if(this->push16(bus, _val16)){
                _step = 2;
                goto RE_ACTION;
            }
            return false;
        case 2:
            _step = 0;
            this->fetch(bus);
            return true;
    }
    return false;
}



// pop : 16bitの値をスタックからpop
// スタックポインタが指すアドレスに格納されている値をレジスタに格納した後に，スタックポインタをインクリメント
bool Cpu::pop16(Peripherals &bus, uint16_t &val){
    static uint8_t _step = 0;
    static uint8_t _lo = 0, _hi = 0;

    switch(_step){
        case 0:
            // 下位8bitの値取得し、SPをインクリメント
            _lo = bus.read(this->regs.sp);
            this->regs.sp += 1;
            _step = 1;
            break;
        case 1:
            // 上位8bitの値取得し、SPをインクリメント
            _hi = bus.read(this->regs.sp);
            this->regs.sp += 1;
            // 結果代入
            val = (uint16_t)(_hi << 8) | (uint16_t)_lo; 
            _step = 2;
            break;
        case 2:
            _step = 0;
            return true;
    };

    return false;
}
bool Cpu::pop(Peripherals &bus, Reg16 dst){
    static uint16_t _val = 0;
    
    if(this->pop16(bus, _val)){
        // 取り出した値をレジスタに書き込み、サイクル消費しない
        this->write16(bus, dst, _val);
        this->fetch(bus);
        return true;
    }

    return false;
}



// call ：　プログラムカウンタの値をスタックにpushし、その後元のプログラムカウンタに戻す
// 6サイクル固定
bool Cpu::call(Peripherals &bus){
    static uint8_t _step = 0;
    static uint16_t _val16 = 0;

    RE_ACTION:
    switch(_step){
        case 0:
            // プログラムカウンタの値取り出し
            if(this->read16(bus, this->imm16, _val16)){
                _step = 1;
                goto RE_ACTION;
            }
            return false;
        case 1:
            // プログラムカウンタの値をpush（3サイクル）
            if(this->push16(bus, this->regs.pc)){
                // 記録した値を戻す
                this->regs.pc = _val16;
                _step = 2;
                goto RE_ACTION;
            }
            return false;
        case 2:
            _step = 0;
            this->fetch(bus);
            return true;
    };

    return false;
}


// JR c : フラグがcを満たしていればJR命令（プログラムカウンタに加算）を行う
bool Cpu::cond(Peripherals &bus, Cond c){
    switch(c){
        case Cond::NZ: return !this->regs.zf();     // not Zフラグ
        case Cond::Z: return this->regs.zf();       // Zフラグ
        case Cond::NC: return !this->regs.cf();     // not Cフラグ
        case Cond::C: return this->regs.cf();       // Cフラグ
    }
    return true;
}
void Cpu::jr_c(Peripherals &bus, Cond c){
    static uint8_t _step = 0;
    static uint8_t _val8 = 0;

    RE_ACTION:
    switch(_step){
        case 0:
            if(this->read8(bus, this->imm8, _val8)) {
                if(this->cond(bus, c)){
                    this->regs.pc += (int8_t)_val8;
                    _step = 1;                      // ジャンプの場合はサイクル数+1
                }
                else _step = 2;
                goto RE_ACTION;
            }
            break;
        case 1:
            _step = 2;
            break;
        case 2:
            _step = 0;
            this->fetch(bus);
            break;
    }
}



// RET : return
// 16bitの値をプログラムカウンタに代入する、4サイクル
void Cpu::ret(Peripherals &bus){
    static uint8_t _step = 0;
    static uint16_t _val16 = 0;

    switch(_step){
        case 0:
            if(this->pop16(bus, _val16)){
                this->dVal = _val16;
                this->regs.pc = _val16;
                _step = 1;
            }
            break;
        case 1:
            _step = 0;
            this->fetch(bus);
            break;
    };
}



// テンプレート関数の実体化
template void Cpu::ld<Reg8, Reg8>(Peripherals &bus, Reg8 dst, Reg8 src);
template void Cpu::ld<Reg8, Imm8>(Peripherals &bus, Reg8 dst, Imm8 src);
template void Cpu::ld<Reg8, Indirect>(Peripherals &bus, Reg8 dst, Indirect src);
template void Cpu::ld<Indirect, Reg8>(Peripherals &bus, Indirect dst, Reg8 src);
template void Cpu::ld<Direct8, Reg8>(Peripherals &bus, Direct8 dst, Reg8 src);
template void Cpu::ld16<Reg16, Imm16>(Peripherals &bus, Reg16 dst, Imm16 src);
template void Cpu::chkbit<Reg8>(Peripherals &bus, uint8_t bitsize, Reg8 src);
template bool Cpu::rl<Reg8>(Peripherals &bus, Reg8 src);
template bool Cpu::dec(Peripherals &bus, Reg8 src);
template bool Cpu::inc16(Peripherals &bus, Reg16 src);