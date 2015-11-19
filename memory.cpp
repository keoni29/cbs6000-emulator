/*
 * memory.cpp
 *
 *  Created on: Nov 19, 2015
 *      Author: koen
 */

#include "memory.h"
#include <stdio.h>

memory::memory(uint32_t size, uint16_t banks, bool readOnly)
{
	m_size = size;
	m_banks = banks;
	m_activeBank = 0;
	m_readOnly = readOnly;
	m_memory = new uint8_t[m_size * banks];
}

// Read from memory
uint8_t memory::Read(uint16_t address)
{
	uint16_t bank = m_activeBank % m_banks;
	if (address < m_size)
		return m_memory[bank * m_size + address];
	return 0;
}

// Write to memory
void memory::Write(uint16_t address, uint8_t data)
{
	if (address < m_size && !m_readOnly)
		m_memory[m_activeBank * m_size + address] = data;
}

// Switch active bank
bool memory::switchBank(uint16_t bank)
{
	if (bank >= m_banks)
		return false;	// Bank does not exist

	m_activeBank = bank;
	return true;
}

// Load binary from file
uint32_t memory::loadFromFile(const char* filename)
{
	uint32_t len = 0;
	if (FILE *fp = fopen(filename, "rb"))
	{
		len = fread(m_memory, 1, m_size * m_banks, fp);
		fclose(fp);
	}
	return len;
}
