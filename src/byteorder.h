#ifndef GF_BYTEORDER
#define GF_BYTEORDER

#include <stdint.h>

#ifndef CANBUS_BYTE_ORDER
#define CANBUS_BYTE_ORDER __ORDER_BIG_ENDIAN__
#endif

namespace byteorder
{
    // can(bus) to host long
    inline
    uint32_t ctohl(uint32_t t_val)
    {
        #if CANBUS_BYTE_ORDER == __ORDER_BIG_ENDIAN__
        t_val = ((t_val & 0xff000000) >> 24) | ((t_val &0x00ff0000) >> 8) |
                ((t_val & 0x0000ff00) << 8) | ((t_val &0x000000ff) << 24);
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
        t_val = ((t_val & 0xff00) >> 8) | ((t_val &0x00ff) << 8);
        #endif
        return t_val;
    }

    // host to can(bus) short
    constexpr auto htocs = ctohs;
}

#endif