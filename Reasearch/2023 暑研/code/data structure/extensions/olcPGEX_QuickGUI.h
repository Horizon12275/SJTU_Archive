#pragma once
#include "olcPixelGameEngine.h"

namespace olc::QuickGUI
{
	class Manager;

	// Virtual base class for all controls
	class BaseControl
	{
	public:
		BaseControl(olc::QuickGUI::Manager& manager);
		virtual ~BaseControl();

	public:
		// Switches the control on/off
		void Enable(const bool bEnable);
		// Sets whether or not the control is interactive/displayed
		bool bVisible = true;

		// True on single frame control begins being manipulated
		bool bPressed = false;
		// True on all frames control is under user manipulation
		bool bHeld = false;
		// True on single frame control ceases being manipulated
		bool bReleased = false;

	public:
		// Updates the controls behvaiour
		virtual void Update(olc::PixelGameEngine* pge) = 0;
		// Draws the control using "sprite" based CPU operations
		virtual void Draw(olc::PixelGameEngine* pge) = 0;
		// Draws the control using "decal" based GPU operations
		virtual void DrawDecal(olc::PixelGameEngine* pge) = 0;

	protected:
		// Controls are related to a manager, where the theme resides
		// and control groups can be implemented
		olc::QuickGUI::Manager& m_manager;

		// All controls exists in one of four states
		// Disabled - Greyed out and not interactive
		// Normal - interactive and operational
		// Hover - currently under the users mouse focus
		// Click - user is interacting with the control
		enum class State { Disabled, Normal, Hover, Click } m_state = State::Normal;
		
		// To add a "swish" to things, controls can fade between states
		float m_fTransition = 0.0;
	};


	// A QuickGUI::Manager acts as a convenient grouping of controls
	class Manager
	{
	public:
		// Construct Manager, bCleanUpForMe will automatically DELETE any controls
		// given to this manager via AddControl() if true
		Manager(const bool bCleanUpForMe = true);
		virtual ~Manager();
	
	public:
		// Add a gui element derived form BaseControl to this manager
		void AddControl(BaseControl* control);
		// Updates all controls this manager operates
		void Update(olc::PixelGameEngine* pge);
		// Draws as "sprite" all controls this manager operates
		void Draw(olc::PixelGameEngine* pge);
		// Draws as "decal" all controls this manager operates
		void DrawDecal(olc::PixelGameEngine* pge);
		
	public: // This managers "Theme" can be set here
		// Various element colours
		olc::Pixel colNormal = olc::DARK_BLUE;
		olc::Pixel colHover = olc::BLUE;
		olc::Pixel colClick = olc::CYAN;
		olc::Pixel colDisable = olc::DARK_GREY;
		olc::Pixel colBorder = olc::WHITE;
		olc::Pixel colText = olc::WHITE;
		// Speed to transiton from Normal -> Hover
		float fHoverSpeedOn = 10.0f;
		// Speed to transiton from Hover -> Normal
		float fHoverSpeedOff = 4.0f;
		// Size of grab handle
		float fGrabRad = 8.0f;
		// Copy all theme attributes into a different manager object
		void CopyThemeFrom(const Manager& manager);

	private:
		// Should this manager call delete on the controls it opeerates?
		bool m_bEraseControlsOnDestroy = true;
		// Container of controls
		std::vector<BaseControl*> m_vControls;
	};


	// Creates a Label Control - it's just text!
	class Label : public BaseControl
	{
	public:
		Label(olc::QuickGUI::Manager& manager,  // Associate with a Manager
			const std::string& text,			// Text to display
			const olc::vf2d& pos,				// Location of label top-left
			const olc::vf2d& size);				// Size of label

	public:
		// Position of button
		olc::vf2d vPos;
		// Size of button
		olc::vf2d vSize;
		// Text displayed on button
		std::string sText;
		// Show a border?
		bool bHasBorder = false;
		// Show a background?
		bool bHasBackground = false;
		// Where should the text be positioned?
		enum class Alignment
		{Left, Centre, Right} nAlign = Alignment::Centre;

	public: // BaseControl overrides
		void Update(olc::PixelGameEngine* pge) override;
		void Draw(olc::PixelGameEngine* pge) override;
		void DrawDecal(olc::PixelGameEngine* pge) override;
	};

	class TextBox : public Label
	{
	public:
		TextBox(olc::QuickGUI::Manager& manager,	// Associate with a Manager
			const std::string& text,				// Text to display
			const olc::vf2d& pos,					// Location of text box top-left
			const olc::vf2d& size);					// Size of text box

	public: // BaseControl overrides
		void Update(olc::PixelGameEngine* pge) override;
		void Draw(olc::PixelGameEngine* pge) override;
		void DrawDecal(olc::PixelGameEngine* pge) override;

	protected:
		bool m_bTextEdit = false;

	};

	// Creates a Button Control - a clickable, labelled rectangle
	class Button : public BaseControl
	{
	public:
		Button(olc::QuickGUI::Manager& manager, // Associate with a Manager
			const std::string& text,			// Text to display
			const olc::vf2d& pos,				// Location of button top-left
			const olc::vf2d& size);				// Size of button
				
	public:
		// Position of button
		olc::vf2d vPos;	
		// Size of button
		olc::vf2d vSize;
		// Text displayed on button
		std::string sText;

	public: // BaseControl overrides
		void Update(olc::PixelGameEngine* pge) override;
		void Draw(olc::PixelGameEngine* pge) override;
		void DrawDecal(olc::PixelGameEngine* pge) override;
	};

	// Creates a Button Control - a clickable, labelled rectangle
	class CheckBox : public Button
	{
	public:
		CheckBox(olc::QuickGUI::Manager& manager,	// Associate with a Manager
			const std::string& text,				// Text to display
			const bool check,						// Is checked or not?
			const olc::vf2d& pos,					// Location of button top-left
			const olc::vf2d& size);					// Size of button

	public:
		bool bChecked = false;

	public: // BaseControl overrides
		void Update(olc::PixelGameEngine* pge) override;
		void Draw(olc::PixelGameEngine* pge) override;
		void DrawDecal(olc::PixelGameEngine* pge) override;
	};

	class ImageButton : public Button
	{
	public:
		ImageButton(olc::QuickGUI::Manager& manager,	// Associate with a Manager
			const olc::Renderable &icon,			// Text to display			
			const olc::vf2d& pos,					// Location of button top-left
			const olc::vf2d& size);					// Size of button

	public:
		const olc::Renderable& pIcon;

	public:
		void Draw(olc::PixelGameEngine* pge) override;
		void DrawDecal(olc::PixelGameEngine* pge) override;
	};

	class ImageCheckBox : public ImageButton
	{
	public:
		ImageCheckBox(olc::QuickGUI::Manager& manager,	// Associate with a Manager
			const olc::Renderable& icon,				// Text to display
			const bool check,						// Is checked or not?
			const olc::vf2d& pos,					// Location of button top-left
			const olc::vf2d& size);					// Size of button

	public:
		bool bChecked = false;

	public:
		void Update(olc::PixelGameEngine* pge) override;
		void Draw(olc::PixelGameEngine* pge) override;
		void DrawDecal(olc::PixelGameEngine* pge) override;
	};


	// Creates a Slider Control - a grabbable handle that slides between two locations 
	class Slider : public BaseControl
	{
	public:
		Slider(olc::QuickGUI::Manager& manager, // Associate with a Manager
			const olc::vf2d& posmin,			// Screen location of "minimum"
			const olc::vf2d& posmax,			// Screen location of "maximum"
			const float valmin,					// Value of minimum
			const float valmax,					// Value of maximum
			const float value);					// Starting value

	public:
		// Minium value
		float fMin = -100.0f;
		// Maximum value
		float fMax = +100.0f;
		// Current value
		float fValue = 0.0f;

		// Location of minimum/start
		olc::vf2d vPosMin;
		// Location of maximum/end
		olc::vf2d vPosMax;

	public: // BaseControl overrides
		void Update(olc::PixelGameEngine* pge) override;
		void Draw(olc::PixelGameEngine* pge) override;
		void DrawDecal(olc::PixelGameEngine* pge) override;
	};


	class ListBox : public BaseControl
	{
	public:
		ListBox(olc::QuickGUI::Manager& manager,	// Associate with a Manager
			std::vector<std::string>& vList,			
			const olc::vf2d& pos,					// Location of list top-left
			const olc::vf2d& size);					// Size of list

		// Position of list
		olc::vf2d vPos;
		// Size of list
		olc::vf2d vSize;
		// Show a border?
		bool bHasBorder = true;
		// Show a background?
		bool bHasBackground = true;
		
	public:
		Slider *m_pSlider = nullptr;
		Manager m_group;
		size_t  m_nVisibleItems = 0;
		std::vector<std::string>& m_vList;

	public:
		// Item currently selected
		size_t nSelectedItem = 0;
		size_t nPreviouslySelectedItem = 0;
		// Has selection changed?
		bool bSelectionChanged = false;

	public: // BaseControl overrides
		void Update(olc::PixelGameEngine* pge) override;
		void Draw(olc::PixelGameEngine* pge) override;
		void DrawDecal(olc::PixelGameEngine* pge) override;
	};


	class ModalDialog : public olc::PGEX
	{
	public:
		ModalDialog();

	public:
		void ShowFileOpen(const std::string& sPath);

	protected:
		virtual bool OnBeforeUserUpdate(float& fElapsedTime) override;

	private:
		bool m_bShowDialog = false;

		Manager m_manFileSelect;
		ListBox* m_listVolumes = nullptr;
		ListBox* m_listDirectory = nullptr;
		ListBox* m_listFiles = nullptr;

		std::vector<std::string> m_vVolumes;
		std::vector<std::string> m_vDirectory;
		std::vector<std::string> m_vFiles;
		std::filesystem::path m_path;
	};
}