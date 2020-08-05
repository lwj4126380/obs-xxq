#include "CAirServer.h"
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

int main(int argv, char *argc[])
{
	SetErrorMode(SEM_FAILCRITICALERRORS);
	_setmode(_fileno(stdin), O_BINARY);
	freopen("/dev/null", "w", stderr);

	CAirServer server;
	if (!server.start())
		return -1;

	uint8_t buf[1024] = {0};
	while (true) {
		int read_len =
			fread(buf, 1, 1024,
			      stdin); // read 0 means parent has been stopped
		if (read_len) {
			if (buf[0] == 1) {
				server.stop();
				break;
			}
		} else {
			server.stop();
			break;
		}
	}

	return 0;
}