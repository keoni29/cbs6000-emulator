/*
 * memory.h
 *
 *  Created on: Nov 19, 2015
 *      Author: koen
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include "addressable.h"

class memory : public addressable
{
public:
	memory(uint32_t size, uint16_t banks, bool readOnly);

	uint8_t Read(uint16_t address);
	void Write(uint16_t address, uint8_t data);

	bool switchBank(uint16_t bank);
	uint32_t loadFromFile(const char* filename);
private:
	uint32_t m_size;
	uint16_t m_banks;
	uint16_t m_activeBank;
	uint8_t *m_memory;
	bool m_readOnly;
};


#endif /* MEMORY_H_ */
