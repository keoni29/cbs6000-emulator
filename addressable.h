#ifndef TERMINAL_H_
#define TERMINAL_H_
#include <stdint.h>
class addressable
{
public:
	addressable();
	virtual ~addressable();

	virtual uint8_t Read(uint16_t a);
	virtual void Write(uint16_t a, uint8_t d);
};

#endif
