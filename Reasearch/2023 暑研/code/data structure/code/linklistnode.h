#pragma once
#include "olcPixelGameEngine.h"
#include "Colors.h"
#define linklistnode_x 180
#define linklistnode_y 90

class linklistnode {

private:
	olc::vi2d position;
	std::string data;
	linklistnode* next;


public:
	linklistnode();
	linklistnode(olc::vi2d p, std::string s = "");
	bool inside_left(olc::vi2d p);
	bool inside_right(olc::vi2d p);
	bool operator== (const linklistnode& other);
	bool is_alive();
	olc::vi2d get_position() const;
	olc::vi2d next_position() const;
	linklistnode next_node() const;
	linklistnode* get_next() const;
	std::string get_data() const;
	std::string get_next_data() const;
	void connect(linklistnode & lln);
	void disconnect();
	void set_dead();
	void set_position(olc::vi2d p);
	void set_data(std::string input);
	void DrawSelf(olc::PixelGameEngine* pge);
};

static olc::Pixel LINKLISTNODE_COLOR_DARK(0, 202, 204);
static olc::Pixel LINKLISTNODE_COLOR_BRIGHT(255, 255, 255);