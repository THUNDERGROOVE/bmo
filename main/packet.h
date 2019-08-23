#ifndef __PACKET_H__
#define __PACKET_H__

#include <stddef.h>

#ifndef __xtensa__
#include "ext_gpio.h"
#else
#include "driver/gpio.h"
#endif

#include <stdint.h>

typedef enum {
              PacketPing,
              PacketReboot,
              PacketCMD,
              PacketEnterOTA,
              PacketSetGPIO,
              PacketPWMInit,
              PacketPWMSet,
              PacketOK,
              PacketFail,
              PacketOTAEnd,
              PacketOTAData,

              PacketNULL,
              PacketMAX
} PacketType;

static const char *PacketType_Strings[] = {
                                           "PacketPing",
                                           "PacketReboot",
                                           "PacketCMD",
                                           "PacketEnterOTA",
                                           "PacketSetGPIO",
                                           "PacketPWMInit",
                                           "PacketPWMSet",
                                           "PacketOK",
                                           "PacketFail",
                                           "PacketOTAEnd",
                                           "PacketOTAData",
};

const char *PacketTypeString(PacketType type);

typedef struct {
    uint32_t size;
    char chunk[4096];
} OTAData;

typedef struct {
    uint32_t type;
    union {
        OTAData ota_data;
    };
} Packet;


int WritePacket(int s, Packet *p);
int ReadPacket(int s, Packet *p);

#endif
