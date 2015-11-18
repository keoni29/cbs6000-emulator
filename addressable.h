#ifndef ADDRESSABLE_H_
#define ADDRESSABLE_H_
#include <stdint.h>
class addressable
{
public:
	virtual ~addressable();
	virtual uint8_t Read(uint16_t a) = 0;
	virtual void Write(uint16_t a, uint8_t d) = 0;
};

#endif
