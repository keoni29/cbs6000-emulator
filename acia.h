/*
 * acia.h
 *
 *  Created on: Nov 17, 2015
 *      Author: koen
 */

#ifndef ACIA_H_
#define ACIA_H_

#include "addressable.h"
#include "comDevice.h"
#include <string>

class acia : public addressable
{
private:
	bool m_isReset, m_isInitialized;
	comDevice *m_com;
	uint8_t m_control;
	uint8_t m_status;
	uint8_t m_rxData;
	uint8_t m_txData;

	enum statusBits{
		RDRF,
		TDRE,
		DCD,
		CTS,
		FE,
		OVRN,
		PE,
		IRQ
	};

public:
	acia(comDevice* m_term);
	~acia();
	uint8_t Read(uint16_t address);
	void Write(uint16_t address, uint8_t data);
};

#endif /* ACIA_H_ */
