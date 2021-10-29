/*
 * byteorder.h
 *
 * Author : gfabiano
 */ 


#ifndef byteorder_h
#define byteorder_h

#ifndef CANBUS_BYTE_ORDER
#define CANBUS_BYTE_ORDER __ORDER_BIG_ENDIAN__
#endif


// Fast in hardware byteorder swap for canbus
namespace byteorder
{
    // can(bus) to host long
    inline
    uint32_t ctohl(uint32_t t_val)
    {
        #if CANBUS_BYTE_ORDER == __ORDER_BIG_ENDIAN__
        asm( "rev %1, %0"
             : "=r" (t_val)
             : "r"  (t_val)
        );
        #endif
        return t_val;
    }

    // host to can(bus) long
    constexpr auto htocl = ctohl;


    // can(bus) to host short
    inline
    uint16_t ctohs(uint16_t t_val)
    {
        #if CANBUS_BYTE_ORDER == __ORDER_BIG_ENDIAN__
			uint8_t tmp = (t_val >> 8) & 0xff;
			t_val = (t_val << 8) | tmp;
        #endif
        return t_val;
    }

    // host to can(bus) short
    constexpr auto htocs = ctohs;
}

#endif

