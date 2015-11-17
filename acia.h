/*
 * acia.h
 *
 *  Created on: Nov 17, 2015
 *      Author: koen
 */

#ifndef ACIA_H_
#define ACIA_H_

#include "addressable.h"

class acia : public addressable
{
public:
	~acia();
	uint8_t Read(uint16_t a);
	void Write(uint16_t a, uint8_t d);
private:
	uint8_t m_control;
	uint8_t m_status;
	uint8_t m_rxData;
	uint8_t m_txData;
};

#endif /* ACIA_H_ */
