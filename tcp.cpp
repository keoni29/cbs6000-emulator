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
	if (m_connected == true)
	{
		char data[101];
		std::size_t received;
		sf::Socket::Status stat = client.receive(data, 100, received);
		data[received] = 0;
		if (stat == sf::Socket::Disconnected)
		{
			Log("The client has disconnected.");
			m_connected = false;
		}
		else
		{
			Log(data);
		}
	}
}



