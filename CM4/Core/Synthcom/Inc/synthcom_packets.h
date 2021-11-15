#ifndef SYNTHCOM_PACKETS_H_
#define SYNTHCOM_PACKETS_H_

#include <assert.h>

#define MAX_PAYLOAD 256U
#define PAYLOAD {                           \
                    [SYNTHCOM_TEST] = 56U,  \
                }                           \

/* Possible packet types */
typedef enum SynthCom_PacketType_T
{
    SYNTHCOM_TEST,

    SynthCom_PacketType_end
} SynthCom_PacketType;

/* Structure of the IDENTIFY.request packet payload */
typedef struct SynthComPacket_Test_T
{
    uint32_t data_x;
    uint32_t data_y;
    char buff[40];
    double angle;
} SynthComPacket_Test;
/* static assertion to check that the structure is indeed packed */
static_assert(56U == sizeof(SynthComPacket_Test), "52U != sizeof(SynthComPacket_Test)");

#endif /* AMCOM_PACKETS_H_ */
