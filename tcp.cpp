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
	client.setBlocking(false);
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
	if (m_connected == false)
	{
		if (listener.accept(client) == sf::Socket::Done)
		{
			Log("A client has connected!");
			m_connected = true;
		}
	}
}

// Read from socket
int16_t tcp::Read(uint8_t *buff, int16_t nbytes)
{
	std::size_t received;
	if (m_connected)
	{
		sf::Socket::Status stat;
		stat = client.receive(buff, nbytes, received);
		if (stat == sf::TcpSocket::Disconnected)
		{
			// Client disconnected.
			Log("Connection closed by client!");
			m_connected = false;
		}
	}
	return received;
}

// Read from socket
int16_t tcp::Read(uint8_t & d)
{
	std::size_t received;
	uint8_t buff[1];
	received = Read(buff, 1);
	d = buff[0];
	return received;
}

// Write to socket
int16_t tcp::Write(uint8_t *buff, int16_t nbytes)
{
	if (m_connected)
	{
		sf::Socket::Status stat;
		stat = client.send(buff, nbytes);
		if (stat == sf::TcpSocket::Disconnected)
		{
			Log("Connection closed by client!");
			m_connected = false;
			return 0;
		}
	}
	return nbytes;
}

// Write to socket
int16_t tcp::Write(uint8_t d)
{
	uint8_t buff[1];
	buff[0] = d;
	return Write(buff,1);
}


