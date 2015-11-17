/*
 * tcp.h
 *
 *  Created on: Nov 17, 2015
 *      Author: koen
 */

#ifndef TCP_H_
#define TCP_H_

#include <SFML/Network.hpp>

class tcp
{
private:
	sf::TcpListener listener;
	sf::TcpSocket client;
	bool m_connected;
	// Console log callback function
	typedef void (*ConsoleLog)(std::string s);
	ConsoleLog Log;
public:
	tcp(ConsoleLog l);
	void init();
	void update();
};

#endif /* TCP_H_ */
