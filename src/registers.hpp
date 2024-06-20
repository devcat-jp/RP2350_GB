#ifndef REGISTERS_HPP
#define REGISTERS_HPP

class Registers{
    private:
    public:
        Registers();
        uint8_t a;
        uint8_t b;
        uint8_t c;
        uint8_t d;
        uint8_t e;
        uint8_t f;
        uint8_t h;
        uint8_t l;
        uint16_t pc;
        uint16_t sp;

        uint16_t af();
        uint16_t bc();
        uint16_t de();
        uint16_t hl();
        void write_af(uint16_t val);
        void write_bc(uint16_t val);
        void write_de(uint16_t val);
        void write_hl(uint16_t val);
        bool zf();
        bool nf();
        bool hf();
        bool cf();
        void set_zf(bool flag);
        void set_nf(bool flag);
        void set_hf(bool flag);
        void set_cf(bool flag);
};





#endif