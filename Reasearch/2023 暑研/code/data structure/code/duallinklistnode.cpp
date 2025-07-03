#include "duallinklistnode.h"

duallinklistnode::duallinklistnode() {
	position.x = duallinklistnode_x;
	position.y = duallinklistnode_y;
	data = "";
	next = NULL;
	front = NULL;
}

duallinklistnode::duallinklistnode(olc::vi2d p, std::string s) {
	position = p;
	data = s;
	next = NULL;
	front = NULL;
}

void duallinklistnode::connect_next(duallinklistnode& lln) {
	next = &lln;
}

void duallinklistnode::connect_front(duallinklistnode& lln) {
	front = &lln;
}

void duallinklistnode::disconnect_next() {
	next = NULL;
}

void duallinklistnode::disconnect_front() {
	front = NULL;
}

bool duallinklistnode::inside_left(olc::vi2d p) {
	olc::vi2d diff;
	diff = p - position;
	if (diff.x > 0 && diff.x < duallinklistnode_x / 4 && diff.y > 0 && diff.y < duallinklistnode_y) return 1;
	else return 0;
}

bool duallinklistnode::inside_right(olc::vi2d p) {
	olc::vi2d diff;
	diff = p - position;
	if (diff.x > duallinklistnode_x * 3 / 4 && diff.x < duallinklistnode_x && diff.y > 0 && diff.y < duallinklistnode_y) return 1;
	else return 0;
}

bool duallinklistnode::inside_middle(olc::vi2d p) {
	olc::vi2d diff;
	diff = p - position;
	if (diff.x > duallinklistnode_x / 4 && diff.x < duallinklistnode_x * 3 / 4 && diff.y > 0 && diff.y < duallinklistnode_y) return 1;
	else return 0;
}

olc::vi2d duallinklistnode::next_position() const {
	return next->position;
}

void duallinklistnode::set_position(olc::vi2d p) {
	position = p;
}

void duallinklistnode::DrawSelf(olc::PixelGameEngine* pge) {

	olc::vi2d size_1(duallinklistnode_x / 4, duallinklistnode_y);
	olc::vi2d size_2(duallinklistnode_x / 2, duallinklistnode_y);
	olc::vi2d delta_1(duallinklistnode_x / 4, 0);
	olc::vi2d delta_2(duallinklistnode_x * 3 / 4, 0);
	olc::vi2d dtext(duallinklistnode_x / 10, duallinklistnode_y * 11 / 10);
	int font_size = std::max(int(duallinklistnode_y / 20 - data.length() / 4), 2);

	pge->FillRect(position, size_1, duallinklistnode_COLOR_BRIGHT);
	pge->FillRect(position + delta_1, size_2, duallinklistnode_COLOR_DARK);
	pge->FillRect(position + delta_2, size_1, duallinklistnode_COLOR_BRIGHT);
	pge->DrawString(position + dtext, data, DATA_COLOR, font_size);

	/*
	if (next != NULL) {
		olc::vi2d d1 = { duallinklistnode_x * 3 / 4, duallinklistnode_y / 2 };
		olc::vi2d d2 = { duallinklistnode_x * 1 / 4, duallinklistnode_y / 2 };
		pge->DrawLine(position + d1, next_position() + d2, LINE_COLOR);
	}
	*/
}

olc::vi2d duallinklistnode::get_position() const {
	return position;
}

duallinklistnode duallinklistnode::next_node() const {
	duallinklistnode buffer;
	if (next == NULL) return buffer;
	return *next;
}

bool duallinklistnode::operator== (const duallinklistnode& other) {
	if (data == other.get_data() && next == other.get_next() && position == other.get_position()) return 1;
	else return 0;
}

std::string duallinklistnode::get_data() const {
	return data;
}

duallinklistnode* duallinklistnode::get_next() const {
	return next;
}

void duallinklistnode::set_dead() {
	data = "gg";
}

std::string duallinklistnode::get_next_data() const {
	return next->get_data();
}

bool duallinklistnode::is_alive() {
	if (data == "gg") return 0;
	else return 1;
}

void duallinklistnode::set_data(std::string input) {
	data = input;
}

std::string duallinklistnode::get_front_data() const {
	return front->get_data();
}

duallinklistnode* duallinklistnode::get_front() const {
	return front;
}

duallinklistnode duallinklistnode::front_node() const {
	duallinklistnode buffer;
	if (front == NULL) return buffer;
	return *front;
}

olc::vi2d duallinklistnode::front_position() const {
	return front->position;
}
