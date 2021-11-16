#ifndef __SYNTHCOM_H
#define __SYNTHCOM_H

#include <stdbool.h>
#include "synthcom_packets.h"

void SynthCom_Init(void);
bool SynthCom_process(void);
bool SynthCom_transmit(SynthCom_PacketType packet_type, void* data);

#endif
