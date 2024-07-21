#include "treenode.h"

treenode::treenode() {
	position = {treenode_r, treenode_r};
	data = "";
}

treenode::treenode(olc::vi2d p, std::string s) {
	position = p;
	data = s;
}

void treenode::connect(treenode& lln) {
	next.push_back(& lln);
}

void treenode::disconnect() {
	if (!next.empty()) next.pop_back();
}

bool treenode::inside(olc::vi2d p) {
	olc::vi2d diff;
	diff = p - position;
	if (diff.mag() < treenode_r) return 1;
	else return 0;
}

void treenode::set_position(olc::vi2d p) {
	position = p;
}

void treenode::DrawSelf(olc::PixelGameEngine* pge) {

	int delta = treenode_r / 20;
	olc::vi2d dtext(-treenode_r / 3 + delta, -treenode_r / 3 + delta);
	int font_size = std::max(int(treenode_r / 10 - data.length() / 2), 2);

	olc::vi2d d = { - int( (data.length() - 1) * delta) * treenode_r / 9 , 0};

	pge->FillCircle(position, treenode_r, treenode_COLOR_DARK);
	pge->DrawString(position + dtext + d, data, treenode_COLOR_BRIGHT, font_size);

}

olc::vi2d treenode::get_position() const {
	return position;
}

bool treenode::operator== (const treenode& other) {
	if (data == other.get_data() && position == other.get_position() && next == other.get_next()) return 1;
	else return 0;
}

std::string treenode::get_data() const {
	return data;
}

void treenode::set_dead() {
	data = "gg";
	next.clear();
}

bool treenode::is_alive() {
	if (data == "gg") return 0;
	else return 1;
}

void treenode::set_data(std::string input) {
	data = input;
}

std::list<treenode*> treenode::get_next() const  {
	return next;
}

