#include "packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> 

#ifdef __WIN32__
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif

// TODO: use Read/WritePacket


int nova_connect(const char *hostname, int port) {
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		printf("Failed to connect to nova\n");
		return -1;
	}

	struct hostent *server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr, " ERROR no such server by hostname\n");
		return -1;
	}

	struct sockaddr_in server_addr;
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		  (char *)&server_addr.sin_addr.s_addr, server->h_length);
	server_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		fprintf(stderr, " ERROR in `connect()`\n");
		return -1;
	}

	printf("Sucessfully connected to nova\n");
	return sock;
}

int update_firm(int argc, char **argv) {
	if (argc != 4) {
		fprintf(stderr, "usage: %s ota <hostname> <port>\n", argv[0]);
		return 0;
	}

	printf(" Reading firmware....");
	FILE *f = fopen("../../build/bmo.bin", "rb");
	if (f == NULL) {
		fprintf(stderr, " ERROR could not open ../../build/nova.bin");
		return -1;
	}

	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);
	rewind(f);
	char *firmware = malloc(fsize);
	fread(firmware, 1, fsize, f);
	printf(" done\n");
	printf(" firmware size: %d\n", fsize);

	fclose(f);

	char *hostname = argv[2];
	int port = atoi(argv[3]);

	int sock = nova_connect(hostname, port);

	Packet packet;

	bzero(&packet, sizeof(packet));

	packet.type = PacketEnterOTA;

	printf(" Sending PacketEnterOTA...");

	// Set nova to ESP mode
	int write_size = WritePacket(sock, &packet);
	if (write_size < 0) {
		fprintf(stderr, " ERROR sending PacketEnterOTA\n");
		return 0;
	}

	
	// Check read status
	int read_size = ReadPacket(sock, &packet);
	if (read_size < 0) {
		fprintf(stderr, " ERROR checking read status\n");
		return 0;
	}
	printf(" OK\n");

	size_t pos = 0;
	while (1) {
		bzero(&packet, sizeof(packet));
		packet.type = PacketOTAData;
		int bytes_left = fsize - pos;
		if (bytes_left >= 4096) {
			packet.ota_data.size = 4096;
		} else {
			packet.ota_data.size = bytes_left;
		}
		if (bytes_left <= 0) {
			break;
		}
		fprintf(stdout, " [%d/%d] Updating Firmware...\n",
				bytes_left, fsize);
		fflush(stdout);


		memcpy(packet.ota_data.chunk, firmware + pos,
			   packet.ota_data.size);

		for (int i = 0; i < 16; i++) {
			printf(" %02X", (unsigned int)(packet.ota_data.chunk[i] && 0xFF));
		}
		printf("\n");

		printf("Sending packet type: %s\n", PacketTypeString(packet.type));
		
		write_size = WritePacket(sock, &packet);
		if (write_size < 0) {
			fprintf(stderr, "\r ERROR sending PacketOTAData\n");
			return -1;
		}
		pos += packet.ota_data.size;
	}

	printf("Waiting for nova to catch up...\n");
	sleep(5);

	printf("Sending PacketOTAEnd\n");

	packet.type = PacketOTAEnd;
	write_size = WritePacket(sock, &packet);
	if (write_size < 0) {
		fprintf(stderr, " ERROR sending PacketOTAEnd\n");
		return -1;
	}

	printf("Waiting for nova to catch up...\n");
	sleep(1);

	bzero(&packet, sizeof(packet));
	packet.type = PacketReboot;

	printf("Sending PacketReboot\n");
	write_size = WritePacket(sock, &packet);
	if (write_size < 0) {
		fprintf(stderr, " ERROR sending PacketEnterOTA\n");
		return 0;
	}

	printf("Waiting for nova to catch up...\n");
	sleep(1);

	close(sock);
	return 0;
}

int main(int argc, char **argv) {
    printf("%d\n", sizeof(Packet));
	if (argc < 2) {
		printf("usage: %s\n"
			   "     : ota <hostanme> <port>\n"
			   "     : gpioset <hostname> <port> <pin> <state>", argv[0]);
		return 0;
	}


	char *cmd = argv[1];
	if (strcmp(cmd, "ota") == 0) {
		return update_firm(argc, argv);
	} else {
		printf("unknown cmd: %s\n", cmd);
		return 0;
	}
	return 0;
}
