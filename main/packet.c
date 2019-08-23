#include "packet.h"

#ifdef __xtensa__
#  include "lwip/sockets.h"
#else
#  include <unistd.h>
#endif
#include <stdio.h>

const char *PacketTypeString(PacketType type) {
	if (type >= 0 && type < PacketNULL) {
		return PacketType_Strings[type];
	}

	return "PacketUnknown";
};

int WritePacket(int s, Packet *p) {
	#ifdef __xtensa__
	return lwip_write(s, p, sizeof(Packet));
	#else
	return write(s, p, sizeof(Packet));
	#endif
}

int ReadPacket(int s, Packet *p) {
	#ifdef __xtensa__
    int n = 0;
    while (n != sizeof(Packet)) {
        int r = lwip_read(s, (char *)p+n, sizeof(Packet) - n);
        if (r <= 0) {
            return r;
        }
        n += r;
    }
        return n;
#else
        return read(s, p, sizeof(Packet));
#endif
    }
