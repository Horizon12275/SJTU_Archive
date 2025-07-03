#include "engine.h"


DataStructure::DataStructure()
{
	sAppName = "DataStructure";
	current_state = 0;
	current_selected_state = 0;
	current_selected_linklistnode = NULL;
	current_selected_duallinklistnode = NULL;
	times = TT;
	counter = 0;
	help = 1;
}

bool DataStructure::OnUserCreate()
{
	//initialization

	srand((unsigned)time(NULL));


	return true;
}

bool DataStructure::OnUserUpdate(float fElapsedTime)
{

	if (GetKey(olc::ESCAPE).bPressed) {
		current_state = MENU, times = TT;
		reset();
	}
		
	if (current_state == MENU) menu();
	else if (current_state == LINKLIST) linklist();
	else if (current_state == DULLINKLIST) duallinklist();
	else if (current_state == TREE) tree();
	else if (current_state == TBD) tbd();

	return true;
}

void DataStructure::menu() {
	//Initialization
	int n = 4;

	olc::vi2d start_position;
	olc::vi2d button_size;
	olc::vi2d text_delta;
	olc::vi2d delta_1(Screen_x / 16, Screen_x / 16);
	olc::vi2d Screen(Screen_x, Screen_y);
	olc::vi2d mouse(GetMousePos());

	button_size = { Screen_x - delta_1.x * 2, (Screen_y - delta_1.y * (n + 1)) / n };

	start_position = delta_1;
	olc::vi2d delta_2(0, button_size.y + delta_1.y);
	text_delta = { button_size.x / 12, button_size.y / 3 };

	std::string text[10] = { "LINKLIST", "DUALLINKLIST", "GRAPH", "TBD"};

	if (times == TT) { //Update background when needed
		Clear(BACKGROUND_COLOR);
		for (int i = 0; i < n; i++) {
			FillRect(start_position + i * delta_2, button_size, BUTTON_COLOR);
			DrawString(start_position + i * delta_2 + text_delta, text[i], TEXT_COLOR, text_delta.y / 8);
		}
		times--;
		if (times == 0) times = TT;
	}

	for (int i = 0; i < n; i++) { //Update the selected button all the time
		olc::vi2d diff = mouse - start_position - i * delta_2;
		if (diff.x > 0 && diff.x < button_size.x && diff.y > 0 && diff.y < button_size.y) {
			DrawRect(start_position + i * delta_2, button_size, BOARD_COLOR);
			if (GetMouse(0).bPressed) current_state = i + 1, times = TT;
		}
		else DrawRect(start_position + i * delta_2, button_size, BUTTON_COLOR);
	}
}

void DataStructure::linklist() {
	//Initialization

	olc::vi2d start_position;
	olc::vi2d text_delta;
	olc::vi2d delta_1(Screen_x / 64, Screen_x / 64);

	start_position = delta_1;
	text_delta = { 0, delta_1.y };

	int mouse_x = GetMouseX();
	int mouse_y = GetMouseY();
	olc::vi2d mouse_pos = { mouse_x, mouse_y };


	std::string text[10] = { "Linklist Simulation"
							,"ESC : Return To Menu"
							, "N : Create A New Node"
							, "D : Delete A Node"
							, "H : Switch Help Window"
							, "LeftButton : Select, Move And Reset"
							, "RightButton : Cancel"};

	if (GetMouse(0).bPressed) TextEntryEnable(false);

	if (current_selected_linklistnode != NULL && current_selected_linklistnode->get_data() == "gg") {
		TextEntryEnable(false);
		current_selected_linklistnode = NULL;
	}

	if (times == TT) { //Update Background when needed
		Clear(BACKGROUND_COLOR);
		for (int i = 0;  i < 10; i++) {
			if (help != 1) break;
			DrawString(start_position + i * text_delta, text[i], TEXT_COLOR, text_delta.y / 8);
		}
		times--;
		if (times == 0) times = TT;
	}

	if (IsTextEntryEnabled()) {
		olc::vi2d tmp = { Screen_x * 9 / 11, delta_1.y };
		DrawString(tmp, "Entering...", TEXT_COLOR, text_delta.y / 8);
	}

	if (GetKey(olc::H).bPressed && !IsTextEntryEnabled()) {
		help = 1 - help;
		times = TT;
	}

	for (auto it = Nodes.begin(); it != Nodes.end(); it++) { //Draw Nodes Always
		//linklistnode test({ 100,100 });
		//test.DrawSelf(this);
		if(it->get_data() != "gg") it->DrawSelf(this);
	}

	for (auto it = Nodes.begin(); it != Nodes.end(); it++) { //Draw Lines and Arrows Always
		if (it->get_next() != NULL && it->get_next_data() != "gg") {
			olc::vi2d d1 = { linklistnode_x * 3 / 4, linklistnode_y / 2 };
			olc::vi2d d2 = { linklistnode_x * 1 / 4, linklistnode_y / 2 };
			DrawLine(it->get_position() + d1, it->next_position() + d2, LINE_COLOR);
			olc::vf2d dline = (it->next_position() + d2) - (it->get_position() + d1);
			olc::vf2d dline_norm = dline.norm();
			float scale = linklistnode_y / 6;
			float theta_1 = 10;
			float theta_2 = -10;
			
			olc::vf2d dtri_1_norm = { dline_norm.x * cos(theta_1) - dline_norm.y * sin(theta_1), dline_norm.x * sin(theta_1) + dline_norm.y * cos(theta_1) };
			olc::vf2d dtri_2_norm = { dline_norm.x * cos(theta_2) - dline_norm.y * sin(theta_2), dline_norm.x * sin(theta_2) + dline_norm.y * cos(theta_2) };
			olc::vf2d dtri_1 = dtri_1_norm * scale;
			olc::vf2d dtri_2 = dtri_2_norm * scale;
			olc::vf2d ptri = it->next_position() + d2;
			FillTriangle(ptri, ptri + dtri_1, ptri + dtri_2, LINE_COLOR);
		}
	}

	//linklistnode test({ 100,100 });
	//test.DrawSelf(this);

	if (GetKey(olc::N).bPressed && !IsTextEntryEnabled()) { //Create a new node
		int xa = delta_1.x * 10;
		int xb = Screen_x - delta_1.x * 10;
		int ya = delta_1.y * 10;
		int yb = Screen_y - delta_1.y * 10;
		olc::vi2d gen_position = { rand() % (xb - xa + 1) + xa, rand() % (yb - ya + 1) + ya};
		linklistnode *new_node = new linklistnode(gen_position, std::to_string(counter));
		//new_node->DrawSelf(this);
		Nodes.push_back(*new_node);
		
		counter++;
		times = TT;
	}



	for (auto it = Nodes.begin(); it != Nodes.end(); it++ ) { //Mouse detect and selected node display
		olc::vi2d dx = { linklistnode_x / 2, 0 };
		if (!it->is_alive()) continue;
		if (it->inside_left(mouse_pos)) { //Left side detect
			DrawRect(it->get_position() + dx, { linklistnode_x / 2, linklistnode_y }, LINKLISTNODE_COLOR_BRIGHT);
			DrawRect(it->get_position(), { linklistnode_x / 2, linklistnode_y }, BOARD_COLOR);
			
			if (GetMouse(0).bPressed && current_selected_state != SELECTED) {
				TextEntryEnable(true, it->get_data());
				//current_selected_state = TEXT_STATE;
				current_selected_linklistnode = &(*it);
				times = TT;
			}

			if (GetKey(olc::D).bPressed && !IsTextEntryEnabled()) { //Try to delete a node
				it->set_dead();
				it->disconnect();
				current_selected_linklistnode = NULL;
				current_selected_state = NO_SELECTED;
				times = TT;
			}
			
			//break;
		}
		else if (it->inside_right(mouse_pos)) { //Right side detect
			DrawRect(it->get_position(), { linklistnode_x / 2, linklistnode_y }, LINKLISTNODE_COLOR_DARK);
			DrawRect(it->get_position() + dx, {linklistnode_x / 2, linklistnode_y}, BOARD_COLOR);
			if (GetMouse(0).bPressed) {
				current_selected_state = SELECTED;
				current_selected_linklistnode = &(*it);
				it->disconnect();
			}
			//break;
		}
		else { //Refresh
			DrawRect(it->get_position() + dx, { linklistnode_x / 2, linklistnode_y }, LINKLISTNODE_COLOR_BRIGHT);
			DrawRect(it->get_position(), { linklistnode_x / 2, linklistnode_y }, LINKLISTNODE_COLOR_DARK);
		}
	}

	if (GetMouse(0).bHeld) { //Move the node
		olc::vi2d d1 = { linklistnode_x / 4, linklistnode_y / 2 };
		for (auto& it : Nodes) {
			if (!it.is_alive()) continue;
			if (it.inside_left(GetMousePos())) {
				it.set_position(GetMousePos() - d1);
				times = TT;
				break;
			}
		}
	}

	if (current_selected_state == SELECTED) { //When SELECTED right side go detect and try to connect
		olc::vi2d dx = { linklistnode_x * 3 / 4, linklistnode_y / 2 };
		olc::vi2d dy = { linklistnode_x * 1 / 4, linklistnode_y / 2 };
		//DrawLine(current_selected_linklistnode->get_position() + dx, current_selected_linklistnode->next_position() + dy, LINE_COLOR, linklistnode_y / 20);
		DrawLine(current_selected_linklistnode->get_position() + dx, GetMousePos(), LINE_COLOR);

		olc::vf2d dline = GetMousePos() - (current_selected_linklistnode->get_position() + dx); //Draw Arrow
		olc::vf2d dline_norm = dline.norm();
		float scale = linklistnode_y / 6;
		float theta_1 = 10;
		float theta_2 = -10;

		olc::vf2d dtri_1_norm = { dline_norm.x * cos(theta_1) - dline_norm.y * sin(theta_1), dline_norm.x * sin(theta_1) + dline_norm.y * cos(theta_1) };
		olc::vf2d dtri_2_norm = { dline_norm.x * cos(theta_2) - dline_norm.y * sin(theta_2), dline_norm.x * sin(theta_2) + dline_norm.y * cos(theta_2) };
		olc::vf2d dtri_1 = dtri_1_norm * scale;
		olc::vf2d dtri_2 = dtri_2_norm * scale;
		olc::vf2d ptri = GetMousePos();
		FillTriangle(ptri, ptri + dtri_1, ptri + dtri_2, LINE_COLOR);

		if (GetMouse(1).bPressed) current_selected_state = NO_SELECTED; //Invalid Left Mouse Button input go cancel select
		if (GetMouse(0).bPressed) { //Detect valid Right Mouse Button input and try to connect
			for (auto& it : Nodes) {
				if (!it.is_alive()) continue;
				if (it.inside_left(GetMousePos()) && !(it == *current_selected_linklistnode)) {
					current_selected_linklistnode->connect(it);
					current_selected_linklistnode = NULL;
					current_selected_state = NO_SELECTED;
					break;
				}
			}
		}
		times = TT;
	}

	if (IsTextEntryEnabled() && current_selected_linklistnode != NULL) {
		current_selected_linklistnode->set_data(TextEntryGetString());
		times = TT;
	}
}

void DataStructure::duallinklist() {
	//Initialization

	olc::vi2d start_position;
	olc::vi2d text_delta;
	olc::vi2d delta_1(Screen_x / 64, Screen_x / 64);

	start_position = delta_1;
	text_delta = { 0, delta_1.y };

	int mouse_x = GetMouseX();
	int mouse_y = GetMouseY();
	olc::vi2d mouse_pos = { mouse_x, mouse_y };

	std::string text[10] = { "Duallinklist Simulation"
							,"ESC : Return To Menu"
							, "N : Create A New Node"
							, "D : Delete A Node"
							, "H : Switch Help Window"
							, "LeftButton : Select, Move And Reset"
							, "RightButton : Cancel" };

	if (current_selected_duallinklistnode != NULL && current_selected_duallinklistnode->get_data() == "gg") {
		TextEntryEnable(false);
		current_selected_duallinklistnode = NULL;
	}

	if (GetMouse(0).bPressed) TextEntryEnable(false);

	if (times == TT) { //Update Background when needed
		Clear(BACKGROUND_COLOR);
		for (int i = 0; i < 10; i++) {
			if (help != 1) break;
			DrawString(start_position + i * text_delta, text[i], TEXT_COLOR, text_delta.y / 8);
		}
		times--;
		if (times == 0) times = TT;
	}

	if (IsTextEntryEnabled()) {
		olc::vi2d tmp = { Screen_x * 9 / 11, delta_1.y };
		DrawString(tmp, "Entering...", TEXT_COLOR, text_delta.y / 8);
	}

	if (GetKey(olc::H).bPressed && !IsTextEntryEnabled()) {
		help = 1 - help;
		times = TT;
	}

	for (auto it = dualNodes.begin(); it != dualNodes.end(); it++) { //Draw Nodes Always
		//duallinklistnode test({ 100,100 });
		//test.DrawSelf(this);
		if (it->get_data() != "gg") it->DrawSelf(this);
	}

	for (auto it = dualNodes.begin(); it != dualNodes.end(); it++) { //Draw Lines and Arrows Always
		if (it->is_alive() && it->get_next() != NULL && it->get_next_data() != "gg") {
			olc::vi2d d1 = { duallinklistnode_x * 7 / 8, duallinklistnode_y * 1 / 3 };
			olc::vi2d d2 = { duallinklistnode_x * 1 / 8, duallinklistnode_y * 1 / 3 };
			DrawLine(it->get_position() + d1, it->next_position() + d2, LINE_COLOR);
			olc::vf2d dline = (it->next_position() + d2) - (it->get_position() + d1);
			olc::vf2d dline_norm = dline.norm();
			float scale = duallinklistnode_y / 6;
			float theta_1 = 10;
			float theta_2 = -10;

			olc::vf2d dtri_1_norm = { dline_norm.x * cos(theta_1) - dline_norm.y * sin(theta_1), dline_norm.x * sin(theta_1) + dline_norm.y * cos(theta_1) };
			olc::vf2d dtri_2_norm = { dline_norm.x * cos(theta_2) - dline_norm.y * sin(theta_2), dline_norm.x * sin(theta_2) + dline_norm.y * cos(theta_2) };
			olc::vf2d dtri_1 = dtri_1_norm * scale;
			olc::vf2d dtri_2 = dtri_2_norm * scale;
			olc::vf2d ptri = it->next_position() + d2;
			FillTriangle(ptri, ptri + dtri_1, ptri + dtri_2, LINE_COLOR);
		}
		if (it->is_alive() && it->get_front() != NULL && it->get_front_data() != "gg") {
			olc::vi2d d1 = { duallinklistnode_x * 1 / 8, duallinklistnode_y * 2 / 3 };
			olc::vi2d d2 = { duallinklistnode_x * 7 / 8, duallinklistnode_y * 2 / 3 };
			DrawLine(it->get_position() + d1, it->front_position() + d2, LINE_COLOR);
			olc::vf2d dline = (it->front_position() + d2) - (it->get_position() + d1);
			olc::vf2d dline_norm = dline.norm();
			float scale = duallinklistnode_y / 6;
			float theta_1 = 10;
			float theta_2 = -10;

			olc::vf2d dtri_1_norm = { dline_norm.x * cos(theta_1) - dline_norm.y * sin(theta_1), dline_norm.x * sin(theta_1) + dline_norm.y * cos(theta_1) };
			olc::vf2d dtri_2_norm = { dline_norm.x * cos(theta_2) - dline_norm.y * sin(theta_2), dline_norm.x * sin(theta_2) + dline_norm.y * cos(theta_2) };
			olc::vf2d dtri_1 = dtri_1_norm * scale;
			olc::vf2d dtri_2 = dtri_2_norm * scale;
			olc::vf2d ptri = it->front_position() + d2;
			FillTriangle(ptri, ptri + dtri_1, ptri + dtri_2, LINE_COLOR);
		}
	}

	//duallinklistnode test({ 100,100 });
	//test.DrawSelf(this);

	if (GetKey(olc::N).bPressed && !IsTextEntryEnabled()) { //Create a new node
		int xa = delta_1.x * 10;
		int xb = Screen_x - delta_1.x * 10;
		int ya = delta_1.y * 10;
		int yb = Screen_y - delta_1.y * 10;
		olc::vi2d gen_position = { rand() % (xb - xa + 1) + xa, rand() % (yb - ya + 1) + ya };
		duallinklistnode* new_node = new duallinklistnode(gen_position, std::to_string(counter));
		//new_node->DrawSelf(this);
		dualNodes.push_back(*new_node);

		counter++;
		times = TT;
	}

	for (auto it = dualNodes.begin(); it != dualNodes.end(); it++) { //Mouse detect and selected node display
		olc::vi2d d1 = { duallinklistnode_x * 1 / 4, 0 };
		olc::vi2d d2 = { duallinklistnode_x * 3 / 4, 0 };
		olc::vi2d size_1 = { duallinklistnode_x / 4, duallinklistnode_y };
		olc::vi2d size_2 = { duallinklistnode_x / 2, duallinklistnode_y };
		if (!it->is_alive()) continue;
		if (it->inside_left(mouse_pos)) { //Left side detect
			DrawRect(it->get_position() + d1, size_2, duallinklistnode_COLOR_DARK);
			DrawRect(it->get_position() + d2, size_1, duallinklistnode_COLOR_BRIGHT);
			DrawRect(it->get_position(), size_1, BOARD_COLOR);

			if (GetMouse(0).bPressed) {
				current_selected_state = SELECTED_LEFT;
				current_selected_duallinklistnode = &(*it);
				it->disconnect_front();
				times = TT;
			}
			//break;
		}
		else if (it->inside_middle(mouse_pos)) { //Select
			DrawRect(it->get_position(), size_1, duallinklistnode_COLOR_BRIGHT);
			DrawRect(it->get_position() + d2, size_1, duallinklistnode_COLOR_BRIGHT);
			DrawRect(it->get_position() + d1, size_2, BOARD_COLOR);

			if (GetMouse(0).bPressed && current_selected_state != SELECTED_LEFT && current_selected_state != SELECTED_RIGHT) {
				TextEntryEnable(true, it->get_data());
				//current_selected_state = TEXT_STATE;
				current_selected_duallinklistnode = &(*it);
				times = TT;
			}

			if (GetKey(olc::D).bPressed && !IsTextEntryEnabled()) { //Try to delete a node
				it->set_dead();
				it->disconnect_next();
				it->disconnect_front();
				current_selected_duallinklistnode = NULL;
				current_selected_state = NO_SELECTED;
				times = TT;
			}
		}
		else if (it->inside_right(mouse_pos)) { //Right side detect
			DrawRect(it->get_position(), size_1, duallinklistnode_COLOR_BRIGHT);
			DrawRect(it->get_position() + d1, size_2, duallinklistnode_COLOR_DARK);
			DrawRect(it->get_position() + d2, size_1, BOARD_COLOR);

			if (GetMouse(0).bPressed) {
				current_selected_state = SELECTED_RIGHT;
				current_selected_duallinklistnode = &(*it);
				it->disconnect_next();
				times = TT;
			}
			//break;
		}
		else { //Refresh
			DrawRect(it->get_position(), size_1, duallinklistnode_COLOR_BRIGHT);
			DrawRect(it->get_position() + d2, size_1, duallinklistnode_COLOR_BRIGHT);
			DrawRect(it->get_position() + d1, size_2, duallinklistnode_COLOR_DARK);
		}
	}

	if (GetMouse(0).bHeld) { //Move the node
		olc::vi2d d1 = { duallinklistnode_x / 2, duallinklistnode_y / 2 };
		for (auto& it : dualNodes) {
			if (!it.is_alive()) continue;
			if (it.inside_middle(GetMousePos())) {
				it.set_position(GetMousePos() - d1);
				times = TT;
				break;
			}
		}
	}

	if (current_selected_state == SELECTED_LEFT) { //When SELECTED right side go detect and try to connect
		olc::vi2d d1 = { duallinklistnode_x * 1 / 8, duallinklistnode_y * 2 / 3 };
		//olc::vi2d d2 = { duallinklistnode_x * 1 / 8, duallinklistnode_y * 2 / 3 };
		//DrawLine(current_selected_duallinklistnode->get_position() + dx, current_selected_duallinklistnode->next_position() + dy, LINE_COLOR, duallinklistnode_y / 20);
		DrawLine(current_selected_duallinklistnode->get_position() + d1, GetMousePos(), LINE_COLOR);

		olc::vf2d dline = GetMousePos() - (current_selected_duallinklistnode->get_position() + d1); //Draw Arrow
		olc::vf2d dline_norm = dline.norm();
		float scale = duallinklistnode_y / 6;
		float theta_1 = 10;
		float theta_2 = -10;

		olc::vf2d dtri_1_norm = { dline_norm.x * cos(theta_1) - dline_norm.y * sin(theta_1), dline_norm.x * sin(theta_1) + dline_norm.y * cos(theta_1) };
		olc::vf2d dtri_2_norm = { dline_norm.x * cos(theta_2) - dline_norm.y * sin(theta_2), dline_norm.x * sin(theta_2) + dline_norm.y * cos(theta_2) };
		olc::vf2d dtri_1 = dtri_1_norm * scale;
		olc::vf2d dtri_2 = dtri_2_norm * scale;
		olc::vf2d ptri = GetMousePos();
		FillTriangle(ptri, ptri + dtri_1, ptri + dtri_2, LINE_COLOR);

		if (GetMouse(1).bPressed) current_selected_state = NO_SELECTED; //Invalid Left Mouse Button input go cancel select
		if (GetMouse(0).bPressed) { //Detect valid Right Mouse Button input and try to connect
			for (auto& it : dualNodes) {
				if (!it.is_alive()) continue;
				if (it.inside_middle(GetMousePos()) && !(it == *current_selected_duallinklistnode)) {
					current_selected_duallinklistnode->connect_front(it);
					current_selected_duallinklistnode = NULL;
					current_selected_state = NO_SELECTED;
					break;
				}
			}
		}
		times = TT;
	}

	if (current_selected_state == SELECTED_RIGHT) { //When SELECTED right side go detect and try to connect
		olc::vi2d d1 = { duallinklistnode_x * 7 / 8, duallinklistnode_y * 1 / 3 };
		//olc::vi2d d2 = { duallinklistnode_x * 1 / 8, duallinklistnode_y * 2 / 3 };
		//DrawLine(current_selected_duallinklistnode->get_position() + dx, current_selected_duallinklistnode->front_position() + dy, LINE_COLOR, duallinklistnode_y / 20);
		DrawLine(current_selected_duallinklistnode->get_position() + d1, GetMousePos(), LINE_COLOR);

		olc::vf2d dline = GetMousePos() - (current_selected_duallinklistnode->get_position() + d1); //Draw Arrow
		olc::vf2d dline_norm = dline.norm();
		float scale = duallinklistnode_y / 6;
		float theta_1 = 10;
		float theta_2 = -10;

		olc::vf2d dtri_1_norm = { dline_norm.x * cos(theta_1) - dline_norm.y * sin(theta_1), dline_norm.x * sin(theta_1) + dline_norm.y * cos(theta_1) };
		olc::vf2d dtri_2_norm = { dline_norm.x * cos(theta_2) - dline_norm.y * sin(theta_2), dline_norm.x * sin(theta_2) + dline_norm.y * cos(theta_2) };
		olc::vf2d dtri_1 = dtri_1_norm * scale;
		olc::vf2d dtri_2 = dtri_2_norm * scale;
		olc::vf2d ptri = GetMousePos();
		FillTriangle(ptri, ptri + dtri_1, ptri + dtri_2, LINE_COLOR);

		if (GetMouse(1).bPressed) current_selected_state = NO_SELECTED; //Invalid Left Mouse Button input go cancel select
		if (GetMouse(0).bPressed) { //Detect valid Right Mouse Button input and try to connect
			for (auto& it : dualNodes) {
				if (!it.is_alive()) continue;
				if (it.inside_middle(GetMousePos()) && !(it == *current_selected_duallinklistnode)) {
					current_selected_duallinklistnode->connect_next(it);
					current_selected_duallinklistnode = NULL;
					current_selected_state = NO_SELECTED;
					break;
				}
			}
		}
		times = TT;
	}

	if (IsTextEntryEnabled() && current_selected_duallinklistnode != NULL) {
		current_selected_duallinklistnode->set_data(TextEntryGetString());
		times = TT;
	}
}

void DataStructure::tree() {
	//Initialization

	olc::vi2d start_position;
	olc::vi2d text_delta;
	olc::vi2d delta_1(Screen_x / 64, Screen_x / 64);

	start_position = delta_1;
	text_delta = { 0, delta_1.y };

	int mouse_x = GetMouseX();
	int mouse_y = GetMouseY();
	olc::vi2d mouse_pos = { mouse_x, mouse_y };


	std::string text[10] = { "Graph Simulation"
							,"ESC : Return To Menu"
							, "N : Create A New Node"
							, "D : Delete A Node"
							, "C : Connect A New Child"
							, "Z : Disconnect A Child"
							, "H : Switch Help Window"
							, "LeftButton : Move And Connect"
							, "RightButton : Start Input" };

	if (GetMouse(0).bPressed) TextEntryEnable(false);

	if (current_selected_treenode != NULL && current_selected_treenode->get_data() == "gg") {
		TextEntryEnable(false);
		current_selected_treenode = NULL;
	}

	if (times == TT) { //Update Background when needed
		Clear(BACKGROUND_COLOR);
		for (int i = 0; i < 10; i++) {
			if (help != 1) break;
			DrawString(start_position + i * text_delta, text[i], TEXT_COLOR, text_delta.y / 8);
		}
		times--;
		if (times == 0) times = TT;
	}

	if (IsTextEntryEnabled()) {
		olc::vi2d tmp = { Screen_x * 9 / 11, delta_1.y };
		DrawString(tmp, "Entering...", TEXT_COLOR, text_delta.y / 8);
	}

	if (GetKey(olc::H).bPressed && !IsTextEntryEnabled()) { //Control help window
		help = 1 - help;
		times = TT;
	}

	for (auto it = treeNodes.begin(); it != treeNodes.end(); it++) { //Draw treeNodes Always
		//treenode test({ 100,100 });
		//test.DrawSelf(this);
		if (it->get_data() != "gg") it->DrawSelf(this);
	}

	if (current_selected_treenode != NULL) {
		DrawCircle(current_selected_treenode->get_position(), treenode_r, BOARD_COLOR);
	}
	
	for (auto it = treeNodes.begin(); it != treeNodes.end(); it++) { //Draw Lines and Arrows Always
		if (it->get_next().empty()) continue;
		std::list<treenode*> tmp = it->get_next();
		for (auto pt = tmp.begin(); pt != tmp.end(); pt++) {
			if (!it->get_next().empty() && (*pt)->get_data() != "gg") {
				olc::vf2d dline = (*pt)->get_position() - it->get_position();
				olc::vf2d dline_norm = dline.norm();
				float scale = treenode_r / 3;
				float theta_1 = 10;
				float theta_2 = -10;
				DrawLine(it->get_position() + dline_norm * treenode_r, (*pt)->get_position() - dline_norm * treenode_r, LINE_COLOR);
				olc::vf2d dtri_1_norm = { dline_norm.x * cos(theta_1) - dline_norm.y * sin(theta_1), dline_norm.x * sin(theta_1) + dline_norm.y * cos(theta_1) };
				olc::vf2d dtri_2_norm = { dline_norm.x * cos(theta_2) - dline_norm.y * sin(theta_2), dline_norm.x * sin(theta_2) + dline_norm.y * cos(theta_2) };
				olc::vf2d dtri_1 = dtri_1_norm * scale;
				olc::vf2d dtri_2 = dtri_2_norm * scale;
				olc::vf2d ptri = (*pt)->get_position() - dline_norm * treenode_r;
				FillTriangle(ptri, ptri + dtri_1, ptri + dtri_2, LINE_COLOR);
			}
		}
	}
	

	//treenode test({ 100,100 });
	//test.DrawSelf(this);

	if (GetKey(olc::N).bPressed && !IsTextEntryEnabled()) { //Create a new node
		int xa = delta_1.x * 10;
		int xb = Screen_x - delta_1.x * 10;
		int ya = delta_1.y * 10;
		int yb = Screen_y - delta_1.y * 10;
		olc::vi2d gen_position = { rand() % (xb - xa + 1) + xa, rand() % (yb - ya + 1) + ya };
		treenode* new_node = new treenode(gen_position, std::to_string(counter));
		//new_node->DrawSelf(this);
		treeNodes.push_back(*new_node);

		counter++;
		times = TT;
	}

	for (auto it = treeNodes.begin(); it != treeNodes.end(); it++) { //Mouse detect and selected node display
		//olc::vi2d dx = { treenode_x / 2, 0 };
		if (!it->is_alive()) continue;
		if (it->inside(mouse_pos)) { //Left side detect
			DrawCircle(it->get_position(), treenode_r, BOARD_COLOR);

			if (GetKey(olc::C).bPressed && !IsTextEntryEnabled()) {
				current_selected_state = SELECTED;
				current_selected_treenode = &(*it);
				times = TT;
			}

			if(GetMouse(1).bPressed) {
				TextEntryEnable(true, it->get_data());
				current_selected_treenode = &(*it);
				current_selected_state = NO_SELECTED;
				times = TT;
			}

			if (GetKey(olc::D).bPressed && !IsTextEntryEnabled()) { //Try to delete a node
				current_selected_treenode = NULL;
				current_selected_state = NO_SELECTED;
				it->set_dead();
				times = TT;
			}

			if (GetKey(olc::Z).bPressed && !IsTextEntryEnabled()) {
				it->disconnect();
				times = TT;
			}
			//break;
		}
		else { //Refresh
			DrawCircle(it->get_position(), treenode_r, treenode_COLOR_DARK);
		}
	}

	if (GetMouse(0).bHeld && current_selected_state != SELECTED) { //Move the node
		for (auto& it : treeNodes) {
			if (!it.is_alive()) continue;
			if (it.inside(GetMousePos())) {
				it.set_position(GetMousePos());
				times = TT;
				break;
			}
		}
	}

	if (current_selected_state == SELECTED) { //When SELECTED go detect and try to connect
		//olc::vi2d dx = { treenode_x * 3 / 4, treenode_y / 2 };
		//olc::vi2d dy = { treenode_x * 1 / 4, treenode_y / 2 };
		//DrawLine(current_selected_treenode->get_position() + dx, current_selected_treenode->next_position() + dy, LINE_COLOR, treenode_y / 20);
		DrawLine(current_selected_treenode->get_position(), GetMousePos(), LINE_COLOR);

		olc::vf2d dline = GetMousePos() - current_selected_treenode->get_position(); //Draw Arrow
		olc::vf2d dline_norm = dline.norm();
		float scale = treenode_r / 3;
		float theta_1 = 10;
		float theta_2 = -10;

		olc::vf2d dtri_1_norm = { dline_norm.x * cos(theta_1) - dline_norm.y * sin(theta_1), dline_norm.x * sin(theta_1) + dline_norm.y * cos(theta_1) };
		olc::vf2d dtri_2_norm = { dline_norm.x * cos(theta_2) - dline_norm.y * sin(theta_2), dline_norm.x * sin(theta_2) + dline_norm.y * cos(theta_2) };
		olc::vf2d dtri_1 = dtri_1_norm * scale;
		olc::vf2d dtri_2 = dtri_2_norm * scale;
		olc::vf2d ptri = GetMousePos();
		FillTriangle(ptri, ptri + dtri_1, ptri + dtri_2, LINE_COLOR);

		if (GetMouse(2).bPressed) {
			current_selected_state = NO_SELECTED;
			current_selected_treenode = NULL;
			TextEntryEnable(false);
		}		//Invalid Left Mouse Button input go cancel select
		if (GetMouse(0).bPressed) { //Detect valid Left Mouse Button input and try to connect
			for (auto& it : treeNodes) {
				if (!it.is_alive()) continue;
				if (it.inside(GetMousePos()) && !(it == *current_selected_treenode)) {
					current_selected_treenode->connect(it);
					current_selected_treenode = NULL;
					current_selected_state = NO_SELECTED;
					TextEntryEnable(false);
					times = TT;
					break;
				}
			}
		}
		times = TT;
	}

	if (IsTextEntryEnabled() && current_selected_treenode != NULL) {
		current_selected_treenode->set_data(TextEntryGetString());
		times = TT;
	}
}

void DataStructure::tbd() {
	if (times == TT) {
		Clear(BACKGROUND_COLOR);
		DrawString({ Screen_x / 10 , Screen_y / 3 }, "TBD", TEXT_COLOR, 5);
		times--;
		if (times == 0) times = TT;
	}
}

void DataStructure::reset() {
	Nodes.clear();
	dualNodes.clear();
	treeNodes.clear();
	current_state = 0;
	current_selected_state = 0;
	current_selected_linklistnode = NULL;
	current_selected_duallinklistnode = NULL;
	current_selected_treenode = NULL;
	times = TT;
	counter = 0;
	help = 1;
	TextEntryEnable(false);
}