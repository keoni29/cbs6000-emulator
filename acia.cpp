/*
 * acia.cpp
 *
 *  Created on: Nov 17, 2015
 *      Author: koen
 */

#include "acia.h"

acia::acia(comDevice * com)
{
	m_com = com;
	m_control = 0;
	m_status = 1<<TDRE;
	m_rxData = 0;
	m_txData = 0;
	m_isReset = false;
	m_isInitialized = false;
}

acia::~acia()
{}

uint8_t acia::Read(uint16_t a)
{
	// Update register contents.
	if (!(m_status & (1<<RDRF))) // Is the receive data register empty?
	{
		uint8_t c;
		int n;
		n = m_com->Read(c);
		if (n == 1)			// Yes! Try to receive a character.
		{
			m_rxData = c;
			// Byte received!
			m_status |= 1<<RDRF;
		}
		else if (n == 0)
		{
			// No byte received
			m_status &= ~(1<<RDRF);
		}
		else
		{
			// Something went wrong
		}
	}
	if (a & 1)
	{
		// Clear data register full flag.
		m_status &= ~(1<<RDRF);
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
		m_com->Write(d);
	}
	else
	{
		m_control = d;
		if((m_control & 0x3) == 0x3)
		{
			if(!m_isReset)
			{
				m_isReset = true;
			}
		}
		else
		{
			if(m_isReset)
			{
				m_isInitialized = true;
			}
			else
			{
				//Warning: CPU tries to set up ACIA before resetting it!
			}
		}
	}
}
