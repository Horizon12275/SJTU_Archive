#pragma once
#include "olcPixelGameEngine.h"

class units {

private:

	std::string UnitType;
	char side;
	int mobility,inix,iniy;
	bool alive;

public:

	units();
	units(int x, int y, std::string ut, char s);
	units(const units & u);
	std::string type() const;
	char player() const;
	int mob() const;
	int get_inix() const;
	int get_iniy() const;
	bool is_alive() const;
	void set_dead();
	void reset_mobility();
	void clear_mobility();
	void reduce_mobility(int rm);

};