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
        void fetch(Peripherals &bus);
        void decode(Peripherals &bus);
        void cb_decode(Peripherals &bus);
        void cb_prefixed(Peripherals &bus);
        //
        bool read8(Peripherals &bus, Reg8 src, uint8_t &val);
        bool read8(Peripherals &bus, Imm8 src, uint8_t &val);
        bool read8(Peripherals &bus, Indirect src, uint8_t &val);
        bool read8(Peripherals &bus, Direct8 src, uint8_t &val);
        bool read16(Peripherals &bus, Reg16 src, uint16_t &val);
        bool read16(Peripherals &bus, Imm16 src, uint16_t &val);
        bool write8(Peripherals &bus, Reg8 dst, uint8_t val);
        bool write8(Peripherals &bus, Indirect dst, uint8_t val);
        bool write8(Peripherals &bus, Direct8 dst, uint8_t val);
        bool write16(Peripherals &bus, Reg16 dst, uint16_t val);
        bool cond(Peripherals &bus, Cond cond);
        //
        void nop(Peripherals &bus);
        void jr_c(Peripherals &bus, Cond c);
        template<typename T, typename U> void ld(Peripherals &bus, T dst, U src);
        template<typename T, typename U> void ld16(Peripherals &bus, T dst, U src);
        template<typename T> void chkbit(Peripherals &bus, uint8_t bitsize, T src);

    public:
        Cpu();
        Ctx ctx;
        Imm8 imm8;
        Imm16 imm16;
        Registers regs;
        uint32_t cycle;
        uint16_t dVal;

        void emulate_cycle(Peripherals &bus);
        
};





#endif
