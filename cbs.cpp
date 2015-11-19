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

#define versionInfo "CBS6000 emulator"

const int FPS = 60;						// Framerate
const int AVG_INSTRUCTION_CYCLES = 3; 	// (Cycles it takes for an instruction to execute)

// Built in IO
#define DDR		0x0000
#define PORT 	0x0001

// IO devices
#define IOSTART	0xD000
#define ACIA 	0x800
#define ACIA2 	0x820
#define ADC		0x840
#define CIA 	0x000
#define CIA2 	0x860
#define IOEND	0xDFFF

void consoleLog(const std::string &s);

// Terminal emulators and networking stuff
terminal term(55,53,0,0);
terminal dbgterm(25,53,55*8,0);
tcp net(consoleLog);

// Addressable devices
acia acia1(&term);
acia acia2(&net);
memory ram(65536, 2, false);
memory rom(8192, 1, true);

#define WINDOW_WIDTH 80 * 8
#define WINDOW_HEIGHT 53 * 8

uint8_t MemoryRead(uint16_t address)
{
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
	if (address >= 0xE000)
	{
		return rom.Read(address & 0x1FFF);
	}

	return ram.Read(address & 0xFFFF);
}

void MemoryWrite(uint16_t address, uint8_t data){
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

	if (address >= 0xE000)
	{
		rom.Write(address & 0x1FFF, data);	// ** Rom is usually set to read-only
		return;
	}
	ram.Write(address & 0xFFFF, data);
}

void consoleLog(const std::string &s)
{
	dbgterm.printString(s + "\r\n");
}

int main(int argc, char* argv[])
{
	mos6502 cpu(MemoryRead, MemoryWrite);
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

	int ctile = 177;
	term.setCursorTile(ctile);
	dbgterm.setCursorTile(ctile);
	dbgterm.enableCursor(true);
	dbgterm.setTextColor(sf::Color::Green);

	consoleLog("---[Debug console]---");
	consoleLog(versionInfo);
	net.init();
	consoleLog("Starting CPU emulation");
	cpu.Reset();

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
							consoleLog("Stopped execution");
						}
					}
					// Execute single instruction
					if (event.key.code == sf::Keyboard::F5)
					{
						if (HALT)
						{
							cpu.Run(1);
							consoleLog("Executed one instruction");
						}
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

		// Update the screen
		window.clear();
		net.update();
		term.update();
		dbgterm.update();
		window.draw(term);
		window.draw(dbgterm);
		window.display();
	}
	return 0;
}

