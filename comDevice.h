/*
 * comDevice.h
 *
 *  Created on: Nov 18, 2015
 *      Author: koen
 */

#ifndef COMDEVICE_H_
#define COMDEVICE_H_

#include <stdint.h>

class comDevice
{
public:
	virtual ~comDevice();
	virtual int16_t Read(uint8_t *buff, int16_t nbytes) = 0;
	virtual int16_t Read(uint8_t &d) = 0;
	virtual int16_t Write(uint8_t *buff, int16_t nbytes) = 0;
	virtual int16_t Write(uint8_t d) = 0;
	//virtual bool GetStatus(uint16_t field, uint16_t value) const = 0;
	//virtual bool SetConfig(uint16_t field, uint16_t value) = 0;
};

#endif /* COMDEVICE_H_ */
