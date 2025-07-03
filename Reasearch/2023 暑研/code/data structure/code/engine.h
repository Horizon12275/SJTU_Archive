#pragma once
#define Screen_x 1680
#define Screen_y 1050
#define TT 5000

#include "linklistnode.h"
#include "duallinklistnode.h"
#include "treenode.h"

#define OLC_PGEX_QUICKGUI
#include "extensions/olcPGEX_QuickGUI.h"


class DataStructure : public olc::PixelGameEngine {

private:
	int current_state, times, counter, current_selected_state, help;
	enum state{MENU = 0, LINKLIST, DULLINKLIST, TREE, TBD};
	enum select_state{NO_SELECTED = 0, SELECTED, NEW_LEFT_STATE, NEW_RIGHT_STATE, TEXT_STATE, SELECTED_LEFT, SELECTED_RIGHT};
	linklistnode* current_selected_linklistnode;
	std::list<linklistnode> Nodes;
	
	duallinklistnode* current_selected_duallinklistnode;
	std::list<duallinklistnode> dualNodes;

	treenode* current_selected_treenode;
	std::list<treenode> treeNodes;

public:

	DataStructure();
	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;
	void menu();
	void linklist();
	void duallinklist();
	void tree();
	void tbd();
	void reset();
};




