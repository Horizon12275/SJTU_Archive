#include "linklistnode.h"

linklistnode::linklistnode() {
	position.x = linklistnode_x;
	position.y = linklistnode_y;
	data = "";
	next = NULL;
}

linklistnode::linklistnode(olc::vi2d p, std::string s) {
	position = p;
	data = s;
	next = NULL;
}

void linklistnode::connect(linklistnode & lln) {
	next = &lln;
}

void linklistnode::disconnect() {
	next = NULL;
}

bool linklistnode::inside_left(olc::vi2d p) {
	olc::vi2d diff;
	diff = p - position;
	if (diff.x > 0 && diff.x < linklistnode_x / 2 && diff.y > 0 && diff.y < linklistnode_y) return 1;
	else return 0;
}

bool linklistnode::inside_right(olc::vi2d p) {
	olc::vi2d diff;
	diff = p - position;
	if (diff.x > linklistnode_x / 2 && diff.x < linklistnode_x && diff.y > 0 && diff.y < linklistnode_y) return 1;
	else return 0;
}

olc::vi2d linklistnode::next_position() const  {
	return next->position;
}

void linklistnode::set_position(olc::vi2d p) {
	position = p;
}

void linklistnode::DrawSelf(olc::PixelGameEngine* pge) {

	olc::vi2d size(linklistnode_x / 2, linklistnode_y);
	olc::vi2d delta(linklistnode_x / 2, 0);
	olc::vi2d dtext(linklistnode_x / 10, linklistnode_y * 11 / 10);
	int font_size = std::max(int(linklistnode_y / 20 - data.length() / 4), 2);

	pge->FillRect(position, size, LINKLISTNODE_COLOR_DARK);
	pge->FillRect(position + delta, size, LINKLISTNODE_COLOR_BRIGHT);
	pge->DrawString(position + dtext, data, DATA_COLOR, font_size);

	/*
	if (next != NULL) {
		olc::vi2d d1 = { linklistnode_x * 3 / 4, linklistnode_y / 2 };
		olc::vi2d d2 = { linklistnode_x * 1 / 4, linklistnode_y / 2 };
		pge->DrawLine(position + d1, next_position() + d2, LINE_COLOR);
	}
	*/
}

olc::vi2d linklistnode::get_position() const {
	return position;
}

linklistnode linklistnode::next_node() const {
	linklistnode buffer;
	if (next == NULL) return buffer;
	return *next;
}

bool linklistnode::operator== (const linklistnode& other) {
	if (data == other.get_data() && next == other.get_next() && position == other.get_position() ) return 1;
	else return 0;
}

std::string linklistnode::get_data() const {
	return data;
}

linklistnode* linklistnode::get_next() const{
	return next;
}

void linklistnode::set_dead() {
	data = "gg";
}

std::string linklistnode::get_next_data() const {
	return next->get_data();
}

bool linklistnode::is_alive() {
	if (data == "gg") return 0;
	else return 1;
}

void linklistnode::set_data(std::string input) {
	data = input;
}