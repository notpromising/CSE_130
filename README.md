# CSE_130
HTTP Server

README:
I documented by thought process:


first read
if result of read is 0 then close the fd
if read bytes is > 0, then copy that into request buffer.
we compare, strstr(read_bytes, "\r\n\r\n");
if its not there, then repeat the reading and copying
oncce we have rnrn, then we parse the request line, -> strtok, split them 3 and strcmp between them
if GET||HEAD
-> check if file exitts, or check permission for opening the file
-> else send error messg of 404file exist or 403 for permission
we make the format headers:
get the content length =file_len
and send headers using send() to fd of socket
if (GET) then send the file
for put request()
check for permission of file,
check if we have entire file
if yes then write file
if no then write and read to file until content-length.


*/
Source Cite:


Reference sheet on HTTP protocol: https://www.w3.org/Protocols/rfc2616/rfc2616.html


https://gist.github.com/laobubu/d6d0e9beb934b60b2e552c2d03e1409e
https://stackoverflow.com/questions/20019786/safe-and-portable-way-to-convert-a-char-to-uint16-t
https://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html#sec4
https://pubs.opengroup.org/onlinepubs/007904975/functions/recv.html
https://stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c
https://www.tutorialspoint.com/http/http_requests.htm
https://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response
https://www.geeksforgeeks.org/memset-c-example/
https://www.tutorialspoint.com/c_standard_library/c_function_strstr.htm
https://www.geeksforgeeks.org/c-program-to-read-a-range-of-bytes-from-file-and-print-it-to-console/
https://stackoverflow.com/questions/18109458/read-from-a-text-file-and-parse-lines-into-words-in-c
https://stackoverflow.com/questions/41286260/parse-http-request-line-in-c
https://www.tutorialspoint.com/c_standard_library/c_function_strstr.htm
https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm



/* To sum up http server:
-> We open a port (lets say 4 digit port) for listening
-> When a contact is made, we get some information like GET, PUT or HEAD request
-> We translate that request into a file request
-> Then we open the file and split it back to the client
*/
/*
1.The idea is to build an http server. Open two terminals. One runs the "./httpserver 1234" command.
2.argc-> tells us the total number of argument counter
3.argv-> represents the argument that were entered on the command line. Like argv[0] tells us its calling the binary file name
4. Create--> listen--> accept connections on a socket
5. Socket is our fd --> integer that represents
GET:
send file content requested by the client via the port that we connected
PUT:
write from a file:
Send a response code to client with a message

main(argc, argv) logic:
we open/listen to a socket, connect to same port which is file descriptor
-> check for correct argc count and and that all arg are correct.
-> while(1) keep waiting and receiving connection until we force to close that

handleConnection() logic:
-> Given the port number as our input or in the form of file descriptor

We want to loop till we recieve bytes from client request through recv()

We receive the request-line and keep it in a buffer
Then we parse each word by space and store the request, file text, version, content and content-length in a seprate buff
If request is PUT():
-> do the error check-> if correct request, if we can open file for reading
-> we can open the body file to read
-> read the content and store it in buffer.
-> create a response header with content of the file and
-> send it to client while writing to their buffer
If request is GET():
-> do error check-> if request is correct and if we can open file for reading
-> upon that we can open the body file for reading
-> we can read the content of file and store that in a separate buffer.
-> then we create a response header and send the necessary message(body) to the client
If request is HEAD():
-> only send the header and not body of the response.
