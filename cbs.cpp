/* Filename: cbs.cpp
 * Authors: Koen van Vliet <8by8mail@gmail.com>
 * Date: 2015/10/26
 * Description: CBS6000 Microcomputer Emulator
 * Version: -
 * Known bugs:	-
 *
 * Todo: Create classes for ADC, CIA1, CIA2
 */

#include "acia.h"
#include <iostream>
#include "memory.h"
#include "mos6502.h"
#include <stdint.h>
#include <stdio.h>
#include "tcp.h"
#include "terminal.h"
#include "disasm.h"

#define versionInfo "CBS6000 emulator"

const int FPS = 60;						// Framerate
const int AVG_INSTRUCTION_CYCLES = 3; 	// (Cycles it takes for an instruction to execute)

// IO devices
#define IOSTART	0xD000
#define ACIA 	0x800
#define ACIA2 	0x820
#define ADC		0x840
#define CIA 	0x000
#define CIA2 	0x860
#define IOEND	0xDFFF

// Bankswitching pins
// Address $0000 - $03FF can be fixed to bank 1
// HIGH to disable this feature (default LOW)
#define RAML	3
// HIGH to disable rom (default LOW)
#define ROMDIS	4
// LOW: Bank 0 selected, HIGH: Bank 1 selected (default HIGH)
#define BANKSW	5

void consoleLog(const std::string &s);

#define TERMWIDTH 60
#define DBGWIDTH 40

// Terminal emulators and networking stuff
terminal term(TERMWIDTH,53,0,0);
terminal dbgterm(DBGWIDTH,43,TERMWIDTH*8,0);
terminal dasmterm(DBGWIDTH,9,TERMWIDTH*8,44 * 8);
tcp net(consoleLog);

// Addressable devices
acia acia1(&term);
acia acia2(&net);
memory ram(65536, 2, false);
memory rom(8192, 1, true);

#define WINDOW_WIDTH (TERMWIDTH + DBGWIDTH) * 8
#define WINDOW_HEIGHT 53 * 8

// Built in IO
#define DDR		0x0000
#define PORT 	0x0001
// 6510 internal IO Port
uint8_t Direction;
uint8_t PortOut;
// Pulldown resistors are on ROMDIS and RAML to make sure
// that the rom is enabled when the computer first boots.
// The rest of the pins either use internal pullups or external ones.
uint8_t PinDefault = ~((1<<ROMDIS) | (1<<RAML));

/*
void toHex(char* str, uint16_t value, uint8_t length)
{
	uint16_t prod = value;
	char* lut = "0123456789ABCDEF";
	for(uint8_t i = 0; i < length; i++)
	{
		str[length-i-1] = lut[prod % 16];
		prod /= 16;
	}
	str[length] = 0;
}*/

uint8_t MemoryRead(uint16_t address)
{
	if (address == DDR)
	{
		return Direction;
	}

	if (address == PORT)
	{
		return  (PortOut & Direction) | (~Direction & PinDefault);
	}

	if (address >= IOSTART && address <= IOEND)
	{
		switch (address & 0x08E0)
		{
		case ACIA:
			return acia1.Read(address & 1);
			break;
		case ACIA2:
			return acia2.Read(address & 1);
			break;
		default:
			return 0x00;
			break;
		}
	}

	uint8_t PortIn = (PortOut & Direction) | (~Direction & PinDefault);

	if (address >= 0xE000 && !( PortIn & (1<<ROMDIS) ) )
	{
		return rom.Read(address & 0x1FFF);
	}

	// Address $0000 - $03FF can be fixed to bank 1
	uint16_t bank = 0;
	if ( ( PortIn & (1<<BANKSW) ) || ( !( PortIn & (1<<RAML) ) && address < 0x0400 ) )
	{
		bank = 1;
	}

	ram.switchBank(bank);
	return ram.Read(address & 0xFFFF);
}

void MemoryWrite(uint16_t address, uint8_t data){
	if (address == DDR)
	{
		Direction = data;
		return;
	}

	if (address == PORT)
	{
		PortOut = data;
		return;
	}


	if (address >= IOSTART && address <= IOEND)
	{
		switch(address & 0x08E0)
		{
		case ACIA:
			acia1.Write(address & 1, data);
			break;
		case ACIA2:
			acia2.Write(address & 1, data);
			break;
		default:
			return;
			break;
		}
	}

	uint8_t PortIn = (PortOut & Direction) | (~Direction & PinDefault);

	if (address >= 0xE000 && !( PortIn & (1<<ROMDIS) ) )
	{
		rom.Write(address & 0x1FFF, data);	// ** Rom is usually set to read-only
		return;
	}

	// Address $0000 - $03FF can be fixed to bank 1
	uint16_t bank = 0;
	if ( ( PortIn & (1<<BANKSW) ) || ( !( PortIn & (1<<RAML) ) && address < 0x0400 ) )
	{
		bank = 1;
	}

	ram.switchBank(bank);
	ram.Write(address & 0xFFFF, data);
}

void consoleLog(const std::string &s)
{
	dbgterm.printString(s + "\r\n");
}

int main(int argc, char* argv[])
{
	mos6502 cpu(MemoryRead, MemoryWrite);
	disasm dasm;
	int SPEED = 100; // %
	int CPUFREQ = 1000000; // Hz - CPU clock frequency
	bool HALT = false;
	char str[50];

	if (argc < 2)
	{
		std::cout << "Filename required!" << std::endl;
		return 0;
	}

	if (rom.loadFromFile(argv[1]) == 0)
	{
		std::cout << "Rom invalid!" << std::endl;
	}

	std::string caption("CBS6000 Emulator - (C)2015 Koen van Vliet");
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), caption);
	window.setFramerateLimit(FPS);

	if (!term.load("terminal8x8_gs_ro.png",8,8))
	{
		std::cout << "Failed to open bitmap font file!" << std::endl;
		return 0;
	}

	if (!dbgterm.load("terminal8x8_gs_ro.png",8,8))
	{
		std::cout << "Failed to open bitmap font file!" << std::endl;
		return 0;
	}

	if (!dasmterm.load("terminal8x8_gs_ro.png",8,8))
	{
		std::cout << "Failed to open bitmap font file!" << std::endl;
		return 0;
	}

	int ctile = 177;
	term.setCursorTile(ctile);
	dbgterm.setCursorTile(ctile);
	dbgterm.enableCursor(false);
	dbgterm.setTextColor(sf::Color::Green);
	dasmterm.enableCursor(false);
	dasmterm.setTextColor(sf::Color::Cyan);

	consoleLog("---[Debug console]---");
	consoleLog(versionInfo);
	net.init();
	consoleLog("Starting CPU emulation");

	// Reset Cpu and internal IO port for the 6510
	Direction = 0x00;
	PortOut = 0x00;
	cpu.Reset();

	bool SHOWDASM = false;

	while(window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type) {
				case sf::Event::Closed :
					window.close();
					break;
				case sf::Event::TextEntered:
					if (event.text.unicode < 128)
					{
						char c = (char)event.text.unicode;
						term.feedChar(c);
					}
					break;
				case sf::Event::KeyPressed :
					// Increase and decrease the CPU speed
					if (event.key.code == sf::Keyboard::PageUp)
					{
						if (SPEED < 500)
						{
							SPEED += 5;
						}

						sprintf(str, "%d%% speed", SPEED);
						consoleLog(str);
					}
					if (event.key.code == sf::Keyboard::PageDown)
					{
						if (SPEED)
						{
							SPEED -= 5;
							if (SPEED < 0)
							{
								SPEED = 0;
							}
							sprintf(str, "%d%% speed", SPEED);
							consoleLog(str);
						}
					}

					// Stop execution
					if (event.key.code == sf::Keyboard::Delete)
					{
						HALT = true;
						SHOWDASM = true;
						consoleLog("Stopped execution");
					}

					// Stop/Resume execution
					if (event.key.code == sf::Keyboard::F8)
					{
						if (HALT)
						{
							HALT = false;
							consoleLog("Resumed execution");
						}
						else
						{
							HALT = true;
							SHOWDASM = true;
							consoleLog("Stopped execution");
						}
					}
					// Execute single instruction
					if (event.key.code == sf::Keyboard::F5)
					{
						if (HALT)
						{
							cpu.Run(1);
							SHOWDASM = true;
							consoleLog("Executed one instruction");
						}
					}

					if (event.key.code == sf::Keyboard::F1)
					{
						uint16_t state[6];
						cpu.GetState(state);

						sprintf(str,"A:%3d,X:%3d,Y:%3d\r\nPC:%5d,SP:%5d,SR:%3d",state[0], state[1], state[2], state[3], state[4], state[5]);
						consoleLog(str);
					}

					if (event.key.code == sf::Keyboard::End)
					{
						// Reset Cpu and internal IO port for the 6510
						Direction = 0x00;
						PortOut = 0x00;
						cpu.Reset();
						consoleLog("--[Reset CPU]--");
					}

					break;
				default:
					break;
			}
		}

		// Run a bunch of instructions
		if(!HALT)
		{
			int IPF = (CPUFREQ * SPEED) /(FPS * AVG_INSTRUCTION_CYCLES * 100); 			// Instructions per frame
			cpu.Run(IPF);
		}

		if (SHOWDASM)
		{
			SHOWDASM = false;
			// Todo fix pc
			uint16_t state[6];
			uint8_t buff[3];
			uint16_t pc;
			cpu.GetState(state);
			pc = state[3];
			for(int i = 0; i< 9; i++)
			{
				uint pc_prev = pc;
				dasmterm.printString("\r\n");
				buff[0] = MemoryRead(pc);
				buff[1] = MemoryRead(pc+1);
				buff[2] = MemoryRead(pc+2);
				pc += 1 + dasm.getDisassembly((char*)str, buff, 1, pc_prev);
				if(i ==0)
					dasmterm.printString(">");
				else
					dasmterm.printString(" ");
				dasmterm.printString(str);
			}

		}

		// Update the screen
		window.clear();
		net.update();
		term.update();
		dbgterm.update();
		dasmterm.update();
		window.draw(term);
		window.draw(dbgterm);
		window.draw(dasmterm);
		window.display();
	}
	return 0;
}

