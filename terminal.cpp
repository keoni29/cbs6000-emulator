/*
 * terminal.cpp
 *
 *  Created on: Nov 16, 2015
 *      Author: Koen van Vliet
 */


#include "terminal.h"

terminal::terminal(int cols, int rows, int xoff, int yoff) :  m_enableCursor(true), m_frameCount(0), m_cx(0), m_cy(0), m_tileWidth(0), m_tileHeight(0), m_textColor(sf::Color::Yellow){
	m_xoff = xoff;
	m_yoff = yoff;
	m_cols = cols;
	m_rows = rows;
	tiles = new int[m_cols * m_rows];
	for(int i = 0; i < m_cols * m_rows; i++)
		tiles[i] = ' ';
};

terminal::~terminal()
{
	delete[] tiles;
}

int16_t terminal::Read(uint8_t &d)
{
	if (rxBuffer.size() == 0)
	{
		return 0;
	}
	d = rxBuffer.front();
	rxBuffer.erase(rxBuffer.begin());
	return 1;
}

int16_t terminal::Read(uint8_t *buff, int16_t nbytes)
{
	int i;
	for(i = 0; i < nbytes; i++)
	{
		if (!Read(buff[i]))
			break;
	}
	return i;	// Return amount of received characters.
}

int16_t terminal::Write(uint8_t d)
{
	/* Backspace character */
	if (d == 0x8)
	{
		d = ' ';
		if (m_cx)
		{
			m_cx --;
		}
		else
		{
			if (m_cy)
			{
				m_cx = m_cols - 1;
				m_cy --;
			}
		}
		setTile(m_cx, m_cy, (int)d);
	}
	else if (d == 0xD)
	{
		m_cx = 0;
	}
	else if (d == 0xA)
	{
		newLine();
	}
	else
	{
		tileColor(m_cx, m_cy, m_textColor);
		setTile(m_cx, m_cy, (int)d);
		if (++m_cx == m_cols)
		{
			newLine();
		}
	}

	updateCursor();
	return 1;
}

int16_t terminal::Write(uint8_t *buff, int16_t nbytes)
{
	int i;
	for(i = 0; i < nbytes; i ++)
	{
		if(!Write(buff[i]))
			break;
	}
	return i;		// Return amount of characters that have been displayed
}

void terminal::printString(const std::string& str)
{
	for(uint16_t i = 0; i < str.length(); i++)
	{
		Write(str.at(i));
	}
}

void terminal::feedChar(char c)
{
	rxBuffer.push_back(c);
}

bool terminal::load(const std::string& tileset, int w, int h)
{
	m_tileWidth = w;
	m_tileHeight = h;
	// load the tileset texture
	if (!m_tileset.loadFromFile(tileset))
		return false;

	// resize the vertex array to fit the level size
	m_vertices.setPrimitiveType(sf::Quads);
	m_vertices.resize(m_cols * m_rows * 4);

	m_cursor.setPrimitiveType(sf::Quads);
	m_cursor.resize(4);

	// populate the vertex array, with one quad per tile
	for (int i = 0; i < m_cols; ++i)
	{
		for (int j = 0; j < m_rows; ++j)
		{
			// get the current tile number
			int tileNumber = tiles[i + j * m_cols];

			// get a pointer to the current tile's quad
			sf::Vertex* quad = &m_vertices[(i + j * m_cols) * 4];

			moveTile(quad, i, j);

			setTile(i, j, tileNumber);
		}
	}
	updateCursor();
	return true;
}

void terminal::setCursorTile(int tileNumber)
{
	// get a pointer to the current tile's quad
	sf::Vertex* quad = &m_cursor[0];
	int tu = tileNumber % (m_tileset.getSize().x / m_tileWidth);
	int tv = tileNumber / (m_tileset.getSize().x / m_tileWidth);
	quad[0].texCoords = sf::Vector2f(tu * m_tileWidth, tv * m_tileHeight);
	quad[1].texCoords = sf::Vector2f((tu + 1) * m_tileWidth, tv * m_tileHeight);
	quad[2].texCoords = sf::Vector2f((tu + 1) * m_tileWidth, (tv + 1) * m_tileHeight);
	quad[3].texCoords = sf::Vector2f(tu * m_tileWidth, (tv + 1) * m_tileHeight);
}

void terminal::setTextColor(sf::Color c)
{
	m_textColor = c;
}

void terminal::enableCursor(bool enable)
{
	m_enableCursor = enable;
}

void terminal::updateCursor()
{
	m_frameCount = 0;
	// Update the cursor position
	// get a pointer to the cursor quad
	sf::Vertex* quad = &m_cursor[0];
	moveTile(quad, m_cx, m_cy);
}

void terminal::moveTile(sf::Vertex* tile, int x, int y)
{
	tile[0].position = sf::Vector2f(x * m_tileWidth + m_xoff, y * m_tileHeight + m_yoff);
	tile[1].position = sf::Vector2f((x + 1) * m_tileWidth + m_xoff, y * m_tileHeight + m_yoff);
	tile[2].position = sf::Vector2f((x + 1) * m_tileWidth + m_xoff, (y + 1) * m_tileHeight + m_yoff);
	tile[3].position = sf::Vector2f(x * m_tileWidth + m_xoff, (y + 1) * m_tileHeight + m_yoff);
}

void terminal::newLine()
{
	m_cx = 0;
	if (m_cy == m_rows - 1)
	{
		scrollDown();
	}
	else
	{
		m_cy ++;
	}
}

void terminal::scrollDown()
{
	// populate the vertex array, with one quad per tile
	for (int i = 0; i < m_cols; ++i)
	{
		for (int j = 0; j < m_rows; ++j)
		{
			// get the current tile number
			int tileNumber;
			sf::Color col;
			if (j == m_rows - 1)
			{
				tileNumber = ' ';
				col = m_textColor;
			}
			else
			{
				tileNumber= getTile(i, j + 1);
				col = getColor(i, j+1);
			}
			setTile(i, j, tileNumber);
			tileColor(i, j, col);
		}
	}
}

void terminal::setTile(int x, int y, int tileNumber)
{
	tiles[y * m_cols + x] = tileNumber;
	int tu = tileNumber % (m_tileset.getSize().x / m_tileWidth);
	int tv = tileNumber / (m_tileset.getSize().x / m_tileWidth);
	// get a pointer to the current tile's quad
	sf::Vertex* quad = &m_vertices[(x + y * m_cols) * 4];
	// define its 4 texture coordinates
	quad[0].texCoords = sf::Vector2f(tu * m_tileWidth, tv * m_tileHeight);
	quad[1].texCoords = sf::Vector2f((tu + 1) * m_tileWidth, tv * m_tileHeight);
	quad[2].texCoords = sf::Vector2f((tu + 1) * m_tileWidth, (tv + 1) * m_tileHeight);
	quad[3].texCoords = sf::Vector2f(tu * m_tileWidth, (tv + 1) * m_tileHeight);
}

void terminal::tileColor(int x, int y, sf::Color col)
{
	// get a pointer to the current tile's quad
	sf::Vertex* quad = &m_vertices[(x + y * m_cols) * 4];
	// define its 4 texture coordinates
	quad[0].color = col;
	quad[1].color = col;
	quad[2].color = col;
	quad[3].color = col;
}

sf::Color terminal::getColor(int x, int y)
{
	sf::Vertex* quad = &m_vertices[(x + y * m_cols) * 4];
	return quad[0].color;
}

int terminal::getTile(int x, int y) const
{
	return tiles[y * m_cols + x];
}

void terminal::update()
{
	m_frameCount = (m_frameCount + 1);
}

void terminal::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// apply the transform
	states.transform *= getTransform();

	// apply the tileset texture
	states.texture = &m_tileset;

	// draw the vertex array
	target.draw(m_vertices, states);

	// draw the cursor
	if (m_enableCursor)
		if(!(m_frameCount % 30 / 15))
			target.draw(m_cursor, states);
}
