#pragma once
#include "olcPixelGameEngine.h"
#include "Colors.h"
#define duallinklistnode_x 180
#define duallinklistnode_y 90

class duallinklistnode {

private:
	olc::vi2d position;
	std::string data;
	duallinklistnode* next;
	duallinklistnode* front;

public:
	duallinklistnode();
	duallinklistnode(olc::vi2d p, std::string s = "");
	bool inside_left(olc::vi2d p);
	bool inside_right(olc::vi2d p);
	bool inside_middle(olc::vi2d p);
	bool operator== (const duallinklistnode& other);
	bool is_alive();
	olc::vi2d get_position() const;
	olc::vi2d next_position() const;
	olc::vi2d front_position() const;
	duallinklistnode next_node() const;
	duallinklistnode front_node() const;
	duallinklistnode* get_next() const;
	duallinklistnode* get_front() const;
	std::string get_data() const;
	std::string get_next_data() const;
	std::string get_front_data() const;
	void connect_next(duallinklistnode& lln);
	void connect_front(duallinklistnode& lln);
	void disconnect_next();
	void disconnect_front();
	void set_dead();
	void set_position(olc::vi2d p);
	void set_data(std::string input);
	void DrawSelf(olc::PixelGameEngine* pge);
};

static olc::Pixel duallinklistnode_COLOR_DARK(0, 202, 204);
static olc::Pixel duallinklistnode_COLOR_BRIGHT(255, 255, 255);
