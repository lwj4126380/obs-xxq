#include "Airplay2Head.h"
#include "FgAirplayServer.h"

void *fgServerStart(const char serverName[AIRPLAY_NAME_LEN],
		    unsigned int raopPort, unsigned int airplayPort,
		    IAirServerCallback *callback)
{
	FgAirplayServer *pServer = new FgAirplayServer();
	int ret = pServer->start(serverName, raopPort, airplayPort, callback);
	if (ret < 0) {
		fgServerStop(pServer);
		return NULL;
	} else
		return pServer;
}

void fgServerStop(void *handle)
{
	if (handle != NULL) {
		FgAirplayServer *pServer = (FgAirplayServer *)handle;
		pServer->stop();

		delete pServer;
		pServer = NULL;
	}
}