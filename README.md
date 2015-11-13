# Network
## Created in Feb. 2015
#### A simple implementation of a web sever that understands HTTP protocol
##### Compile on comp112-01 server at Tufts CS
	gcc -g a1.c -lnsl
##### Description
* Takes a single argument: a port number on which to listen for requests.
* Accepts TCP requests from web browsers and responds with a single
HTML page of content based on the URL:
	* The main page (/): Descriptions of the last 10 browsers that
accessed it. For the browser details, you should display the
client IP address, client hostname, and client user agent.
Client user agent note should only list the user agent – no
cookies and other headers.
	* An about page (about.html): Anything you’d like to say about
yourself. It doesn’t have to be long.
	* Return "HTTP/1.1 404 Not Found" as HTTP response if URL other
than "/" or "about.html" is requested.
* Protects users from script injection attacks
