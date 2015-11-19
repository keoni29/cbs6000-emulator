#ifndef ADDRESSABLE_H_
#define ADDRESSABLE_H_
#include <stdint.h>
class addressable
{
public:
	virtual ~addressable();
	virtual uint8_t Read(uint16_t address) = 0;
	virtual void Write(uint16_t address, uint8_t data) = 0;
};

#endif
