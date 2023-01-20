#include <stdint.h>

// Parses port number and binds and listens on it.
// returns a fd as a positive integer if successful
// returns -1 if passed an invalid port number
// returns -2 if opening the socket failed
// returns -3 if binding the socket failed
// returns -4 if listening failed
int create_listen_socket(uint16_t port);
