#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <dlfcn.h>
#include <err.h>
#include <ogg/ogg.h>

extern char **environ;

static int (*next_ogg_stream_packetin)(ogg_stream_state *os, ogg_packet *op);

int ogg_stream_packetin(ogg_stream_state *os, ogg_packet *op) {
	if (next_ogg_stream_packetin == NULL)
		next_ogg_stream_packetin =
			(int (*)())dlsym(RTLD_NEXT, "ogg_stream_packetin");

	/* only mangle the comment header */
	if (op->packetno != 1)
		return next_ogg_stream_packetin(os, op);

	/* nothing to be done if we don't *have* a mangler */
	char *mangler = getenv("OGG_MANGLER");
	if (mangler == NULL)
		return next_ogg_stream_packetin(os, op);

	int pc[2], cp[2];
	if (pipe(pc) == -1 || pipe(cp) == -1)
		err(12, "pipe");

	pid_t pid1 = fork();
	if (pid1 == -1)
		err(12, "fork");

	if (pid1 == 0) {
		if (close(cp[1]) == -1 || close(pc[0]) == -1)
			err(22, "close");
		if (dup2(cp[0], 0) == -1 || dup2(pc[1], 1) == -1)
			err(22, "dup2");
		execl("/bin/sh", "sh", "-c", mangler,
				/* XXX obviously evil */
				environ[-3], environ[-2], (char *)0);
		err(22, "execl");
	}

	if (close(cp[0]) == -1 || close(pc[1]) == -1)
		err(12, "close");

	uint64_t len = op->bytes;

	pid_t pid2 = fork();
	if (pid2 == -1)
		err(12, "fork");

	if (pid2 == 0) {
		if (close(pc[0]) == -1)
			err(32, "close");
		/* these don't need to succeed */
		write(cp[1], &len, 8);
		write(cp[1], op->packet, len);
		_exit(0);
	}

	if (close(cp[1]) == -1)
		err(12, "close");

	if (read(pc[0], &len, 8) != 8)
		err(12, "read");
	void *old_packet = op->packet;
	op->packet = malloc(len);
	if (op->packet == NULL)
		err(12, "malloc");
	op->bytes = len;

	do {
		ssize_t r = read(pc[0], op->packet+op->bytes-len, len);
		if (r == -1)
			err(12, "read");
		else if (r == 0)
			errx(11, "short read");
		len -= r;
	} while (len > 0);

	/* nothing to be done on error here */
	close(pc[0]);

	int r = next_ogg_stream_packetin(os, op);
	free(op->packet);
	op->packet = old_packet;

	while (waitpid(pid1, NULL, 0) == -1)
		/* only EINTR is possible */;
	while (waitpid(pid2, NULL, 0) == -1)
		/* only EINTR is possible */;

	return r;
}
