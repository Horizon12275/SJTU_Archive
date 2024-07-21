#include "field.h"

field::field() {
	rows = 0;
	columns = 0;
	units_on_battlefield = nullptr;
	terrains = nullptr;
}

field::field(int x, int y) {
	rows = x;
	columns = y;
	units_on_battlefield = new units * [x * y + 1];
	terrains = new char[x * y + 1];
	for (int i = 0; i <= x * y; i++) {
		units_on_battlefield[i] = nullptr;
		terrains[i] = '\0';
	}
}

field::field(const field& f) {
	rows = f.row();
	columns = f.column();
	units_on_battlefield = new units * [rows * columns + 1];
	terrains = new char[rows * columns + 1];
	for (int i = 0; i <= rows * columns; i++) {
		units_on_battlefield[i] = nullptr;
		terrains[i] = '\0';
	}
	for (int i = 0; i <= rows * columns; i++) {
		units utmp(*f.unit(i));
		units* new_unit = &utmp;
		units_on_battlefield[i] = new_unit;
		terrains[i] = f.terrain(i);
	}
}

void field::set_terrains(int x, int y, char t) {
	terrains[columns * (x - 1) + y] = t;
}

void field::set_units(int x, int y, units* u) {
	units_on_battlefield[columns * (x - 1) + y] = u;
}

int field::row() const {
	return rows;
}

int field::column() const {
	return columns;
}

units* field::unit(int x, int y) const {
	return units_on_battlefield[columns * (x - 1) + y];
}

units* field::unit(int i) const {
	return units_on_battlefield[i];
}

bool field::isunit(int x, int y) const {
	if (units_on_battlefield[columns * (x - 1) + y] == nullptr) return 0;
	else return 1;
}

char field::terrain(int x, int y) const {
	return terrains[columns * (x - 1) + y];
}

char field::terrain(int i) const {
	return terrains[i];
}