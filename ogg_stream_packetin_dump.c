#include <unistd.h>
#include <ogg/ogg.h>

int ogg_stream_packetin(ogg_stream_state *os, ogg_packet *op) {
	if (op->packetno == 1) {
		write(4, &op->bytes, 8);
		write(4, op->packet, op->bytes);
		_exit(0);
	}
	return 0;
}
