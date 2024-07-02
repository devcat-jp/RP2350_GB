#ifndef CPU_HPP
#define CPU_HPP

#include "peripherals.hpp"
#include "registers.hpp"

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
};


class Cpu{
    private:
        // フェッチ
        inline void fetch(Peripherals &bus){
            this->ctx.opecode = bus.read(this->regs.pc);
            this->regs.pc += 1;
            this->ctx.cb = false;
        }

        inline void cb_decode(Peripherals &bus){
            switch(this->ctx.opecode){
                case 0x10: this->rl(bus, Reg8::B); break;                       // 2サイクル
                case 0x11: this->rl(bus, Reg8::C); break;                       // 2サイクル
                case 0x6C: this->chkbit(bus, 5, Reg8::H); break;                // 2サイクル
            }
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
        //
        bool read8(Peripherals &bus, Reg8 src, uint8_t &val);
        bool read8(Peripherals &bus, Imm8 src, uint8_t &val);
        bool read8(Peripherals &bus, Indirect src, uint8_t &val);
        bool read8(Peripherals &bus, Direct8 src, uint8_t &val);
        bool read16(Peripherals &bus, Reg16 src, uint16_t &val);
        


        inline bool read16(Peripherals &bus, Imm16 src, uint16_t &val){
            static uint8_t _step = 0;
            uint8_t _tmp;

            switch(_step){
                case 0:
                    val = bus.read(this->regs.pc);
                    this->regs.pc += 1;
                    _step = 1;
                    return false;
                case 1:
                    _tmp = bus.read(this->regs.pc);
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


        bool write8(Peripherals &bus, Reg8 dst, uint8_t val);
        bool write8(Peripherals &bus, Indirect dst, uint8_t val);
        bool write8(Peripherals &bus, Direct8 dst, uint8_t val);
        
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

        bool cond(Peripherals &bus, Cond cond);
        //
        void nop(Peripherals &bus);
        void jp(Peripherals &bus);
        void jr(Peripherals &bus);
        void jr_c(Peripherals &bus, Cond c);
        bool push16(Peripherals &bus, uint16_t val);
        bool push(Peripherals &bus, Reg16 src);
        bool pop16(Peripherals &bus, uint16_t &val);
        bool pop(Peripherals &bus, Reg16 dst);
        bool call(Peripherals &bus);
        void ret(Peripherals &bus);
        template<typename T, typename U> void ld(Peripherals &bus, T dst, U src);
        

        // ld16 d s ： s の値を d  に格納する
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

        template<typename T> void chkbit(Peripherals &bus, uint8_t bitsize, T src);
        template<typename T> void cp(Peripherals &bus, T src);
        template<typename T> bool dec(Peripherals &bus, T src);
        template<typename T> bool inc(Peripherals &bus, T src);
        template<typename T> bool inc16(Peripherals &bus, T src);
        template<typename T> bool rl(Peripherals &bus, T src);
        

    public:
        // 変数
        Ctx ctx;
        Imm8 imm8;
        Imm16 imm16;
        Registers regs;
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
        
        // CPUのエミュレート
        void emulate_cycle(Peripherals &bus){
            // 16bit命令
            if (this->ctx.cb) {
                this->cb_decode(bus);
                return;
            }

            // 8bit命令
            switch(this->ctx.opecode){
                
                case 0x00: this->nop(bus); break;
                ///*
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
                case 0x01: this->ld16(bus, Reg16::BC, this->imm16); break;
                case 0x11: this->ld16(bus, Reg16::DE, this->imm16); break;
                case 0x21: this->ld16(bus, Reg16::HL, this->imm16); break;      // 3サイクル
                case 0x31: this->ld16(bus, Reg16::SP, this->imm16); break;      // 3サイクル
                case 0x3D: this->dec(bus, Reg8::A); break;                      // 1サイクル
                case 0x05: this->dec(bus, Reg8::B); break;
                case 0x0D: this->dec(bus, Reg8::C); break;
                case 0x15: this->dec(bus, Reg8::D); break;
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
                //*/
                //default: this->ld16(bus, Reg16::SP, this->imm16); break;      // 3サイクル
            }
        }
};





#endif
