#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_error(char *errType){
	if(strcmp(errType, "socket") == 0){
		printf("Socket Error:\n");
		switch(errno){
			case EACCES:
				printf("Permission to create a sock of the Specified type and/or protocol is denied");
				exit(EXIT_FAILURE);
			default:
				printf("Unhandled Error when creating Socket");
				exit(EXIT_FAILURE);
		}
	} else if(strcmp(errType, "bind") == 0){
		printf("Binding Error:\n");
		switch(errno){
			case EACCES:
				printf("The address is protected, and the user is not the superuser\n");
				exit(EXIT_FAILURE);
			case EADDRINUSE:
				printf("The given address is already in use\n");
				exit(EXIT_FAILURE);
			case EBADF:
				printf("Sockfd is not a valid file descriptor\n");
				exit(EXIT_FAILURE);
			case EINVAL:
				printf("The socket is already bound to an address\n");
				exit(EXIT_FAILURE);
			case ENOTSOCK:
				printf("The file descriptor does sockfd does not refer to a socket\n");
				exit(EXIT_FAILURE);
			case EADDRNOTAVAIL:
				printf("A nonexistant interface was requested or the requested address was not local\n");
				exit(EXIT_FAILURE);
			case EFAULT:
				printf("addr points outside the user's accessible address space\n");
				exit(EXIT_FAILURE);
			case ELOOP:
				printf("Too many symbolic links were encountered in resolving addr\n");
				exit(EXIT_FAILURE);
			case ENAMETOOLONG:
				printf("addr is too long\n");
				exit(EXIT_FAILURE);
			case ENOENT:
				printf("A component in the directory prefix of the socket pathname does not exist\n");
				exit(EXIT_FAILURE);
			case ENOMEM:
				printf("Insufficient kernel memory was available\n");
				exit(EXIT_FAILURE);
			case ENOTDIR:
				printf("A component of the path prefix is not a directory\n");
				exit(EXIT_FAILURE);
			case EROFS:
				printf("The socket inode would reside on a read-only filesystem\n");
				exit(EXIT_FAILURE);
			default:
				printf("Unhandled Error when Binding\n");
				exit(EXIT_FAILURE);
		}
	} else if(strcmp(errType, "sendto") == 0){
		printf("Sendto Error:\t");
		switch(errno){
			case EACCES:
				printf("EACCES\n");
				exit(EXIT_FAILURE);
			case EAGAIN:
				printf("EAGAIN || EWOULDBLOCK\n");
				exit(EXIT_FAILURE);
			case EBADF:
				printf("EBADF\n");
				exit(EXIT_FAILURE);
			case ECONNRESET:
				printf("ECONNRESET\n");
				exit(EXIT_FAILURE);
			case EDESTADDRREQ:
				printf("EDESTADDREQ\n");
				exit(EXIT_FAILURE);
			case EFAULT:
				printf("EFAULT\n");
				exit(EXIT_FAILURE);
			case EINTR:
				printf("EINTR\n");
				exit(EXIT_FAILURE);
			case EINVAL:
				printf("EINVAL\n");
				exit(EXIT_FAILURE);
			case EISCONN:
				printf("EISCONN\n");
				exit(EXIT_FAILURE);
			case EMSGSIZE:
				printf("EMSGSIZE\n");
				exit(EXIT_FAILURE);
			case ENOBUFS:
				printf("ENOBUFS\n");
				exit(EXIT_FAILURE);
			case ENOMEM:
				printf("ENOMEM\n");
				exit(EXIT_FAILURE);
			case ENOTCONN:
				printf("ENOTCONN\n");
				exit(EXIT_FAILURE);
			case ENOTSOCK:
				printf("ENOTSOCK\n");
				exit(EXIT_FAILURE);
			case EOPNOTSUPP:
				printf("EOPNOTSUPP\n");
				exit(EXIT_FAILURE);
			case EPIPE:
				printf("EPIPE\n");
				exit(EXIT_FAILURE);
			default:
				printf("No idea what the error code is... :(\n");
				exit(EXIT_FAILURE);
		}
	}
}
