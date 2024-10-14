
#ifndef INTERRUPTS
#define INTERRUPTS


// 割り込みで使用する定数
// PPUなどは再現しないで大丈夫？
const uint8_t TIMER = 1 << 2;
const uint8_t JOYPAD = 1 << 4;



class Interrupts {
    private:
    public:
        bool ime;
        uint8_t int_flags;
        uint8_t int_enable;

    // 割り込み要求
    void irq(uint8_t val){
        this->int_flags |= val;
    }

    inline uint8_t read(uint16_t addr){
        if(addr == 0xFF0F) return this->int_flags;
        if(addr == 0xFFFF) return this->int_enable;
        return 0xFF;
    }

    inline void write(uint16_t addr, uint8_t val){
        if(addr == 0xFF0F) this->int_flags = val;
        if(addr == 0xFFFF) this->int_enable = val;
    }

};




#endif