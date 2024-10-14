#ifndef CPU_HPP
#define CPU_HPP

#include "peripherals.hpp"
#include "registers.hpp"
#include "interrupts.hpp"

// enum
enum class Reg8 {A, B, C, D, E, H, L};
enum class Reg16 {AF, BC, DE, HL, SP};
enum class Indirect {BC, DE, HL, CFF, HLD, HLI};
enum class Direct8 {D, DFF};
enum class Cond {NZ, Z, NC, C};
enum class Imm8{};
enum class Imm16{};

// struct
struct Ctx {
    uint8_t opecode;
    bool cb;
    bool int_flag;
};


class Cpu{
    private:
        // フェッチ
        inline void fetch(Peripherals &bus){
            this->ctx.opecode = bus.read(this->interrupts, this->regs.pc);
            // 割り込み確認
            if(this->interrupts.ime && this->interrupts.get_interrupts() > 0){
                this->ctx.int_flag = true;
            } else {
                this->regs.pc += 1;
                this->ctx.int_flag = false;;
            }
            this->ctx.cb = false;
        }

        // 0xCBの場合は16bit命令
        inline void cb_prefixed(Peripherals &bus) {
            static uint8_t _val = 0;
            // プログラムカウンタの値を読む
            if (this->read8(bus, this->imm8, _val)) {
                this->ctx.opecode = _val;
                this->ctx.cb = true;
                this->cb_decode(bus);
            }
        }
        

        //---------------------------------------------------------------------------------------------
        // 8bitレジスタのR、サイクル消費はしない
        inline bool read8(Peripherals &bus, Reg8 src, uint8_t &val){
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
        // 8bitレジスタのRW、サイクル消費はしない
        inline bool write8(Peripherals &bus, Reg8 dst, uint8_t val){
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
        // 16bitレジスタのR、サイクル消費はしない
        inline bool read16(Peripherals &bus, Reg16 src, uint16_t &val){
            switch(src){
                case Reg16::AF: val = this->regs.af(); return true;
                case Reg16::BC: val = this->regs.bc(); return true;
                case Reg16::DE: val = this->regs.de(); return true;
                case Reg16::HL: val = this->regs.hl(); return true;
                case Reg16::SP: val = this->regs.sp; return true;
            };
            return false;
        }
        // 16bitレジスタのW、サイクル消費はしない
        inline bool write16(Peripherals &bus, Reg16 src, uint16_t val){
            switch(src){
                case Reg16::AF: this->regs.write_af(val); return true;
                case Reg16::BC: this->regs.write_bc(val); return true;
                case Reg16::DE: this->regs.write_de(val); return true;
                case Reg16::HL: this->regs.write_hl(val); return true;
                case Reg16::SP: this->regs.sp = val; return true;
            };
            return false;
        }

        //---------------------------------------------------------------------------------------------
        // プログラムカウンタが指す場所から読み取られる8bitのR、サイクル1消費
        inline bool read8(Peripherals &bus, Imm8 src, uint8_t &val){
            static uint8_t _step = 0;

            switch(_step){
                case 0:
                    _step = 1;
                    return false;
                case 1:
                    val = bus.read(this->interrupts, this->regs.pc);
                    this->regs.pc += 1;
                    _step = 0;
                    return true;
            };
            return false;
        }
        // プログラムカウンタが指す場所から読み取られる16bit、サイクル2消費
        inline bool read16(Peripherals &bus, Imm16 src, uint16_t &val){
            static uint8_t _step = 0;
            uint8_t _tmp;

            switch(_step){
                case 0:
                    val = bus.read(this->interrupts, this->regs.pc);
                    this->regs.pc += 1;
                    _step = 1;
                    return false;
                case 1:
                    _tmp = bus.read(this->interrupts, this->regs.pc);
                    this->regs.pc += 1;
                    val |= _tmp << 8;
                    _step = 2;
                    return false;
                case 2:
                    _step = 0;
                    return true;
            };
            return false;
        }


        //---------------------------------------------------------------------------------------------
        // 16bitレジスタ、もしくは2つの8bitレジスタからなる16bitが指す場所の8bitを読み取る
        // サイクル1消費
        inline bool read8(Peripherals &bus, Indirect src, uint8_t &val){
            static uint8_t _step = 0;

            switch(_step){
                case 0:
                    _step = 1;
                    return false;
                case 1:
                    uint16_t addr = 0;
                    switch(src){
                        case Indirect::BC: val = bus.read(this->interrupts, this->regs.bc()); break;
                        case Indirect::DE: val = bus.read(this->interrupts, this->regs.de()); break;
                        case Indirect::HL: val = bus.read(this->interrupts, this->regs.hl()); break;
                        case Indirect::HLD:
                            // HLの値を読んだ後にデクリメントする
                            addr = this->regs.hl();
                            val = bus.read(this->interrupts, addr);
                            addr -= 1;
                            this->regs.write_hl(addr); 
                            break;
                        case Indirect::HLI:
                            // HLの値を読んだ後にインクリメントする
                            addr = this->regs.hl();
                            val = bus.read(this->interrupts, addr);
                            addr += 1;
                            this->regs.write_hl(addr); 
                            break;
                    };
                    _step = 0;

                    return true;
            }
            return false;
        }
        inline bool write8(Peripherals &bus, Indirect dst, uint8_t val){
            static uint8_t _step = 0;

            switch(_step){
                case 0:
                    _step = 1;
                    return false;
                case 1:
                    uint16_t addr = 0;
                    switch(dst){
                        case Indirect::BC: bus.write(this->interrupts, this->regs.bc(), val); break;
                        case Indirect::DE: bus.write(this->interrupts, this->regs.de(), val); break;
                        case Indirect::HL: bus.write(this->interrupts, this->regs.hl(), val); break;
                        case Indirect::HLD:
                            // HLの値を書いた後にデクリメントする
                            addr = this->regs.hl();
                            bus.write(this->interrupts, addr, val);
                            addr -= 1;
                            this->regs.write_hl(addr); 
                            break;
                        case Indirect::HLI:
                            // HLの値を書いた後にインクリメントする
                            addr = this->regs.hl();
                            bus.write(this->interrupts, addr, val);
                            addr += 1;
                            this->regs.write_hl(addr); 
                            break;
                    };

                    _step = 0;
                    return true;
            }
            return false;
        }

        //---------------------------------------------------------------------------------------------
        // プログラムカウンタが指す場所から読み取られる16bitが指す場所から読み取られる8bit
        // Dだと3サイクル、DEFは2サイクル
        inline bool read8(Peripherals &bus, Direct8 src, uint8_t &val){
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
                    val = bus.read(this->interrupts, _val16);                      // 作成したアドレスの値を読む
                    _step = 0;
                    return true;
            };

            return false;
        }
        inline bool write8(Peripherals &bus, Direct8 dst, uint8_t val){
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
                    bus.write(this->interrupts, _val16, val);                      // 作成したアドレスの値書く
                    _step = 0;
                    return true;
            };
            return false;
        }

        
        
        //---------------------------------------------------------------------------------------------
        // NOP命令、何もしない
        inline void nop(Peripherals &bus){
            this->fetch(bus);
        }

        //---------------------------------------------------------------------------------------------
        // ld d s ： s の値を d  に格納する
        template<typename T, typename U> void ld(Peripherals &bus, T dst, U src){
            static uint8_t _step = 0;
            static uint8_t _val8 = 0;
            switch(_step){
                case 0:
                    if(this->read8(bus, src, _val8)){
                        _step = 1;
                    }
                    break;
                case 1:
                    if(this->write8(bus, dst, _val8)){
                        this->fetch(bus);
                        _step = 0;
                    }
                    break;
            };
        }
        template<typename T, typename U> void ld16(Peripherals &bus, T dst, U src){
            static uint8_t _step = 0;
            static uint16_t _val16 = 0;
            switch(_step){
                case 0:
                    if(this->read16(bus, src, _val16)) {
                        _step = 1;
                    }
                    break;
                case 1:
                    if(this->write16(bus, dst, _val16)){
                        this->fetch(bus);
                        _step = 0;
                    }
                    break;
            };
        }

        //---------------------------------------------------------------------------------------------
        // CP s : Aレジスタからsの値を引き、レジスタ設定を行う
        template<typename T> void cp(Peripherals &bus, T src){
            static uint8_t _val8 = 0;

            if(this->read8(bus, src, _val8)){
                uint8_t _result = this->regs.a - _val8; 
                // フラグ設定
                this->regs.set_zf(_result == 0);                             // 演算結果が0の場合は1
                this->regs.set_nf(true);                                     // 無条件にtrue
                this->regs.set_hf((this->regs.a & 0xf) < (_val8 & 0xf));      // 4bit目からの繰り下がりが発生した場合に1
                this->regs.set_cf(this->regs.a < _val8);                      // 8bit目からの繰り下がりが発生した場合に1
                this->fetch(bus);
            }
        }

        //---------------------------------------------------------------------------------------------
        // bit num s : s の num bit目が0か1かを確認する
        template<typename T> void chkbit(Peripherals &bus, uint8_t bitsize, T src){
            static uint8_t _val8 = 0;
            if(this->read8(bus, src, _val8)){
                _val8 &= 1 << bitsize;
                this->regs.set_zf(_val8 == 0);      // Zフラグ、指定bitが0の場合は1にする
                this->regs.set_nf(false);           // Nフラグ、無条件に0
                this->regs.set_hf(true);            // Hフラグ、無条件に1
                this->fetch(bus);
            }
        }

        //---------------------------------------------------------------------------------------------
        // dec : sをデクリメント
        // 8bitの場合
        template<typename T> bool dec(Peripherals &bus, T src){
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

        //---------------------------------------------------------------------------------------------
        // INC s : sをインクリメントする
        // 8bit操作の時はフラグレジスタ操作も必要
        template<typename T> bool inc(Peripherals &bus, T src){
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

        template<typename T> bool inc16(Peripherals &bus, T src){
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

        //---------------------------------------------------------------------------------------------
        // RL s : sの値とCフラグを合わせた9bitの値を左に回転 = 1bit左シフト、Cフラグを最下位bitにセットする
        template<typename T> bool rl(Peripherals &bus, T src){
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

        //---------------------------------------------------------------------------------------------
        // push ：　16bitの値を、スタックポインタをデクリメントした後にスタックポインタが指すアドレスに値を格納する
        // 3サイクル
        inline bool push16(Peripherals &bus, uint16_t val){
            static uint8_t _step = 0;

            switch(_step){
                case 0:
                    // メモリアクセス回数 + 1
                    _step = 1;
                    return false;
                case 1:
                    // SPをデクリメントし、SPが指すアドレスに値を書き込む（H）
                    this->regs.sp -= 1;
                    bus.write(this->interrupts, this->regs.sp, (uint8_t)(val >> 8));
                    _step = 2;
                    return false;
                case 2:
                    // 下位ビット
                    this->regs.sp -= 1;
                    bus.write(this->interrupts, this->regs.sp, (uint8_t)(val & 0xFF));
                    _step = 3;
                    return false;
                case 3:
                    _step = 0;
                    return true;
            }
            return false;
        }
        // 4サイクル固定
        inline bool push(Peripherals &bus, Reg16 src){
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

        //---------------------------------------------------------------------------------------------
        // pop : 16bitの値をスタックからpop
        // スタックポインタが指すアドレスに格納されている値をレジスタに格納した後に，スタックポインタをインクリメント
        inline bool pop16(Peripherals &bus, uint16_t &val){
            static uint8_t _step = 0;
            static uint8_t _lo = 0, _hi = 0;

            switch(_step){
                case 0:
                    // 下位8bitの値取得し、SPをインクリメント
                    _lo = bus.read(this->interrupts, this->regs.sp);
                    this->regs.sp += 1;
                    _step = 1;
                    break;
                case 1:
                    // 上位8bitの値取得し、SPをインクリメント
                    _hi = bus.read(this->interrupts, this->regs.sp);
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
        inline bool pop(Peripherals &bus, Reg16 dst){
            static uint16_t _val = 0;
            
            if(this->pop16(bus, _val)){
                // 取り出した値をレジスタに書き込み、サイクル消費しない
                this->write16(bus, dst, _val);
                this->fetch(bus);
                return true;
            }

            return false;
        }

        //---------------------------------------------------------------------------------------------
        // call ：　プログラムカウンタの値をスタックにpushし、その後元のプログラムカウンタに戻す
        // 6サイクル固定
        inline bool call(Peripherals &bus){
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

        //---------------------------------------------------------------------------------------------
        // JP : PCに値を格納する = ジャンプする
        inline void jp(Peripherals &bus){
            static uint8_t _step = 0;
            static uint16_t _val16 = 0;

            switch(_step){
                case 0:
                    if(this->read16(bus, this->imm16, _val16)){
                        this->regs.pc = _val16;
                        _step = 1;
                        break;
                    }
                    break;
                case 1:
                    _step = 0;
                    this->fetch(bus);
                    break;
            };
        }

        //---------------------------------------------------------------------------------------------
        // JR : プログラムカウンタに値を加算する
        inline void jr(Peripherals &bus){
            static uint8_t _step = 0;
            static uint8_t _val8 = 0;

            switch(_step){
                case 0:
                    if(this->read8(bus, this->imm8, _val8)){
                        this->regs.pc += (int8_t)_val8;
                        _step = 1;
                    }
                    break;
                case 1:
                    _step = 0;
                    this->fetch(bus);
                    break;
            };
        }


        //---------------------------------------------------------------------------------------------
        // JR c : フラグがcを満たしていればJR命令（プログラムカウンタに加算）を行う
        inline bool cond(Peripherals &bus, Cond c){
            switch(c){
                case Cond::NZ: return !this->regs.zf();     // not Zフラグ
                case Cond::Z: return this->regs.zf();       // Zフラグ
                case Cond::NC: return !this->regs.cf();     // not Cフラグ
                case Cond::C: return this->regs.cf();       // Cフラグ
            }
            return true;
        }
        inline void jr_c(Peripherals &bus, Cond c){
            static uint8_t _step = 0;
            static uint8_t _val8 = 0;

            RE_ACTION:
            switch(_step){
                case 0:
                    if(this->read8(bus, this->imm8, _val8)) {
                        if(this->cond(bus, c)){
                            this->regs.pc += (uint16_t)((int8_t)_val8);
                            _step = 1;                      // ジャンプの場合はサイクル数+1
                        }
                        else _step = 2;
                        //goto RE_ACTION;
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

        //---------------------------------------------------------------------------------------------
        // RET : return
        // 16bitの値をプログラムカウンタに代入する、4サイクル
        inline void ret(Peripherals &bus){
            static uint8_t _step = 0;
            static uint16_t _val16 = 0;

            switch(_step){
                case 0:
                    if(this->pop16(bus, _val16)){
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


        //---------------------------------------------------------------------------------------------
        // RETI
        // RETに加え割り込みレジスタを有効にする
        inline void reti(Peripherals &bus){
            static uint8_t _step = 0;
            static uint16_t _val16 = 0;

            switch(_step){
                case 0:
                    if(this->pop16(bus, _val16)){
                        this->regs.pc = _val16;
                        _step = 1;
                    }
                    break;
                case 1:
                    _step = 0;
                    this->interrupts.ime = true;
                    this->fetch(bus);
                    break;
            };
        }

        //---------------------------------------------------------------------------------------------
        // EI
        // 割り込みレジスタを有効にする
        inline void ei(Peripherals &bus){
            this->fetch(bus);                   // fetchが先
            this->interrupts.ime = true;
        } 

        //---------------------------------------------------------------------------------------------
        // di
        // 割り込みレジスタを有効にする
        inline void di(Peripherals &bus){
            this->interrupts.ime = true;
            this->fetch(bus);
        } 

        //---------------------------------------------------------------------------------------------
        // call_isr
        // 割り込み処理
        inline void call_isr(Peripherals &bus){
            static uint8_t _step = 0;

            switch(_step){
                case 0:
                    if(this->pop16(bus, this->regs.pc)){
                        // 割り込み処理
                        _step = 1;
                    }
                    break;
                case 1:
                    _step = 0;
                    this->interrupts.ime = false;
                    this->fetch(bus);
                    break;
            };
        }









        

    public:
        // 変数
        Ctx ctx;
        Imm8 imm8;
        Imm16 imm16;
        Registers regs;
        Interrupts interrupts;
        uint32_t cycle;
        uint16_t dVal;
        uint8_t step;
        uint16_t val16;


        // コンストラクタ
        Cpu(){
            this->cycle = 0;
            this->step = 0;
            this->val16 = 0;
        }

        // 16bit命令
        inline void cb_decode(Peripherals &bus){
            switch(this->ctx.opecode){
                case 0x10: this->rl(bus, Reg8::B); break;                       // 2サイクル
                case 0x11: this->rl(bus, Reg8::C); break;                       // 2サイクル
                case 0x12: this->rl(bus, Reg8::D); break;
                case 0x6C: this->chkbit(bus, 5, Reg8::H); break;                // 2サイクル
            }
        }
        
        // CPUのエミュレート
        inline void emulate_cycle(Peripherals &bus){
            // 割り込み処理
            if(this->ctx.int_flag){
                this->call_isr(bus);

            } else {
                // 16bit命令
                if (this->ctx.cb) {
                    this->cb_decode(bus);
                    return;
                }

                // 8bit命令
                switch(this->ctx.opecode){
                    
                    case 0x00: this->nop(bus); break;
                    case 0x1A: this->ld(bus, Reg8::A, Indirect::DE); break;         // 2サイクル
                    case 0x3E: this->ld(bus, Reg8::A, this->imm8); break;           // 2サイクル
                    case 0x06: this->ld(bus, Reg8::B, this->imm8); break;
                    case 0x0E: this->ld(bus, Reg8::C, this->imm8); break;           // 2サイクル
                    case 0x2E: this->ld(bus, Reg8::L, this->imm8); break;
                    
                    case 0x78: this->ld(bus, Reg8::A, Reg8::B); break;
                    case 0x79: this->ld(bus, Reg8::A, Reg8::C); break;              // 1サイクル
                    case 0x7A: this->ld(bus, Reg8::A, Reg8::D); break;
                    case 0x7B: this->ld(bus, Reg8::A, Reg8::E); break;
                    case 0x7C: this->ld(bus, Reg8::A, Reg8::H); break;
                    case 0x7D: this->ld(bus, Reg8::A, Reg8::L); break;
                    
                    case 0x47: this->ld(bus, Reg8::B, Reg8::A); break;              // 1サイクル

                    case 0x57: this->ld(bus, Reg8::D, Reg8::A); break;
                    case 0x12: this->ld(bus, Indirect::DE, Reg8::A); break;
                    case 0x22: this->ld(bus, Indirect::HLI, Reg8::A); break;        // 2サイクル
                    case 0x2A: this->ld(bus, Reg8::A, Indirect::HLI); break;
                    case 0x32: this->ld(bus, Indirect::HLD, Reg8::A); break;
                    case 0xEA: this->ld(bus, Direct8::D, Reg8::A); break;
                    case 0xE0: this->ld(bus, Direct8::DFF, Reg8::A); break;         // 2サイクル
                    
                    case 0x01: this->ld16(bus, Reg16::BC, this->imm16); break;
                    case 0x11: this->ld16(bus, Reg16::DE, this->imm16); break;
                    case 0x21: this->ld16(bus, Reg16::HL, this->imm16); break;      // 3サイクル
                    case 0x31: this->ld16(bus, Reg16::SP, this->imm16); break;      // 3サイクル
                    case 0x3D: this->dec(bus, Reg8::A); break;                      // 1サイクル
                    case 0x05: this->dec(bus, Reg8::B); break;
                    case 0x0D: this->dec(bus, Reg8::C); break;
                    case 0x15: this->dec(bus, Reg8::D); break;
                    case 0x1C: this->inc(bus, Reg8::E); break;
                    case 0x14: this->inc(bus, Reg8::D); break;
                    case 0x23: this->inc16(bus, Reg16::HL); break;
                    case 0x13: this->inc16(bus, Reg16::DE); break;
                    case 0xF5: this->push(bus, Reg16::AF); break;                   // 4サイクル
                    case 0xC5: this->push(bus, Reg16::BC); break;
                    case 0xE5: this->push(bus, Reg16::HL); break;
                    case 0xF1: this->pop(bus, Reg16::AF); break;                    // 3サイクル
                    case 0xC1: this->pop(bus, Reg16::BC); break;
                    case 0xC3: this->jp(bus); break;
                    case 0x18: this->jr(bus); break;
                    case 0x28: this->jr_c(bus, Cond::Z); break;                     // 2-3サイクル
                    case 0x20: this->jr_c(bus, Cond::NZ); break;                    // 2-3サイクル
                    case 0xCD: this->call(bus); break;                              // 
                    case 0xC9: this->ret(bus); break;
                    case 0xCB: this->cb_prefixed(bus); break;
                    case 0xFE: this->cp(bus, this->imm8); break;
                    case 0xF3: this->di(bus); break;
                }
            }
        }
};





#endif
