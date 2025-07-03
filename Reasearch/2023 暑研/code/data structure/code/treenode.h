#pragma once
#include "olcPixelGameEngine.h"
#include "Colors.h"
#define treenode_r 45

class treenode {

private:
	olc::vi2d position;
	std::string data;
	std::list<treenode*> next;


public:
	treenode();
	treenode(olc::vi2d p, std::string s = "");
	bool inside(olc::vi2d p);
	bool operator== (const treenode& other);
	bool is_alive();
	olc::vi2d get_position() const;
	std::string get_data() const;
	std::list<treenode*> get_next() const;
	void connect(treenode& lln);
	void disconnect();
	void set_dead();
	void set_position(olc::vi2d p);
	void set_data(std::string input);
	void DrawSelf(olc::PixelGameEngine* pge);
};

static olc::Pixel treenode_COLOR_DARK(0, 202, 204);
static olc::Pixel treenode_COLOR_BRIGHT(255, 255, 255); 
