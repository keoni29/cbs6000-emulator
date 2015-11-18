/* Filename: cbs.cpp
 * Authors: Koen van Vliet <8by8mail@gmail.com>
 * Date: 2015/10/26
 * Description: CBS6000 Microcomputer Emulator
 * Version: -
 * Known bugs:	- Colors don't scroll then the text does.
 *
 * Todo: Make base class for addressable devices.
 * Todo: Create classes for ACIA1, ACIA2, ADC, CIA1, CIA2, ROM, RAM
 * Todo: Change comDevice to comInterface or comIface?
 */

#include "mos6502.h"
#include <iostream>
#include "terminal.h"
#include "tcp.h"
#include "acia.h"
#include <stdio.h>
#include <unistd.h>

#define versionInfo "CBS6000 emulator\r\nby Koen van Vliet\r\nFirst version\r\n"

// Calculate amt of instructions per frame.
// The emulator is not cycle-accurate, but this approximates
// the execution time of the real hardware.
const int FPS = 60;				// Frames per second
const int CPUSPEED = 1000000; 	// CPU clock frequency
const int AIC = 3; 				// Average instruction clockcycles
const int IPS = CPUSPEED / AIC;	// Instructions per second
const int IPF = IPS/FPS; 		// Instructions per frame

// Memory devices
#define RAMSTART	0x0000
#define RAMEND		0xCFFF
#define ROMSTART 	0xE000
#define ROMEND		0xFFFF

// Built in IO
#define DDR		0x0000
#define PORT 	0x0001

// IO devices
#define IOSTART	0xD000
#define ACIA 	0xD800
#define ACIA2 	0xD840
#define CIA 	0xD000
#define CIA2 	0xD860
#define IOEND	0xDFFF

// IO register addresses
#define ACIA_CTRL	0
#define ACIA_SR		0
#define ACIA_DAT	1


uint8_t ram[RAMEND - RAMSTART + 1];
uint8_t rom[ROMEND - ROMSTART + 1];

void consoleLog(std::string s);

terminal term(55,53,0,0);
terminal dbgterm(25,53,55*8,0);
tcp net(consoleLog);

acia acia1(&term);

#define WINDOW_WIDTH 80 * 8
#define WINDOW_HEIGHT 53 * 8

size_t loadRom(const char* filename)
{
	size_t len = 0;
	if (FILE *fp = fopen(filename, "rb"))
	{
		len = fread(rom, 1, sizeof(rom), fp);
		fclose(fp);
	}
	return len;
}

uint8_t MemoryRead(uint16_t address)
{
	if (address >= IOSTART && address <= IOEND)
	{
		return acia1.Read(address);
	}
	if (address >= ROMSTART && address <= ROMEND)
		return rom[address - ROMSTART];
	if (address >= RAMSTART && address <= RAMEND)
		return ram[address - RAMSTART];
	return 0xFF;
}

void MemoryWrite(uint16_t address, uint8_t value){
	if (address >= IOSTART && address <= IOEND)
	{
		acia1.Write(address, value);
	}
	if (address >= RAMSTART && address <= RAMEND)
		ram[address - RAMSTART] = value;
}

void consoleLog(std::string s)
{
	cout << s << std::endl;
}

int main(int argc, char* argv[])
{
	mos6502 cpu(MemoryRead, MemoryWrite);

	if (argc < 2)
	{
		std::cout << "Filename required!" << std::endl;
		return 0;
	}

	if (loadRom(argv[1]) == 0)
	{
		std::cout << "Rom invalid!" << std::endl;
	}

	std::string caption("MCS65 Emulator - By Koen van Vliet");
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), caption);
	//window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);
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

	net.init();
	//dbgterm.printString("---- [Debug console] ----\r\n");
	//dbgterm.printString(versionInfo);
	cpu.Reset();
	//dbgterm.printString("Starting emulation\r\n");

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
						if (event.key.code == sf::Keyboard::Up)
						{
							term.setCursorTile(++ctile);
							std::cout << ctile << std::endl;
						}
						if (event.key.code == sf::Keyboard::Down)
						{
							term.setCursorTile(--ctile);
							std::cout << ctile << std::endl;
						}
					break;
				default:
					break;
			}
		}
		// Run a bunch of instructions
		cpu.Run(IPF);

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

