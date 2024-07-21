#pragma once
#include "units.h"

class field {

private:

	int rows, columns;
	char* terrains;
	units** units_on_battlefield = nullptr;

public:

	field();
	field(int x, int y);
	field(const field& f);
	void set_terrains(int x, int y, char t);
	void set_units(int x, int y, units* u);
	bool isunit(int x, int y) const;
	units* unit(int x, int y) const;
	units* unit(int i) const;
	char terrain(int x, int y) const;
	char terrain(int i) const;
	int row() const;
	int column() const;
	
};