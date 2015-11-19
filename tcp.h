/*
 * tcp.h
 *
 *  Created on: Nov 17, 2015
 *      Author: koen
 */

#ifndef TCP_H_
#define TCP_H_

#include <SFML/Network.hpp>
#include "comDevice.h"

class tcp : public comDevice
{
private:
	sf::TcpListener listener;
	sf::TcpSocket client;
	bool m_connected;
	// Console log callback function
	typedef void (*ConsoleLog)(const std::string &s);
	ConsoleLog Log;
public:
	tcp(ConsoleLog l);
	void init();
	void update();

	int16_t Read(uint8_t *buff, int16_t nbytes);
	int16_t Read(uint8_t & d);
	int16_t Write(uint8_t *buff, int16_t nbytes);
	int16_t Write(uint8_t d);
};

#endif /* TCP_H_ */
