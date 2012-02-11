#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <string.h>
#include  <stdio.h>
#include  <arpa/inet.h>
#define MAXBUF 256
#define PUERTO 5000
#define GROUP "224.0.1.1"

int main(void) {
	int s;
	struct sockaddr_in srv;
	char buf[MAXBUF];
	bzero(&srv, sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_port = htons(PUERTO);
	if (inet_aton(GROUP, &srv.sin_addr) < 0) {
		perror("inet_aton");
		return 1;
	}
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return 1;
	}
	while (fgets(buf, MAXBUF, stdin)) {
		//printf("input = %s", buf);
		if (sendto(s, buf, strlen(buf), 0,(struct sockaddr *)&srv, sizeof(srv)) < 0) {
			perror("sendfrom err");
		} else {
			fprintf(stdout, "%s: %s", GROUP, buf);
		}
	}
}

