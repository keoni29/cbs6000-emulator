/*
 * acia.cpp
 *
 *  Created on: Nov 17, 2015
 *      Author: koen
 */

#include "acia.h"

uint8_t acia::Read(uint16_t a)
{
	if (a & 1)
	{
		return m_rxData;
	}
	else
	{
		return m_status;
	}
}

void acia::Write(uint16_t a, uint8_t d)
{
	if (a & 1)
	{
		// Callback for sending a byte
	}
	else
	{
		m_control = d;
	}
}
