#include "units.h"

units::units() {
	UnitType = "";
	side = '\0';
	mobility = -1;
	inix = -1;
	iniy = -1;
	alive = 0;
}

units::units(int x,int y, std::string ut, char s) {
	UnitType = ut;
	side = s;
	inix = x;
	iniy = y;
	alive = 1;
	reset_mobility();
}

units::units(const units& u) {
	UnitType = u.type();
	side = u.player();
	inix = u.get_inix();
	iniy = u.get_iniy();
	alive = u.is_alive();
	reset_mobility();
}

std::string units::type() const {
	return UnitType;
}

char units::player() const {
	return side;
}

int units::mob() const {
	return mobility;
}

void units::reset_mobility() {
	if (UnitType == "MG") mobility = 2;
	else if (UnitType == "AR") mobility = 3;
	else if (UnitType == "FT") mobility = 4;
	else if (UnitType == "KN") mobility = 5;
	else mobility = -1;
}

void units::reduce_mobility(int rm) {
	mobility -= rm;
}

int units::get_inix() const {
	return inix;
}

int units::get_iniy() const {
	return iniy;
}

void units::clear_mobility(){
	mobility = -1;
}

void units::set_dead() {
	alive = 0;
}

bool units::is_alive() const {
	return alive;
}