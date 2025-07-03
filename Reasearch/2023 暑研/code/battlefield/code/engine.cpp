#include "engine.h"

Battlefield::Battlefield(field& f, std::vector<units>& u)
{
	sAppName = "Battlefield";
	current_field = f;

	/*
	std::cout << current_field.unit(2, 2) << std::endl;
	*/

	current_units = u;
	for (int i = 0; i < current_units.size(); i++)
		if(current_units[i].is_alive())
			current_field.set_units(current_units[i].get_inix(), current_units[i].get_iniy(), &current_units[i]);

	//std::cout << current_field.unit(2, 2) << std::endl;
	//std::cout << current_units[0].get_inix()<<" " << current_units[0].get_iniy() << " " << &current_units[0] << std::endl;
	//std::cout << current_field.unit(2, 2) << std::endl;
	//std::cout << current_field.unit(2, 2)->type() << std::endl;

	current_state = 0;	//1 for vaild left mouse button already clicked    0 for no invalid click left mouse button
	current_input_row = -1, current_input_column = -1, current_selected_row = -1, current_selected_column = -1;
	current_player = 0;	//1 for player A	0 for player B		in this turn
	current_skill = '\0';
	is_ended = 0;		//1 for already ended	0 for not ended
	new_turn();
	sprite_size = { block_size, block_size };

	flood_map();
}

bool Battlefield::OnUserCreate()
{
	//initialization

	srand((unsigned)time(NULL));

	//Load the sprite

	Mountain = std::make_unique<olc::Sprite>("./resource/Mountain.png");
	Water = std::make_unique<olc::Sprite>("./resource/Water.png");
	Forest = std::make_unique<olc::Sprite>("./resource/Forest.png");
	Abyss = std::make_unique<olc::Sprite>("./resource/Abyss.png");
	Grass = std::make_unique<olc::Sprite>("./resource/Grass.png");
	Knight_A = std::make_unique<olc::Sprite>("./resource/Knight_A.png");
	Footman_A = std::make_unique<olc::Sprite>("./resource/Footman_A.png");
	Archer_A = std::make_unique<olc::Sprite>("./resource/Archer_A.png");
	Mage_A = std::make_unique<olc::Sprite>("./resource/Mage_A.png");
	Knight_B = std::make_unique<olc::Sprite>("./resource/Knight_B.png");
	Footman_B = std::make_unique<olc::Sprite>("./resource/Footman_B.png");
	Archer_B = std::make_unique<olc::Sprite>("./resource/Archer_B.png");
	Mage_B = std::make_unique<olc::Sprite>("./resource/Mage_B.png");


	//Load Fragment Sprite
	sprFragment = std::make_unique<olc::Sprite>("./resource/tut_fragment.png");

	//Create decal of fragment
	decFragment = std::make_unique<olc::Decal>(sprFragment.get());

	return true;
}

bool Battlefield::OnUserUpdate(float fElapsedTime)
{

	//std::cout << current_input_row <<" "<< current_input_column << " " << current_selected_row << " " << current_selected_column << std::endl;

	if (current_state == 0 || 
		current_field.unit(current_selected_row, current_selected_column)->type() != "MG" || 
		current_field.unit(current_selected_row, current_selected_column)->mob() == -1) {
		current_skill = '\0';
	}

	if (GetMouse(0).bPressed) {	//left mouse button detect

		/*
		std::cout << int(GetMouseX()) << " " << int(GetMouseY()) << std::endl;
		std::cout << int(GetMouseX()) / block_size << " " << int(GetMouseY()) / block_size << std::endl;
		*/

		set_input();

		if (left_mouse_button_is_valid()) {
			//std::cout << "is_valid" << std::endl;
			current_state = 1;
			set_selected();
		}
		else {
			//std::cout << "not_valid" << std::endl;
			current_state = 0;
			reset_selected();
		}
	}
	
	if (GetMouse(1).bPressed && current_state == 1 && current_skill == '\0') {	//right mouse button detect after valid left mouse button input

		set_input();

		if (current_field.isunit(current_input_row, current_input_column) == 0 &&
			right_mouse_button_to_move_is_valid()) {		//if there's no unit then move
			move();
		}
		else if (current_field.isunit(current_input_row, current_input_column) == 1 &&
			right_mouse_button_to_attack_melee_is_valid()) {	//if there's one unit input, check the melee attackability , if true then attack
			attack();
			//move();
		}
		else if (current_field.isunit(current_input_row, current_input_column) == 1 &&
			right_mouse_button_to_attack_remote_is_valid()) {	//if there's one unit input, check the remote attackability , if true then attack
			attack();
		}
		else reset_input();
			//in other situations remain the selected button and the current state
	}

	if (GetMouse(1).bPressed && current_state == 1 && current_skill == 'F' && current_field.unit(current_selected_row, current_selected_column)->mob() != -1) {

		set_input();

		if (right_mouse_button_to_fireball_is_valid()) {

			//std::cout << "Fireball" << std::endl;

			olc::vi2d velocity = { current_input_row - current_selected_row, current_input_column - current_selected_column };
			fireball(current_input_row, current_input_column, velocity);
		}
		else reset_input();

		detect_selected();
	}

	if (GetMouse(1).bPressed && current_state == 1 && current_skill == 'E' && current_field.unit(current_selected_row, current_selected_column)->mob() != -1) {

		set_input();

		if (right_mouse_button_to_earthquake_is_valid()) {

			//std::cout << "Earthquake" << std::endl;

			earthquake(current_input_row, current_input_column);
			flood_map();
		}
		else reset_input();

		detect_selected();
	}

	if (current_state == 1 && 
		current_field.unit(current_selected_row, current_selected_column)->type() == "MG" &&
		current_field.unit(current_selected_row, current_selected_column)->mob() != -1) {
		if (GetKey(olc::F).bPressed) {
			current_skill = 'F';
		}
		else if (GetKey(olc::E).bPressed) {
			current_skill = 'E';
		}
		else if (GetKey(olc::CTRL).bPressed) {
			current_skill = '\0';
		}
	}

	detect_ended();

	detect_selected();

	update_fragment(fElapsedTime);

	remove_dead_fragment();

	if (is_ended == 0) {
		
		print_Battlefield();	//draw the current map
		print_info();
		draw_fragment();
		print_skill();
		
	}
	else if (is_ended == 1) print_ended();	//draw the ended map
	
	if (GetKey(olc::SPACE).bPressed) new_turn();	//start the new turn

	return true;
}

bool Battlefield::inside(int x, int y) const {
	if (x >= 1 && x <= current_field.row() && 
		y >= 1 && y <= current_field.column()) return 1;
	else return 0;
}

bool Battlefield::left_mouse_button_is_valid() {		//left mouse button to select
	if (inside(current_input_row, current_input_column) && 
		current_field.isunit(current_input_row, current_input_column)) 
		return 1;
	else 
		return 0;
}

bool Battlefield::right_mouse_button_to_move_is_valid() {	//right mouse button to act
	if (inside(current_input_row, current_input_column) &&
		beside(current_input_row, current_input_column, current_selected_row, current_selected_column))
		return 1;
	else 
		return 0;
}

bool Battlefield::right_mouse_button_to_attack_melee_is_valid() {	//right mouse button to act
	if (inside(current_input_row, current_input_column) &&

		/*
		not_ally(current_input_row, current_input_column, current_selected_row, current_selected_column) &&
		*/

		is_melee_unit(current_selected_row, current_selected_column) &&
		beside(current_input_row, current_input_column, current_selected_row, current_selected_column))
		return 1;
	else 
		return 0;
	
}

bool Battlefield::right_mouse_button_to_attack_remote_is_valid() {	//right mouse button to act
	int max_range = 0, min_range = 0;

	//std::cout << "try" << std::endl;

	if (current_field.unit(current_selected_row, current_selected_column)->type() == "AR") max_range = 2,min_range = 1;

	/*
	//std::cout << range << std::endl;

	//std::cout << current_input_row<<" "<< current_input_column << std::endl;

	//std::cout << inside(current_input_row, current_input_column) << std::endl;
	*/

	if (inside(current_input_row, current_input_column) &&

		/*
		not_ally(current_input_row, current_input_column, current_selected_row, current_selected_column) &&
		*/

		no_mountain(current_selected_row, current_selected_column, current_input_row, current_input_column) &&
		out_min_range(current_input_row, current_input_column, current_selected_row, current_selected_column, min_range) &&
		in_max_range(current_input_row, current_input_column, current_selected_row, current_selected_column, max_range))
		return 1;
	else 
		return 0;
	
}

bool Battlefield::right_mouse_button_to_fireball_is_valid() {
	if (inside(current_input_row, current_input_column) &&
		beside(current_input_row, current_input_column, current_selected_row, current_selected_column))
		return 1;
	else
		return 0;
}

bool Battlefield::right_mouse_button_to_earthquake_is_valid() {
	int max_range = 3;

	if (inside(current_input_row, current_input_column) &&
		in_max_range(current_input_row, current_input_column, current_selected_row, current_selected_column, max_range))
		return 1;
	else
		return 0;
}

void Battlefield::move() {
	int mob_cost = 999;
	char target_terrain = current_field.terrain(current_input_row, current_input_column);
	if (target_terrain == '\0') mob_cost = 1;
	else if (target_terrain == 'F') mob_cost = 2;
	else if (target_terrain == 'W' || target_terrain == 'M' || target_terrain == 'A') mob_cost = 999;
	if (current_field.unit(current_selected_row, current_selected_column)->mob() >= mob_cost) {
		current_field.unit(current_selected_row, current_selected_column)->reduce_mobility(mob_cost);
		current_field.set_units(current_input_row, current_input_column, current_field.unit(current_selected_row, current_selected_column));
		current_field.set_units(current_selected_row, current_selected_column, nullptr);
		set_selected();
	}
}

void Battlefield::attack() {

	//std::cout << "attack" << std::endl;

	for (int i = 0; i < current_units.size(); i++) 
		if (current_field.unit(current_input_row, current_input_column) == &current_units[i]) {
			current_units[i].set_dead();
			break;
		}
	current_field.set_units(current_input_row, current_input_column, nullptr);
	death_effects(current_input_column + 0.5f, current_input_row + 0.5f);
}

void Battlefield::new_turn() {
	char p = '\0';
	current_player = 1 - current_player;
	if (current_player == 1) p = 'A';
	else if (current_player == 0) p = 'B';
	for (int i = 0; i < current_units.size(); i++)
		if (current_units[i].player() == p) current_units[i].reset_mobility();
		else current_units[i].clear_mobility();
	reset_input();
	reset_selected();
	current_state = 0;
}

void Battlefield::print_Battlefield() {
	
	olc::Pixel background(0, 177, 179);
	Clear(background);

	SetPixelMode(olc::Pixel::ALPHA);
	int x = current_field.row(), y = current_field.column();
	for (int i = 1; i <= x + 1; i++) {	//draw rows
		DrawLine(block_size, block_size * i, block_size * (y + 1), block_size * i, olc::WHITE);
	}
	for (int i = 1; i <= y + 1; i++) {	//draw columns
		DrawLine(block_size * i, block_size, block_size * i, block_size * (x + 1), olc::WHITE);
	}
	for (int i = 1; i <= x ; i++) 
		for (int j = 1; j <= y; j++) {	//draw terrains
			std::string sText = "";
			sText = sText + current_field.terrain(i, j);
			//DrawString(j * block_size, i * block_size, sText, olc::WHITE, block_size / 8);
			if (sText == "F")
				DrawSprite(olc::vi2d(j, i) * sprite_size, Forest.get());
			else if (sText == "W")
				DrawSprite(olc::vi2d(j, i) * sprite_size, Water.get());
			else if (sText == "M")
				DrawSprite(olc::vi2d(j, i) * sprite_size, Mountain.get());
			else if (sText == "A")
				DrawSprite(olc::vi2d(j, i) * sprite_size, Abyss.get());
			//else 
				//DrawSprite(olc::vi2d(j, i) * sprite_size, Grass.get());
		}
	if (current_state == 1) {	//draw selection
		olc::Pixel selection(255, 255, 255, 255);
		FillRect(current_selected_column * block_size, current_selected_row * block_size, block_size, block_size, selection);
	}
	for (int i = 1; i <= x; i++)
		for (int j = 1; j <= y; j++) {	//draw units
			if (current_field.unit(i, j) != nullptr) {
				std::string sText = "";
				units* utp = current_field.unit(i, j);
				sText = sText + utp->type();

				/*test
				std::cout<<i <<" " <<j <<" " << utp << std::endl;
				std::cout << utp->type()<<std::endl;
				*/

				//can be optimized by hash instead of if statements

				if (utp->is_alive() && utp->player() == 'A') {
					olc::Pixel green(0, 230, 0, 180);
					FillRect(j * block_size, i * block_size, block_size, block_size, green);
					//DrawString(j * block_size + block_size / 16, i * block_size + block_size / 4, sText, olc::RED, block_size / 16);
					if (sText == "KN")
						DrawSprite(olc::vi2d(j, i) * sprite_size, Knight_A.get());
					else if (sText == "FT")
						DrawSprite(olc::vi2d(j, i) * sprite_size, Footman_A.get());
					else if (sText == "MG")
						DrawSprite(olc::vi2d(j, i) * sprite_size, Mage_A.get());
					else if (sText == "AR")
						DrawSprite(olc::vi2d(j, i) * sprite_size, Archer_A.get());
				}
				else if (utp->is_alive() && utp->player() == 'B') {
					olc::Pixel blue(26, 163, 255, 180);
					FillRect(j * block_size, i * block_size, block_size, block_size, blue);
					//DrawString(j * block_size + block_size / 16, i * block_size + block_size / 4, sText, olc::BLACK, block_size / 16);
					if (sText == "KN")
						DrawSprite(olc::vi2d(j, i) * sprite_size, Knight_B.get());
					else if (sText == "FT")
						DrawSprite(olc::vi2d(j, i) * sprite_size, Footman_B.get());
					else if (sText == "MG")
						DrawSprite(olc::vi2d(j, i) * sprite_size, Mage_B.get());
					else if (sText == "AR")
						DrawSprite(olc::vi2d(j, i) * sprite_size, Archer_B.get());
				}
			}
		}
	SetPixelMode(olc::Pixel::NORMAL);
}

void Battlefield::set_input() {
	current_input_row = int(GetMouseY()) / block_size;
	current_input_column = int(GetMouseX()) / block_size;
}

void Battlefield::reset_input() {
	current_input_row = -1;
	current_input_column = -1;
}

void Battlefield::set_selected() {
	current_selected_row = current_input_row;
	current_selected_column = current_input_column;
}

void Battlefield::reset_selected() {
	current_selected_row = -1;
	current_selected_column = -1;
}

bool Battlefield::beside(int x1, int y1, int x2, int y2) const {
	if (x1 == x2 && y1 == y2) return 0;
	if (abs(x1 - x2) <= 1 && abs(y1 - y2) <= 1) return 1;
	else return 0;
}

bool Battlefield::in_max_range(int x1, int y1, int x2, int y2, int r) const {

	//std::cout << r << std::endl;

	if (x1 == x2 && y1 == y2) return 0;
	if (abs(x1 - x2) <= r && abs(y1 - y2) <= r) return 1;
	else return 0;
}

bool Battlefield::out_min_range(int x1, int y1, int x2, int y2, int r) const {
	if (x1 == x2 && y1 == y2) return 0;
	if (abs(x1 - x2) > r || abs(y1 - y2) > r) return 1;
	else return 0;
}

bool Battlefield::not_ally(int x1, int y1, int x2, int y2) const {
	if (current_field.unit(x1, y1)->player() == current_field.unit(x2, y2)->player()) return 0;
	else return 1;
}

void Battlefield::detect_ended(){
	int player_A_exist = 0, player_B_exist = 0;

	//std::cout << std::endl;

	for (int i = 0; i < current_units.size(); i++) {
		if (current_units[i].is_alive() && current_units[i].player() == 'A') player_A_exist = 1;
		if (current_units[i].is_alive() && current_units[i].player() == 'B') player_B_exist = 1;
	}
	if (player_A_exist && player_B_exist) is_ended = 0;
	else is_ended = 1;

	//std::cout << is_ended << std::endl;

}

void Battlefield::detect_selected() {
	if (current_field.unit(current_selected_row, current_selected_column) == nullptr) {
		reset_input();
		reset_selected();
		current_state = 0;
	}
}

void Battlefield::print_ended() {
	olc::Pixel background(0, 177, 179);
	Clear(background);

	std::string sText = "A Draw!";

	for (int i = 0; i < current_units.size(); i++) 
		if (current_units[i].is_alive()) {
			if(current_units[i].player() == 'A') sText = "Winner is A!";
			else if (current_units[i].player() == 'B') sText = "Winner is B!";
			break;
		}
	
	DrawString(ScreenWidth() / 20, ScreenHeight() / 3, sText, olc::WHITE, block_size / 16);
}

bool Battlefield::no_mountain(int s_x, int s_y, int i_x, int i_y) const {
	int dx = 0, dy = 0;
	dx = (i_x - s_x) / 2;
	dy = (i_y - s_y) / 2;
	if (current_field.terrain(s_x + dx, s_y + dy) == 'M') return 0;
	else return 1;
}

bool Battlefield::is_melee_unit(int x, int y) const{
	if (current_field.unit(x, y)->type() == "KN") return 1;
	if (current_field.unit(x, y)->type() == "FT") return 1;
	return 0;
}

void Battlefield::print_info() {

	//print player state
	std::string sText;

	if (current_player == 1) sText = "Player A's turn";
	else if (current_player == 0) sText = "Player B's turn";

	DrawString(block_size, block_size / 2, sText, olc::WHITE, block_size / 16);

	//print unit state

	//std::cout << current_state << std::endl;

	if (current_state == 1) {
		std::string unit_state;

		unit_state = std::to_string(current_field.unit(current_selected_row,current_selected_column)->mob());

		DrawString(ScreenWidth() - block_size, block_size / 2, unit_state, olc::WHITE, block_size / 16);
	}

}

void Battlefield::print_skill() {
	std::string skill_text = "";
	skill_text = skill_text + current_skill;
	DrawString(ScreenWidth() - block_size, block_size / 2 + block_size * 1, skill_text, olc::WHITE, block_size / 16);
}

void Battlefield::fireball(int x, int y, olc::vi2d v) {
	olc::vi2d position = { x, y };	//the position of fireball

	olc::vf2d direction = { v.y + 0.0f,v.x + 0.0f };
	fireball_effects(y + 0.5f, x + 0.5f, direction);

	while (inside(position.x, position.y) &&
		!current_field.isunit(position.x, position.y) &&
		current_field.terrain(position.x, position.y) != 'F') { //the movement of fireball
		if (current_field.terrain(position.x, position.y) == 'M') return;
		position = position + v;
	}
	if (inside(position.x, position.y) && current_field.isunit(position.x, position.y)) {	//fireball first contact a unit
		current_field.unit(position.x, position.y)->set_dead();
		current_field.set_units(position.x, position.y, nullptr);
		burn_effects(position.y + 0.5f, position.x + 0.5f);
	}
	else if (inside(position.x, position.y) && current_field.terrain(position.x, position.y)) {		//fireball first contact the forest terrain
		burn(position.x, position.y);
	}

}

void Battlefield::burn(int x, int y) {
	if (current_field.terrain(x, y) == 'F') {
		current_field.set_terrains(x, y, '\0');
		burn_effects(y + 0.5f, x + 0.5f);
	}
	else return;
	if (current_field.isunit(x, y)) {
		current_field.unit(x, y)->set_dead();
		current_field.set_units(x, y, nullptr);
		burn_effects(y + 0.5f, x + 0.5f);
	}
	int dx[8] = { 0,0,1,1,1,-1,-1,-1 };
	int dy[8] = { 1,-1,1,-1,0,1,-1,0 };
	for (int i = 0; i < 8; i++) {
		if (inside(x + dx[i], y + dy[i]) &&
			current_field.terrain(x, y) != 'M' &&
			current_field.terrain(x, y) != 'A' &&
			current_field.terrain(x, y) != 'W') {
			burn(x + dx[i], y + dy[i]);
		}
	}
}

void Battlefield::earthquake(int x,int y) {
	int dx[5] = { 0,0,0,1,-1 };
	int dy[5] = { 0,1,-1,0,0 };
	for (int i = 0; i < 5; i++) {
		int xi = x + dx[i], yi = y + dy[i];
		if (inside(xi, yi)) {
			if (current_field.terrain(xi, yi) == 'F' || current_field.terrain(xi, yi) == '\0'){
				current_field.set_terrains(xi, yi, 'A');
				earthquake_effects(yi + 0.5f, xi + 0.5f);
			}
			else if (current_field.terrain(xi, yi) == 'M'){
				current_field.set_terrains(xi, yi, '\0');
				earthquake_effects(yi + 0.5f, xi + 0.5f);
			}
			if (current_field.isunit(xi, yi)) {
				current_field.unit(xi, yi)->set_dead();
				current_field.set_units(xi, yi, nullptr);
				earthquake_effects(yi + 0.5f, xi + 0.5f);
			}
		}
	}
}

void Battlefield::flood(int x, int y, std::vector<std::vector<int>>& v) {
	if (current_field.terrain(x, y) != 'A' && current_field.terrain(x, y) != 'W') return;
	if (current_field.terrain(x, y) == 'A') current_field.set_terrains(x, y, 'W');
	int dx[8] = { 0,0,1,1,1,-1,-1,-1 };
	int dy[8] = { 1,-1,1,-1,0,1,-1,0 };
	for (int i = 0; i < 8; i++) {
		int xi = x + dx[i], yi = y + dy[i];
		if (inside(xi, yi) && !v[xi][yi]) {
			v[xi][yi] = 1;
			flood(xi, yi, v);
		}
	}
}

void Battlefield::flood_map() {
	int r = current_field.row(), c = current_field.column();
	std::vector<std::vector<int>> visited(r + 2);
	for (int i = 0; i < visited.size(); i++)
		visited[i].resize(c + 2);
	for (int i = 0; i < r + 2; i++)
		for (int j = 0; j < c + 2; j++)
			visited[i][j] = 0;
	for (int i = 1; i <= r; i++)
		for (int j = 1; j <= c; j++)
			if(!visited[i][j] && current_field.terrain(i, j) == 'W') {
				visited[i][j] = 1;
				flood(i, j, visited);
			}
}

void Battlefield::death_effects(float x, float y) { //similar to draw
	for (int i = 0; i < 100; i++)
	{
		sFragment f; 
		f.pos = { x , y };
		float fAngle = float(rand()) / float(RAND_MAX) * 2.0f * 3.14159f;
		float fVelocity = float(rand()) / float(RAND_MAX) * 10.0f;
		f.vel = { fVelocity * cos(fAngle), fVelocity * sin(fAngle) };
		f.fAngle = fAngle;
		f.fTime = 3.0f;
		olc::Pixel death(46, 184, 46);
		f.colour = death;
		listFragments.push_back(f);
	}
}

void Battlefield::burn_effects(float x, float y) { //similar to draw
	for (int i = 0; i < 30; i++)
	{
		sFragment f;
		f.pos = { x , y };
		float fAngle = float(rand()) / float(RAND_MAX) * 2.0f * 3.14159f;
		float fVelocity = float(rand()) / float(RAND_MAX) * 30.0f;
		f.vel = { fVelocity * cos(fAngle), fVelocity * sin(fAngle) };
		f.fAngle = fAngle;
		f.fTime = 3.0f;
		olc::Pixel burn(255, 255, 102);
		f.colour = burn;
		listFragments.push_back(f);
	}
}

void Battlefield::fireball_effects(float x, float y, olc::vf2d velocity) {
	for (int i = 0; i < 50; i++)
	{
		sFragment f;
		f.pos = { x , y };
		float fAngle = float(rand()) / float(RAND_MAX) * 2.0f * 3.14159f;
		float fSpeed = float(rand()) / float(RAND_MAX) * 10.0f + 2.0f;
		f.vel = fSpeed * velocity;
		f.fAngle = fAngle;
		f.fTime = 1.0f;
		olc::Pixel fireball(255, 102, 102);
		f.colour = fireball;
		listFragments_fireball.push_back(f);
	}
}

void Battlefield::earthquake_effects(float x, float y) { //similar to draw
	for (int i = 0; i < 30; i++)
	{
		sFragment f;
		f.pos = { x , y };
		float fAngle = float(rand()) / float(RAND_MAX) * 2.0f * 3.14159f;
		float fVelocity = float(rand()) / float(RAND_MAX) * 10.0f;
		f.vel = { fVelocity * cos(fAngle), fVelocity * sin(fAngle) };
		f.fAngle = fAngle;
		f.fTime = 3.0f;
		olc::Pixel earthquake(153, 102, 51);
		f.colour = earthquake;
		listFragments.push_back(f);
	}
}

void Battlefield::update_fragment(float fElapsedTime) {
	for (auto& f : listFragments)
	{
		f.vel += olc::vf2d(0.0f, 20.0f) * fElapsedTime;
		f.pos += f.vel * fElapsedTime;
		f.fAngle += 5.0f * fElapsedTime;
		f.fTime -= fElapsedTime;
		f.colour.a = int((f.fTime / 3.0f) * 255);
	}
	for (auto& f : listFragments_fireball)
	{
		f.pos += f.vel * fElapsedTime;
		f.fAngle += 5.0f * fElapsedTime;
		f.fTime -= fElapsedTime;
		f.colour.a = int((f.fTime / 3.0f) * 255);
	}
}

void Battlefield::remove_dead_fragment() {
	listFragments.erase(
		std::remove_if(listFragments.begin(), listFragments.end(), [](const sFragment& f) { return f.fTime < 0.0f; }),
		listFragments.end()
	);
	listFragments_fireball.erase(
		std::remove_if(listFragments_fireball.begin(), listFragments_fireball.end(), [](const sFragment& f) { return f.fTime < 0.0f; }),
		listFragments_fireball.end()
	);
}

void Battlefield::draw_fragment() {
	//Draw Fragments
	for (auto& f : listFragments)
		DrawRotatedDecal(f.pos * block_size, decFragment.get(), f.fAngle, { 4, 4 }, { 1, 1 }, f.colour);
	for (auto& f : listFragments_fireball)
		DrawRotatedDecal(f.pos * block_size, decFragment.get(), f.fAngle, { 4, 4 }, { 1, 1 }, f.colour);
}