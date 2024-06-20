
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

    switch(_step){
        case 0:
            if(this->read8(bus, this->imm8, _val8)) {
                if(this->cond(bus, c)){
                    this->regs.pc += (int8_t)_val8;
                    _step = 1;                      // ジャンプの場合はサイクル数+1
                }
                else _step = 2;
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



// push ：　スタックポインタ（16bit）をデクリメントした後にスタックポインタが指すアドレスに値を格納する
// 4サイクル固定
bool push16(Peripherals &bus, uint16_t val){
    static uint8_t _step = 0;

    switch(_step){
        case 0:
            _step = 1;
            return false;
        case 1:
            _step = 2;
            return false;
        case 2:
            _step = 3;
            return false;
        case 3:
            _step = 4;
            return false;
        case 4:
            // SPをデクリメントし、SPが指すアドレスに値を書き込む（H）
            this->regs.sp -= 1;
            bus.write(this->regs.sp, (uint8_t)(val >> 8));
            // 下位ビット
            this->regs.sp -= 1;
            bus.write(this->regs.sp, (uint8_t)(val & 0xFF));
            _step = 0;
            return true;
    }
    return false;
}



/*
    // push ：　16bit値をデクリメントした後にスタックポインタが指すアドレスに値を格納する
    pub fn push16 (&mut self, bus: &mut Peripherals, val: u16) -> Option<()> {
        //println!("[push16]");
        static STEP: AtomicU8 = AtomicU8::new(0);
        static VAL8: AtomicU8 = AtomicU8::new(0);
        static VAL16: AtomicU16 = AtomicU16::new(0);
        match STEP.load(Relaxed) {
            0 => {
                // pushはメモリアクセス数+1のサイクル数
                STEP.store(1, Relaxed);
                None
            },
            1 => {
                // 値を取得
                let [lo, hi] = u16::to_le_bytes(val);
                // デクリメントしたアドレスに書き込み
                self.regs.sp = self.regs.sp.wrapping_sub(1);
                bus.write(&mut self.interrupts, self.regs.sp, hi);
                //
                VAL8.store(lo, Relaxed);
                STEP.store(2, Relaxed);
                None
            },
            2 => {
                // デクリメントしたアドレスに書き込み
                self.regs.sp = self.regs.sp.wrapping_sub(1);
                bus.write(&mut self.interrupts, self.regs.sp, VAL8.load(Relaxed));
                //
                STEP.store(0, Relaxed);
                Some(())
            },
            _ => panic!("Not implemented: push16"),
        }
    }
    pub fn push (&mut self, bus: &mut Peripherals, src: Reg16) {
        //println!("push");
        static STEP: AtomicU8 = AtomicU8::new(0);
        static VAL16: AtomicU16 = AtomicU16::new(0);
        match STEP.load(Relaxed) {
            0 => {
                // pushはレジスタ操作のみなのでサイクル消費しない
                VAL16.store(self.read16(bus, src).unwrap(), Relaxed);
                STEP.store(1, Relaxed);
                // 応答が得られたので再度処理を行う
                self.push(bus, src);
            },
            1 => {
                if self.push16(bus, VAL16.load(Relaxed)).is_some() {
                    STEP.store(2, Relaxed);
                }
            },
            2 => {
                STEP.store(0, Relaxed);
                self.fetch(bus);
            },
            _ => panic!("Not implemented: push"),
        }
    }
*/



// テンプレート関数の実体化
template void Cpu::ld<Reg8, Reg8>(Peripherals &bus, Reg8 dst, Reg8 src);
template void Cpu::ld<Reg8, Imm8>(Peripherals &bus, Reg8 dst, Imm8 src);
template void Cpu::ld<Reg8, Indirect>(Peripherals &bus, Reg8 dst, Indirect src);
template void Cpu::ld<Indirect, Reg8>(Peripherals &bus, Indirect dst, Reg8 src);
template void Cpu::ld<Direct8, Reg8>(Peripherals &bus, Direct8 dst, Reg8 src);
template void Cpu::ld16<Reg16, Imm16>(Peripherals &bus, Reg16 dst, Imm16 src);
template void Cpu::chkbit<Reg8>(Peripherals &bus, uint8_t bitsize, Reg8 src);