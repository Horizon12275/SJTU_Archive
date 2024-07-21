#pragma once
#define block_size 75
#include "field.h"
#include "units.h"
#include<map>

class Battlefield : public olc::PixelGameEngine {

private:

	field current_field;
	std::vector<units> current_units;
	bool is_ended;
	int current_state;		//1 for vaild left mouse button already clicked    0 for no invalid click left mouse button
	int current_input_row, current_input_column, current_selected_row, current_selected_column;
	int current_player;		//1 for player A	0 for player B		in this turn
	char current_skill;

	//Sprite
	std::unique_ptr<olc::Sprite> Mountain, Water, Forest, Abyss, Grass, Knight_A, Footman_A, Archer_A, Mage_A, Knight_B, Footman_B, Archer_B, Mage_B;
	olc::vi2d sprite_size;

	//Decal
	std::unique_ptr<olc::Sprite> sprFragment;
	std::unique_ptr<olc::Decal> decFragment;

	float fTargetFrameTime = 1.0f / 100.0f;
	float fAccumulatedTime = 0.0f;

	struct sFragment
	{
		olc::vf2d pos;
		olc::vf2d vel;
		float fAngle;
		float fTime;
		olc::Pixel colour;
	};

	std::list<sFragment> listFragments;
	std::list<sFragment> listFragments_fireball;

public:

	Battlefield(field& f, std::vector<units>& u);
	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;
	bool left_mouse_button_is_valid();
	bool right_mouse_button_to_move_is_valid();
	bool right_mouse_button_to_attack_melee_is_valid();
	bool right_mouse_button_to_attack_remote_is_valid();
	bool right_mouse_button_to_fireball_is_valid();
	bool right_mouse_button_to_earthquake_is_valid();
	bool inside(int x, int y) const;
	bool beside(int x1, int y1, int x2, int y2) const;
	bool in_max_range(int x1, int y1, int x2, int y2, int r) const;
	bool out_min_range(int x1, int y1, int x2, int y2, int r) const;
	bool no_mountain(int s_x, int s_y, int i_x, int i_y) const;
	bool is_melee_unit(int x, int y) const;
	bool not_ally(int x1, int y1, int x2, int y2) const;
	void detect_ended();
	void detect_selected();
	void set_input();
	void reset_input();
	void set_selected();
	void reset_selected();
	void print_Battlefield();
	void print_info();
	void move();
	void attack();
	void fireball(int x, int y, olc::vi2d v);
	void earthquake(int x, int y);
	void burn(int x, int y);
	void flood(int x, int y, std::vector<std::vector<int>>& v);
	void flood_map();
	void fireball_effects(float x, float y, olc::vf2d velocity);
	void death_effects(float x, float y);
	void burn_effects(float x, float y);
	void earthquake_effects(float x, float y);
	void update_fragment(float fElapsedTime);
	void draw_fragment();
	void remove_dead_fragment();
	void new_turn();
	void print_ended();
	void print_skill();
};