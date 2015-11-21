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


// Terminal emulators and networking stuff
tcp net(consoleLog);
terminal term(80,30,0,0);

// Terminals for debug window
terminal dbgterm(40,16,41 * 8,0);
terminal dasmterm(40,9,0,7*8);
terminal regterm(25,6,0,0);
terminal zpterm(86,33,0,0);

// Addressable devices
acia acia1(&term);
acia acia2(&net);
memory ram(65536, 2, false);
memory rom(8192, 1, true);

// todo settle with optimal dimensions
#define WINDOW_WIDTH 80 * 8
#define WINDOW_HEIGHT 30 * 16

#define DBGWINDOW_WIDTH 82 * 8
#define DBGWINDOW_HEIGHT 16 * 16

#define ZPWINDOW_WIDTH 86 * 8
#define ZPWINDOW_HEIGHT 33 * 16

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
		// tODO FIX THIS
		Direction = data;
		return;
	}

	if (address == PORT)
	{
		// TODO FIX THIS
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
	// Todo optimize this string buffer size?
	char str[400];

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
	sf::RenderWindow dbgwindow(sf::VideoMode(DBGWINDOW_WIDTH, DBGWINDOW_HEIGHT), caption);
	sf::RenderWindow zpwindow(sf::VideoMode(ZPWINDOW_WIDTH, ZPWINDOW_HEIGHT), caption);
	window.setFramerateLimit(FPS);
	dbgwindow.setFramerateLimit(FPS);
	zpwindow.setFramerateLimit(FPS);

	if (!term.load("terminal8x16.png",8,16))
	{
		std::cout << "Failed to open bitmap font file!" << std::endl;
		return 0;
	}

	if (!dbgterm.load("terminal8x16.png",8,16))
	{
		std::cout << "Failed to open bitmap font file!" << std::endl;
		return 0;
	}

	if (!dasmterm.load("terminal8x16.png",8,16))
	{
		std::cout << "Failed to open bitmap font file!" << std::endl;
		return 0;
	}

	if (!regterm.load("terminal8x16.png",8,16))
	{
		std::cout << "Failed to open bitmap font file!" << std::endl;
		return 0;
	}

	if (!zpterm.load("terminal8x16.png",8,16))
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
	regterm.enableCursor(false);
	regterm.setTextColor(sf::Color::White);
	zpterm.enableCursor(false);
	zpterm.setTextColor(sf::Color::White);

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
		if (SPEED <= 20)
		{
			SHOWDASM = true;
		}

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
						consoleLog("F8 to continue, F5 to step");
					}

					// Stop/Resume execution
					if (event.key.code == sf::Keyboard::F8)
					{
						if (HALT)
						{
							HALT = false;
							consoleLog("Resumed execution...");
						}
						else
						{
							HALT = true;
							SHOWDASM = true;
							consoleLog("F8 to continue, F5 to step");
						}
					}
					// Execute single instruction
					if (event.key.code == sf::Keyboard::F5)
					{
						if (HALT)
						{
							cpu.Run(1);
							SHOWDASM = true;
						}
					}


					if (event.key.code == sf::Keyboard::End)
					{
						// Reset Cpu and internal IO port for the 6510
						Direction = 0x00;
						PortOut = 0x00;
						cpu.Reset();
						HALT = false;
						consoleLog("Reset CPU");
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
			// Show live disassembly of the memory
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

			// Display CPU register contents
			char charA = state[0], charX = state[1], charY = state[2];

			if (charA < ' ')
			{
				charA = ' ';
			}
			if (charX < ' ')
			{
				charX = ' ';
			}
			if (charY < ' ')
			{
				charY = ' ';
			}

			sprintf(str,"\r\n"
						"A:  $%02X  %03d \'%c\'\r\n"
						"X:  $%02X  %03d \'%c\'\r\n"
						"Y:  $%02X  %03d \'%c\'\r\n"
						"PC: $%04X\r\n"
						"SP: $%02X  %03d\r\n"
						"SR: $%02X, Z: %d, C: %d",
						state[0],
						state[0],
						charA,
						state[1],
						state[1],
						charX,
						state[2],
						state[2],
						charY,
						state[3],
						state[4],
						state[4],
						state[5],
						(state[5]>>1) & 1,
						state[5] & 1);
			regterm.printString(str);

			// Show zero page contents
			zpterm.setTextColor(sf::Color::Cyan);
			zpterm.printString("\r\n----: -0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F");
			zpterm.setTextColor(sf::Color::White);
			for(int i = 0; i < 16; i++)
			{
				uint8_t d;
				zpterm.printString("\r\n");
				sprintf(str, "%04X: ", (uint16_t)(i * 16));
				zpterm.printString(str);
				for(int j = 0; j < 16; j++)
				{
					d = MemoryRead(i*16 + j);
					if (j == 15)
						sprintf(str,"%02X",d);
					else
						sprintf(str,"%02X,",d);
					if(i*16 + j >= 0x5C && i*16+j < 0x5C+ 0x27)
						zpterm.setTextColor(sf::Color::Green);
					zpterm.printString(str);
					zpterm.setTextColor(sf::Color::White);
				}
				for(int j = 0; j < 16; j++)
				{
					d = MemoryRead(i*16 + j);
					if (d < ' ')
					{
						d = ' ';
					}
					if(i*16 + j >= 0x5C && i*16+j < 0x5C+ 0x27)
						zpterm.setTextColor(sf::Color::Green);
					zpterm.Write(d);
					zpterm.setTextColor(sf::Color::Blue);
					zpterm.Write(ctile);
					zpterm.setTextColor(sf::Color::White);
				}
				zpterm.setTextColor(sf::Color::Blue);
				for(int i = 0; i< 86; i++)
					zpterm.Write(177);
				zpterm.setTextColor(sf::Color::White);
			}
		}

		// Update the screen
		window.clear();
		dbgwindow.clear();
		zpwindow.clear();
		net.update();
		term.update();
		dbgterm.update();
		dasmterm.update();
		zpterm.update();
		regterm.update();
		window.draw(term);
		dbgwindow.draw(dbgterm);
		zpwindow.draw(zpterm);
		//if(HALT)
		//{
			dbgwindow.draw(dasmterm);
			dbgwindow.draw(regterm);
		//}
		window.display();
		dbgwindow.display();
		zpwindow.display();
	}
	return 0;
}

