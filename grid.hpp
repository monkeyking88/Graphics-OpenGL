#pragma once

class Grid
{
public:
	Grid( size_t dim );
	~Grid();

	void reset();

	size_t getDim() const;

	float getHeight( float x, float y ) const;
	float getColour( float x, float y ) const;

	void setHeight( float x, float y, float h );
	void setColour( float x, float y, float c );
	
private:
	size_t m_dim;
	float *m_heights;
	float *m_cols;
};
