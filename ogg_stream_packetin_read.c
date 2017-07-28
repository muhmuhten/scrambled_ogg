#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <dlfcn.h>
#include <err.h>
#include <ogg/ogg.h>

extern char **environ;

static int (*next_ogg_stream_packetin)(ogg_stream_state *os, ogg_packet *op);

int ogg_stream_packetin(ogg_stream_state *os, ogg_packet *op) {
	if (next_ogg_stream_packetin == NULL)
		next_ogg_stream_packetin =
			(int (*)())dlsym(RTLD_NEXT, "ogg_stream_packetin");

	if (op->packetno != 1)
		return next_ogg_stream_packetin(os, op);

	char *file = getenv("TAGS_FILE");
	if (file == NULL)
		errx(11, "ogg_stream_packetin/read loaded, but no TAGS_FILE");

	int fd = open(file, O_RDONLY);
	if (fd == -1)
		err(12, "open %s", file);

	uint64_t len = op->bytes;
	if (read(fd, &len, 8) != 8)
		err(12, "read");
	void *old_packet = op->packet;
	op->packet = malloc(len);
	if (op->packet == NULL)
		err(12, "malloc");
	op->bytes = len;

	do {
		ssize_t r = read(fd, op->packet+op->bytes-len, len);
		if (r == -1)
			err(12, "read");
		else if (r == 0)
			errx(11, "short read");
		len -= r;
	} while (len > 0);

	/* nothing to be done on error here */
	close(fd);

	int r = next_ogg_stream_packetin(os, op);
	free(op->packet);
	op->packet = old_packet;
	return r;
}
