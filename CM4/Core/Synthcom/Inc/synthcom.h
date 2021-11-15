#ifndef __SYNTHCOM_H
#define __SYNTHCOM_H

#include "synthcom_packets.h"

void SynthCom_Init(void);
void SynthCom_receive(void);
void SynthCom_transmit(SynthCom_PacketType packet_type, void* data);

#endif
