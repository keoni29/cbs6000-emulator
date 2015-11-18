/*
 * tcp.cpp
 *
 *  Created on: Nov 17, 2015
 *      Author: koen
 */

#include "tcp.h"
#include <stdio.h>

tcp::tcp(ConsoleLog l) : m_connected(false)
{
	Log = (ConsoleLog)l;
}

void tcp::init(){
	listener.setBlocking(false);
	// bind the listener to a port
	int port = 10000;
	bool listenerOK = false;
	while (!listenerOK && port < 10010)
	{
		char buff[50];
		sprintf(buff, "Trying port %d ...", port);
		Log(buff);
		if (listener.listen(port) == sf::Socket::Done)
		{
			listenerOK = true;
			sprintf(buff, "Listening on port %d!", port);
			Log(buff);
		}
		port++;
	}

	if (!listenerOK)
	{
		Log("Could not create listener!");
	}
}

void tcp::update()
{
	if (listener.accept(client) == sf::Socket::Done)
	{
		Log("A client has connected! ");
		m_connected= true;
	}
}

int16_t tcp::Read(uint8_t *buff, int16_t nbytes)
{
	if (m_connected == true)
	{
		std::size_t received;
		sf::Socket::Status stat = client.receive(buff, nbytes, received);
		return received;
	}
	return 0;
}

int16_t tcp::Read(uint8_t & d)
{
	uint8_t buff[1];
	if (m_connected == true)
	{
		std::size_t received;
		sf::Socket::Status stat = client.receive(buff, 1, received);
		if (received)
		{
			d = buff[0];
			return received;
		}
	}
	return 0;
}
int16_t tcp::Write(uint8_t *buff, int16_t nbytes)
{
	// Write to socket
	client.send(buff, nbytes);
	return -1;
}
int16_t tcp::Write(uint8_t d)
{
	uint8_t buff[1];
	buff[0] = d;
	client.send(buff, 1);
	return -1;
}


