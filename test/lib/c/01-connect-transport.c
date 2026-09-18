#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <mosquitto.h>

static int on_transport_open(struct mosquitto *mosq, void *obj, int *sock)
{
	struct addrinfo hints, *ainfo, *rp;
	const char *port = obj;
	int s = -1;

	(void)mosq;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo("localhost", port, &hints, &ainfo)){
		return MOSQ_ERR_EAI;
	}
	for(rp = ainfo; rp != NULL; rp = rp->ai_next){
		s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(s == -1){
			continue;
		}
		if(connect(s, rp->ai_addr, rp->ai_addrlen) == 0){
			break;
		}
		close(s);
		s = -1;
	}
	freeaddrinfo(ainfo);
	if(s == -1){
		return MOSQ_ERR_ERRNO;
	}
	*sock = s;
	return MOSQ_ERR_SUCCESS;
}

static int run = -1;


static void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	(void)obj;

	if(rc){
		exit(1);
	}else{
		mosquitto_disconnect(mosq);
	}
}


static void on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
	(void)mosq;
	(void)obj;

	run = rc;
}


int main(int argc, char *argv[])
{
	int rc;
	struct mosquitto *mosq;

	if(argc < 2){
		return 1;
	}

	mosquitto_lib_init();

	mosq = mosquitto_new("01-connect-transport", true, argv[1]);
	if(mosq == NULL){
		return 1;
	}
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_disconnect_callback_set(mosq, on_disconnect);

	/* Must fail without a transport open callback */
	rc = mosquitto_connect_transport(mosq, 60, NULL);
	if(rc != MOSQ_ERR_INVAL){
		return 1;
	}

	mosquitto_transport_open_callback_set(mosq, on_transport_open);
	rc = mosquitto_connect_transport(mosq, 60, NULL);
	if(rc != MOSQ_ERR_SUCCESS){
		return rc;
	}

	while(run == -1){
		mosquitto_loop(mosq, -1, 1);
	}
	mosquitto_destroy(mosq);

	mosquitto_lib_cleanup();
	return run;
}
