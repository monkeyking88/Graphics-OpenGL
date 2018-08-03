#include <algorithm>

#include "grid.hpp"

Grid::Grid( size_t d )
	: m_dim( d )
{
	m_heights = new float[ d * d ];
	m_cols = new float[ d * d ];

	reset();
}

void Grid::reset()
{
	size_t sz = m_dim*m_dim;
	std::fill( m_heights, m_heights + sz, 0 );
	std::fill( m_cols, m_cols + sz, 0 );
}

Grid::~Grid()
{
	delete [] m_heights;
	delete [] m_cols;
}

size_t Grid::getDim() const
{
	return m_dim;
}

float Grid::getHeight( float x, float y ) const
{
	return m_heights[ (int)(y * m_dim + x) ];
}

float Grid::getColour( float x, float y ) const
{
	return m_cols[ (int)(y * m_dim + x) ];
}

void Grid::setHeight( float x, float y, float h )
{
	m_heights[ (int)(y * m_dim + x) ] = h;
}

void Grid::setColour( float x, float y, float c )
{
	m_cols[ (int)(y * m_dim + x) ] = c;
}
