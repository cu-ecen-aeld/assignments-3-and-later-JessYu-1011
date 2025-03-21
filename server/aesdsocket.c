#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

#define MAX_LEN 1024
struct fd_data {
	struct addrinfo *res;
	int sockfd;
};

void signal_handle(int sig, siginfo_t *siginfo, void *context) {
	struct fd_data *data = (struct fd_data*) siginfo->si_value.sival_ptr;
	if(data && data->sockfd > 0) {
		close(data->sockfd);
	}
	if(data && data->res) {
		free(data->res);
	}
	syslog(LOG_INFO, "Caught signal,exiting");
	remove("/var/tmp/aesdsocketdata");
	exit(0);
}
bool check_newline(char s[], size_t size) {
	for(int i = 0 ; i < size; i++) {
		if(s[i] == '\n') {
			return true;
		}
	}
	return false;
}

int main(int argc, char *argv[]) {
	int is_daemon = 0;
	int status;
	struct addrinfo hints;
	struct addrinfo *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if(argc > 1 && strcmp(argv[1], "-d") == 0){
		is_daemon = 1;
	}

	// syslog
	openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_DAEMON);
	if((status = getaddrinfo(NULL, "9000", &hints, &res)) != 0) {
		syslog(LOG_ERR, "getaddrinfo error: %s", gai_strerror(status));
		freeaddrinfo(res);
		closelog();
		exit(1);
	}
	
	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1) {
		syslog(LOG_ERR, "socket error: %s", strerror(errno));
		freeaddrinfo(res);
		closelog();
		exit(1);
	}
	int optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
    	syslog(LOG_ERR, "setsockopt SO_REUSEADDR error: %s", strerror(errno));
   	    close(sockfd);
    	freeaddrinfo(res);
    	exit(1);
	}

	// signal handling
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = signal_handle;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
 	sigaction(SIGTERM, &sa, NULL);
	struct fd_data data = {.res=res, .sockfd=sockfd};
	union sigval value;
	value.sival_ptr = &data;

	if(bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		syslog(LOG_ERR, "bind error: %s", strerror(errno));
		close(sockfd);
		freeaddrinfo(res);
		closelog();
		exit(1);
	}
	//after binding, fork
	if(is_daemon) {
		pid_t pid, sid;
		pid = fork();
		if(pid < 0) {
			close(sockfd);
			freeaddrinfo(res);
			exit(1);
		}
		if(pid > 0) {
			close(sockfd);
			freeaddrinfo(res);
			exit(0);
		}
		sid = setsid();
		if(sid < 0) {
			close(sockfd);
			freeaddrinfo(res);
			exit(1);
		}
		if(chdir("/") < 0) {
			close(sockfd);
			freeaddrinfo(res);				
			exit(1);
		}
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}

	if(listen(sockfd, 10) == -1) {
		syslog(LOG_ERR, "listen error: %s", strerror(errno));
		close(sockfd);
		freeaddrinfo(res);
		closelog();
		exit(1);
	}
	
	int client_fd;
	struct sockaddr_storage client_addr;
	char client_ip[INET_ADDRSTRLEN];
	socklen_t addr_size = sizeof(client_addr);
	while(1) {
		FILE * fp = fopen("/var/tmp/aesdsocketdata", "a+");
		if(fp == NULL) {
			perror("Unable to open");
			fclose(fp);
			continue;
		}
		client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
		if(client_fd == -1){
			syslog(LOG_ERR, "accept error: %s", strerror(errno));
			fclose(fp);
			continue;
		}
		
		struct sockaddr_in *addr = (struct sockaddr_in *)&client_addr;
		inet_ntop(AF_INET, &addr->sin_addr, client_ip, sizeof(client_ip));
		syslog(LOG_INFO, "Accepted connection from %s", client_ip); 
		// keep receiving
		char inbuf[MAX_LEN];
		bool newline = false;
		while(!newline) {
			int nbytes = recv(client_fd, inbuf, sizeof(inbuf), 0);
			if(nbytes <= 0) {
				close(client_fd);
				break;
			}	
			newline = check_newline(inbuf, nbytes);
			fwrite(inbuf, sizeof(char), nbytes, fp);
		}
		fseek(fp, 0, SEEK_SET);  
		char fbuf[MAX_LEN];
    	size_t bytes_read;
    	while ((bytes_read = fread(fbuf, 1, sizeof(fbuf), fp)) > 0) {
        	ssize_t bytes_sent = send(client_fd, fbuf, bytes_read, 0);
        	if (bytes_sent == -1) {
            	syslog(LOG_ERR, "Send error: %s", strerror(errno));
            	break;
        	}
    	}
		// send back to the client
		fclose(fp);
		syslog(LOG_INFO, "Closed connection from %s", client_ip);
		close(client_fd);
	}
	close(sockfd);
	freeaddrinfo(res);
	closelog();
	return 0;
}
