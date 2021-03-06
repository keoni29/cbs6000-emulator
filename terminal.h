/*
 * terminal.h
 *
 *  Created on: Nov 16, 2015
 *      Author: Koen van Vliet
 */

#ifndef TERMINAL_H_
#define TERMINAL_H_
#include <SFML/Graphics.hpp>
#include "comDevice.h"
#include <stdarg.h>

class terminal : public sf::Drawable, public sf::Transformable, public comDevice
{
public:
	terminal(int cols, int rows, int xoff, int yoff);
	~terminal();

	int16_t Read(uint8_t *buff, int16_t nbytes);
	int16_t Read(uint8_t & d);
	int16_t Write(uint8_t *buff, int16_t nbytes);
	int16_t Write(uint8_t d);

	void printString(const std::string& str);
	bool load(const std::string& tileset, int w, int h);
	void enableCursor(bool enable);
	void setTextColor(const sf::Color c);
	void feedChar(char c);
	void setCursorTile(int tileNumber);
	void update();
private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	void setTile(int x, int y, int tileNumber);
	sf::Color getColor(int x, int y);
	int getTile(int x, int y) const;
	void newLine();
	void tileColor(int x, int y, sf::Color col);
	void updateCursor();
	void scrollDown();
	void moveTile(sf::Vertex* tile, int x, int y);
	std::vector<char> rxBuffer;
	bool m_enableCursor;
	unsigned long int m_frameCount;
	int m_cx, m_cy;
	int m_cols,m_rows;
	int m_xoff, m_yoff;
	int m_tileWidth, m_tileHeight;
	sf::Color m_textColor;
	sf::VertexArray m_cursor;
	sf::VertexArray m_vertices;
	sf::Texture m_tileset;
	int * tiles;
};

#endif /* TERMINAL_H_ */

