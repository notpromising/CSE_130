#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/stat.h>

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#define BUF_LEN 4096

extern int errno;

/**
   Converts a string to an 16 bits unsigned integer.
   Returns 0 if the string is malformed or out of the range.
 */
uint16_t strtouint16(char number[]) {
  char *last;
  long num = strtol(number, &last, 10);
  if (num <= 0 || num > UINT16_MAX || *last != '\0') {
    return 0;
  }
  return num;
}

/**
   Creates a socket for listening for connections.
   Closes the program and prints an error message on error.
 */
int create_listen_socket(uint16_t port) {
  struct sockaddr_in addr;
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    err(EXIT_FAILURE, "socket error");
  }

  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htons(INADDR_ANY);
  addr.sin_port = htons(port);
  if (bind(listenfd, (struct sockaddr*)&addr, sizeof addr) < 0) {
    err(EXIT_FAILURE, "bind error");
  }

  if (listen(listenfd, 500) < 0) {
    err(EXIT_FAILURE, "listen error");
  }

  return listenfd;
}

/** Returns an int depending on the request type, -1 for unknown,
 *  0 for GET, 1 for PUT, and 2 for HEAD.
 */
int determine_request_type(char *request) {
	if (strcmp(request, "GET") == 0) {
		return 0;
	} else if (strcmp(request, "PUT") == 0) {
		return 1;
	} else if (strcmp(request, "HEAD") == 0) {
		return 2;
	} else {
		return -1;
	}
}

/** Returns a 1 or a 0 if the file name is valid or invalid
 *  References:
 *  https://serverfault.com/questions/9546/filename-length-limits-on-linux
 *  I re-read the spec and didn't even end up using this ^
 */
int validate_filename(char *filename) {
	if(strlen(filename) == 0 || strlen(filename) > 19) {
		return 0;
	} else {
		for (int i=0, max=strlen(filename); i<max; i++) {
			if (!isalnum(filename[i]) && !(filename[i] == '_') && !(filename[i] == '.')) {
				return 0;
			}
		}
		return 1;
	}
	return 1;
	//TODO page 3 of spec has more requriements
}

/** Returns a 1 or a 0 if the host value is valid or invalid
 * References:
 * https://stackoverflow.com/questions/7984955/what-is-wrong-with-my-for-loops-i-get-warnings-comparison-between-signed-and-u
 */
int valid_host(char *host) {
	for (size_t i=0, end = strlen(host); i < end; i++) {
		if (host[i] == ' ') {
			return 0;
		}
	}
	return 1;
}

/** Returns the length of a file if possible or -1 if errors occured
 * References:
 * https://codeforwin.org/2018/03/c-program-find-file-properties-using-stat-function.html
 * stat man page
 */
int file_length(char *filename) {
	struct stat filestats;

	int result = stat(filename, &filestats);
	if (result != 0) {
		return -1;
	}
	int returnval = filestats.st_size;
	return returnval;
}

/** Returns 1 if the string represents a number and 0 otherwise
 */
int is_a_number(char *stringnumber) {
	for (int i=0, max=strlen(stringnumber); i<max; i++) {
		if(!isdigit(stringnumber[i])) {
			printf("index where it wasn't a number %d, \\%02hhx\n", i, stringnumber[i]);
			return 0;
		}
	}
	return 1;
}

/** Sends HTTP status codes other than 200 depending on the code over the connection referenced to by fd
 */
int http_code(int fd, int code) {
	int send_result = -1;
	switch(code){
		case 201:
			send_result = send(fd, "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n", 51, 0);
			break;
		case 400:
			send_result = send(fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n", 60, 0);
			break;
		case 403:
			send_result = send(fd, "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n", 56, 0);
			break;
		case 404:
			send_result = send(fd, "HTTP/1.1 404 File Not Found\r\nContent-Length: 15\r\n\r\nFile Not Found\n", 66, 0);
			break;
		case 500:
			send_result = send(fd, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 22\r\n\r\nInternal Server Error\n", 80, 0);
			break;
		case 501:
			send_result = send(fd, "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n", 68, 0);
			break;
	}
	return send_result;
}

void nullify_buffer(char* buffer, int length) {
	for (int i=0; i<length; i++) {
		buffer[i] = 0x00;
	}
}

void handle_connection(int connfd) {
  // do something
  printf("connection made\n");
  // when done, close socket
  //
  // from section

  char buf[BUF_LEN];
  char *saveline;
  char *savetoken;
  char *filename;
  int request_complete = 0;
  int request_type = -1;
  int content_length = 0;
  
  while(!request_complete) {
	nullify_buffer(buf, BUF_LEN);
	int r = recv(connfd, buf, BUF_LEN, 0);
	//handle errors
	if (r < 0) {
		perror("Error reading from socket");
		printf("r: %d\n", r);
		close(connfd);
		return;
	}
	//printf("buf: %s\n", buf);
	char *headerline = strtok_r(buf, "\r\n", &saveline);
	printf("headerline: %s\n", headerline);
	request_type = determine_request_type(strtok_r(headerline, " ", &savetoken));
	if (request_type < 0) {
		http_code(connfd, 501);
		close(connfd);
		return;
	}

	filename = strtok_r(NULL, " ", &savetoken);
	if (filename[0] != '/') {
		http_code(connfd, 400);
		close(connfd);
		return;
	} else {
		filename++;
	}

	if (!validate_filename(filename)) {
		http_code(connfd, 400);
		close(connfd);
		return;
	}

	char *httpversion = strtok_r(NULL, " ", &savetoken);

	printf("httpversion: %s\n", httpversion);

	if (!(strcmp(httpversion, "HTTP/1.1") == 0)) {
		http_code(connfd, 400);
		printf("ERROR INCORECT HTTP VERSION\n");
		close(connfd);
		return;
	}	

	printf("filename %s\n", filename);


	printf("reqest type: %d\n", request_type);

	char *header_field_name;
	char *header_field_value;
	int finished_reading_headers = 0;

	while (finished_reading_headers == 0) {
		if (headerline == NULL) {
			r = recv(connfd, buf, BUF_LEN, 0);
			printf("r: %d\n", r);
			headerline = strtok_r(buf, "\n", &saveline);
			if (r<0) {
				perror("Error while reading from socket");
				close(connfd);
			}
		} else {
			headerline = strtok_r(NULL, "\n", &saveline);
		}
		//printf("headerline: %s\n", headerline);
		if (headerline != NULL && strcmp(headerline, "\r") == 0) {
			printf("NO MORE HEADERS\n");
			headerline = NULL;
			finished_reading_headers = 1;
		}
		if (headerline != NULL && headerline[strlen(headerline)-1] == '\r') {
			header_field_name = strtok_r(headerline, " ", &savetoken);
			header_field_value = header_field_name + strlen(header_field_name) + 1;
			if (strcasecmp(header_field_name, "Host:") == 0) {
				if (!valid_host(header_field_value)) {
					http_code(connfd, 400);
					close(connfd);
					printf("invalid hostname\n");
					return;
				}
			}
			if (strcasecmp(header_field_name, "Content-Length:") == 0 && request_type == 1) {
				printf("content length header found\n");
				char *content_length_value = strtok_r(header_field_value, "\r", &savetoken);

				if (!is_a_number(content_length_value)) {
					http_code(connfd, 400);
					close(connfd);
					printf("invalid content length\n");
					return;
				} else {
					content_length = atoi(content_length_value);
				}
			}
			printf("name: %s, value: %s\n", header_field_name, header_field_value);
		}
	}

	if (request_type == 0) {
		int fd = open(filename, O_RDONLY);
		if (fd == -1) {
			if (errno == EACCES) {
				printf("you do not have access to that file\n");
				http_code(connfd, 403);
			} else if (errno == ENOENT) {
				http_code(connfd, 404);
			} else {
				http_code(connfd, 500);
			}
			printf("%d, %d\n", errno, ENOENT);
			perror("error");
			close(connfd);
			return;
		} else {
			send(connfd, "HTTP/1.1 200 OK\r\nContent-Length: ", 33, 0);
			char content_len_str[4096] = {0};
			content_length = file_length(filename);
			//printf("Content length: %d\n", content_length);
			sprintf(content_len_str, "%d\r\n\r\n", content_length);
			send(connfd, content_len_str, strlen(content_len_str), 0);

			char content[BUF_LEN*5] = {0};
			while (content_length > 0) {
				printf("Content length: %d\n", content_length);
				int readresult = read(fd, content, BUF_LEN*5);
				int sendresult = send(connfd, content, readresult, 0);
				content_length -= readresult;
			}



		}
	} else if (request_type == 1) {
		int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
		if (fd == -1) {
			printf("cannot create file\n");
			if (errno == EACCES) {
				printf("you do not have access to modify that file\n");
				http_code(connfd, 403);
			} else {
				http_code(connfd, 500);
			}
			close(connfd);
			return;
		} else {
			char *start_of_file;
			start_of_file = strtok_r(NULL, "", &saveline);
			if (start_of_file != NULL) {
				int end_of_headers = start_of_file - buf;
				printf("start of file: %s", start_of_file);
				printf("end of headers: %d\n", end_of_headers);
				printf("%s\n", &buf[end_of_headers]);
				if (content_length <= BUF_LEN - end_of_headers) {
					write(fd, &buf[end_of_headers], content_length);
				} else {
					write(fd, &buf[end_of_headers], BUF_LEN - end_of_headers);
					content_length -= BUF_LEN - end_of_headers;
				}
			}
			while (content_length > 0) {
				int recieved = recv(connfd, buf, BUF_LEN, 0);
				printf("recieved: %d\n", recieved);
				if (recieved < 0) {
					http_code(connfd, 400);
					close(fd);
					close(connfd);
					return;
				} else if (recieved == 0) {
					printf("no incoming data\n");
					close(fd);
					close(connfd);
					return;
				}else {
					content_length -= recieved;
					write(fd, &buf, recieved);
				}
			}
			http_code(connfd, 201);
			close(fd);
		}

	} else if (request_type == 2) {
		int fd = open(filename, O_RDONLY);
                if (fd == -1) {
                        if (errno == EACCES) {
				http_code(connfd, 403);
			} else if (errno == ENOENT) {
				http_code(connfd, 404);
			}
                } else {
			send(connfd, "HTTP/1.1 200 OK\r\nContent-Length: ", 33, 0);
			char content_len_str[4096] = {0};
			content_length = file_length(filename);
			sprintf(content_len_str, "%d\r\n\r\n", content_length);
			send(connfd, content_len_str, strlen(content_len_str), 0);
		}
	} else {
		http_code(connfd, 501);
	}

	request_complete = 1;
  }
  close(connfd);
}

int main(int argc, char *argv[]) {
  int listenfd;
  uint16_t port;

  if (argc != 2) {
    errx(EXIT_FAILURE, "wrong arguments: %s port_num", argv[0]);
  }
  port = strtouint16(argv[1]);
  if (port == 0) {
    errx(EXIT_FAILURE, "invalid port number: %s", argv[1]);
  }
  listenfd = create_listen_socket(port);

  while(1) {
    int connfd = accept(listenfd, NULL, NULL);
    if (connfd < 0) {
      warn("accept error");
      continue;
    }
    handle_connection(connfd);
  }
  return EXIT_SUCCESS;
}