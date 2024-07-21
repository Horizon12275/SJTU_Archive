#include"extensions/olcPGEX_QuickGUI.h"

namespace olc::QuickGUI
{

#pragma region BaseControl
	BaseControl::BaseControl(olc::QuickGUI::Manager& manager) : m_manager(manager)
	{
		m_manager.AddControl(this);
	}

	BaseControl::~BaseControl()
	{

	}

	void BaseControl::Enable(const bool bEnable)
	{
		m_state = bEnable ? State::Normal : State::Disabled;
	}
#pragma endregion

#pragma region Manager
	Manager::Manager(const bool bCleanUpForMe)
	{
		m_bEraseControlsOnDestroy = bCleanUpForMe;
	}

	Manager::~Manager()
	{
		if (m_bEraseControlsOnDestroy)
			for (auto& p : m_vControls)
				delete p;

		m_vControls.clear();
	}

	void Manager::AddControl(BaseControl* control)
	{
		m_vControls.push_back(control);
	}

	void Manager::Update(olc::PixelGameEngine* pge)
	{
		for (auto& p : m_vControls) p->Update(pge);
	}

	void Manager::Draw(olc::PixelGameEngine* pge)
	{
		for (auto& p : m_vControls) p->Draw(pge);
	}

	void Manager::DrawDecal(olc::PixelGameEngine* pge)
	{
		for (auto& p : m_vControls) p->DrawDecal(pge);
	}

	void Manager::CopyThemeFrom(const Manager& manager)
	{
		this->colBorder = manager.colBorder;
		this->colClick = manager.colClick;
		this->colDisable = manager.colDisable;
		this->colHover = manager.colHover;
		this->colNormal = manager.colNormal;
		this->colText = manager.colText;
		this->fGrabRad = manager.fGrabRad;
		this->fHoverSpeedOff = manager.fHoverSpeedOff;
		this->fHoverSpeedOn = manager.fHoverSpeedOn;
	}
#pragma endregion

#pragma region Label
	Label::Label(olc::QuickGUI::Manager& manager, const std::string& text, const olc::vf2d& pos, const olc::vf2d& size)
		: BaseControl(manager)
	{
		vPos = pos; vSize = size; sText = text;
	}

	void Label::Update(olc::PixelGameEngine* pge)
	{

	}

	void Label::Draw(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		if (bHasBackground)
		{
			pge->FillRect(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colNormal);
		}

		if (bHasBorder)
			pge->DrawRect(vPos, vSize - olc::vf2d(1, 1), m_manager.colBorder);

		olc::vf2d vText = pge->GetTextSizeProp(sText);
		switch (nAlign)
		{
		case Alignment::Left:
			pge->DrawStringProp(olc::vf2d(vPos.x + 2.0f, vPos.y + (vSize.y - vText.y) * 0.5f), sText, m_manager.colText);
			break;
		case Alignment::Centre:
			pge->DrawStringProp(vPos + (vSize - vText) * 0.5f, sText, m_manager.colText);
			break;
		case Alignment::Right:
			pge->DrawStringProp(olc::vf2d{ vPos.x + vSize.x - vText.x - 2.0f, vPos.y + (vSize.y - vText.y) * 0.5f }, sText, m_manager.colText);
			break;
		}
	}

	void Label::DrawDecal(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		if (bHasBackground)
		{
			pge->FillRectDecal(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colNormal);
		}

		if (bHasBorder)
		{
			pge->SetDecalMode(olc::DecalMode::WIREFRAME);
			pge->FillRectDecal(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colBorder);
			pge->SetDecalMode(olc::DecalMode::NORMAL);
		}

		olc::vf2d vText = pge->GetTextSizeProp(sText);
		switch (nAlign)
		{
		case Alignment::Left:
			pge->DrawStringPropDecal({ vPos.x + 2.0f, vPos.y + (vSize.y - vText.y) * 0.5f }, sText, m_manager.colText);
			break;
		case Alignment::Centre:
			pge->DrawStringPropDecal(vPos + (vSize - vText) * 0.5f, sText, m_manager.colText);
			break;
		case Alignment::Right:
			pge->DrawStringPropDecal({ vPos.x + vSize.x - vText.x - 2.0f, vPos.y + (vSize.y - vText.y) * 0.5f }, sText, m_manager.colText);
			break;
		}
	}
#pragma endregion


#pragma region TextBox
	TextBox::TextBox(olc::QuickGUI::Manager& manager, const std::string& text, const olc::vf2d& pos, const olc::vf2d& size)
		: Label(manager, text, pos, size)
	{
		nAlign = Alignment::Left;
		bHasBorder = true;
		bHasBackground = false;
	}

	void TextBox::Update(olc::PixelGameEngine* pge)
	{
		if (m_state == State::Disabled || !bVisible)
			return;

		bPressed = false;
		bReleased = false;

		olc::vf2d vMouse = pge->GetMousePos();

		if (vMouse.x >= vPos.x && vMouse.x < vPos.x + vSize.x &&
			vMouse.y >= vPos.y && vMouse.y < vPos.y + vSize.y)
		{
			// Released inside box does nothing to me, but i may have
			// to finish off the neighbours... oo err
			bPressed = pge->GetMouse(olc::Mouse::LEFT).bPressed;
			bReleased = pge->GetMouse(olc::Mouse::LEFT).bReleased;

			if (bPressed && pge->IsTextEntryEnabled() && !m_bTextEdit)
			{
				pge->TextEntryEnable(false);
			}


			if (bPressed && !pge->IsTextEntryEnabled() && !m_bTextEdit)
			{
				pge->TextEntryEnable(true, sText);
				m_bTextEdit = true;
			}

			bHeld = pge->GetMouse(olc::Mouse::LEFT).bHeld;


		}
		else
		{
			// Released outside box
			bPressed = pge->GetMouse(olc::Mouse::LEFT).bPressed;
			bReleased = pge->GetMouse(olc::Mouse::LEFT).bReleased;
			bHeld = pge->GetMouse(olc::Mouse::LEFT).bHeld;

			if (bPressed && m_bTextEdit)
			{
				sText = pge->TextEntryGetString();
				pge->TextEntryEnable(false);
				m_bTextEdit = false;
			}
		}

		if (m_bTextEdit && pge->IsTextEntryEnabled())
			sText = pge->TextEntryGetString();
	}

	void TextBox::Draw(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		if (bHasBackground)
		{
			pge->FillRect(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colNormal);
		}

		if (bHasBorder)
			pge->DrawRect(vPos, vSize - olc::vf2d(1, 1), m_manager.colBorder);

		if (m_bTextEdit && pge->IsTextEntryEnabled())
		{
			// Draw Cursor
			int32_t i = pge->TextEntryGetCursor();
			olc::vf2d vCursorPos = pge->GetTextSizeProp(sText.substr(0, i));
			pge->FillRect(olc::vf2d(vPos.x + 2.0f + vCursorPos.x, (vPos.y + (vSize.y - 10.0f) * 0.5f)), { 2, 10 }, m_manager.colText);
		}

		// Draw Text
		olc::vf2d vText = pge->GetTextSizeProp(sText);
		pge->DrawStringProp(olc::vf2d(vPos.x + 2.0f, vPos.y + (vSize.y - vText.y) * 0.5f), sText, m_manager.colText);

	}

	void TextBox::DrawDecal(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		if (bHasBackground)
		{
			pge->FillRectDecal(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colNormal);
		}

		if (bHasBorder)
		{
			pge->SetDecalMode(olc::DecalMode::WIREFRAME);
			pge->FillRectDecal(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colBorder);
			pge->SetDecalMode(olc::DecalMode::NORMAL);
		}

		if (m_bTextEdit && pge->IsTextEntryEnabled())
		{
			// Draw Cursor
			int32_t i = pge->TextEntryGetCursor();
			olc::vf2d vCursorPos = pge->GetTextSizeProp(sText.substr(0, i));
			pge->FillRectDecal(olc::vf2d(vPos.x + 2.0f + vCursorPos.x, (vPos.y + (vSize.y - 10.0f) * 0.5f)), { 2, 10 }, m_manager.colText);
		}

		// Draw Text
		olc::vf2d vText = pge->GetTextSizeProp(sText);
		pge->DrawStringPropDecal(olc::vf2d(vPos.x + 2.0f, vPos.y + (vSize.y - vText.y) * 0.5f), sText, m_manager.colText);
	}
#pragma endregion

#pragma region Button
	Button::Button(olc::QuickGUI::Manager& manager, const std::string& text, const olc::vf2d& pos, const olc::vf2d& size)
		: BaseControl(manager)
	{
		vPos = pos; vSize = size; sText = text;
	}

	void Button::Update(olc::PixelGameEngine* pge)
	{
		if (m_state == State::Disabled || !bVisible)
			return;

		bPressed = false;
		bReleased = false;
		float fElapsedTime = pge->GetElapsedTime();

		olc::vf2d vMouse = pge->GetMousePos();
		if (m_state != State::Click)
		{
			if (vMouse.x >= vPos.x && vMouse.x < vPos.x + vSize.x &&
				vMouse.y >= vPos.y && vMouse.y < vPos.y + vSize.y)
			{
				m_fTransition += fElapsedTime * m_manager.fHoverSpeedOn;
				m_state = State::Hover;

				bPressed = pge->GetMouse(olc::Mouse::LEFT).bPressed;
				if (bPressed)
				{
					m_state = State::Click;
				}

				bHeld = pge->GetMouse(olc::Mouse::LEFT).bHeld;
			}
			else
			{
				m_fTransition -= fElapsedTime * m_manager.fHoverSpeedOff;
				m_state = State::Normal;
			}
		}
		else
		{
			bHeld = pge->GetMouse(olc::Mouse::LEFT).bHeld;
			bReleased = pge->GetMouse(olc::Mouse::LEFT).bReleased;
			if (bReleased) m_state = State::Normal;
		}

		m_fTransition = std::clamp(m_fTransition, 0.0f, 1.0f);
	}

	void Button::Draw(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		switch (m_state)
		{
		case State::Disabled:
			pge->FillRect(vPos, vSize, m_manager.colDisable);
			break;
		case State::Normal:
		case State::Hover:
			pge->FillRect(vPos, vSize, olc::PixelLerp(m_manager.colNormal, m_manager.colHover, m_fTransition));
			break;
		case State::Click:
			pge->FillRect(vPos, vSize, m_manager.colClick);
			break;
		}

		pge->DrawRect(vPos, vSize - olc::vf2d(1, 1), m_manager.colBorder);
		olc::vf2d vText = pge->GetTextSizeProp(sText);
		pge->DrawStringProp(vPos + (vSize - vText) * 0.5f, sText, m_manager.colText);
	}

	void Button::DrawDecal(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		switch (m_state)
		{
		case State::Disabled:
			pge->FillRectDecal(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colDisable);
			break;
		case State::Normal:
		case State::Hover:
			pge->FillRectDecal(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), olc::PixelLerp(m_manager.colNormal, m_manager.colHover, m_fTransition));
			break;
		case State::Click:
			pge->FillRectDecal(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colClick);
			break;
		}
		pge->SetDecalMode(olc::DecalMode::WIREFRAME);
		pge->FillRectDecal(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colBorder);
		pge->SetDecalMode(olc::DecalMode::NORMAL);

		olc::vf2d vText = pge->GetTextSizeProp(sText);
		pge->DrawStringPropDecal(vPos + (vSize - vText) * 0.5f, sText, m_manager.colText);
	}
#pragma endregion


#pragma region ImageButton
	ImageButton::ImageButton(olc::QuickGUI::Manager& manager, const olc::Renderable& icon, const olc::vf2d& pos, const olc::vf2d& size)
		: Button(manager, "", pos, size), pIcon(icon)
	{

	}

	void ImageButton::Draw(olc::PixelGameEngine* pge)
	{
		Button::Draw(pge);
		pge->DrawSprite(vPos + olc::vi2d(4, 4), pIcon.Sprite());
	}

	void ImageButton::DrawDecal(olc::PixelGameEngine* pge)
	{
		Button::DrawDecal(pge);
		pge->DrawDecal(vPos + olc::vi2d(4, 4), pIcon.Decal());
	}
#pragma endregion


#pragma region ImageCheckBox
	ImageCheckBox::ImageCheckBox(olc::QuickGUI::Manager& manager, const olc::Renderable& gfx, const bool check, const olc::vf2d& pos, const olc::vf2d& size)
		: ImageButton(manager, gfx, pos, size)
	{
		bChecked = check;
	}

	void ImageCheckBox::Update(olc::PixelGameEngine* pge)
	{
		if (m_state == State::Disabled || !bVisible)
			return;

		ImageButton::Update(pge);
		if (bPressed) bChecked = !bChecked;
	}

	void ImageCheckBox::Draw(olc::PixelGameEngine* pge)
	{
		ImageButton::Draw(pge);

		if (bChecked)
			pge->DrawRect(vPos + olc::vf2d(2, 2), vSize - olc::vi2d(5, 5), m_manager.colBorder);
	}

	void ImageCheckBox::DrawDecal(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		ImageButton::DrawDecal(pge);

		pge->SetDecalMode(olc::DecalMode::WIREFRAME);
		pge->FillRectDecal(vPos + olc::vf2d(2, 2), vSize - olc::vf2d(4, 4), m_manager.colBorder);
		pge->SetDecalMode(olc::DecalMode::NORMAL);

		olc::vf2d vText = pge->GetTextSizeProp(sText);
		pge->DrawStringPropDecal(vPos + (vSize - vText) * 0.5f, sText, m_manager.colText);
	}
#pragma endregion


#pragma region CheckBox
	CheckBox::CheckBox(olc::QuickGUI::Manager& manager, const std::string& text, const bool check, const olc::vf2d& pos, const olc::vf2d& size)
		: Button(manager, text, pos, size)
	{
		bChecked = check;
	}

	void CheckBox::Update(olc::PixelGameEngine* pge)
	{
		if (m_state == State::Disabled || !bVisible)
			return;

		Button::Update(pge);
		if (bPressed)
			bChecked = !bChecked;
	}

	void CheckBox::Draw(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		Button::Draw(pge);

		if (bChecked)
			pge->DrawRect(vPos + olc::vf2d(2, 2), vSize - olc::vi2d(5, 5), m_manager.colBorder);
	}

	void CheckBox::DrawDecal(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		Button::DrawDecal(pge);

		if (bChecked)
		{
			pge->SetDecalMode(olc::DecalMode::WIREFRAME);
			pge->FillRectDecal(vPos + olc::vf2d(2, 2), vSize - olc::vf2d(4, 4), m_manager.colBorder);
			pge->SetDecalMode(olc::DecalMode::NORMAL);
		}

		olc::vf2d vText = pge->GetTextSizeProp(sText);
		pge->DrawStringPropDecal(vPos + (vSize - vText) * 0.5f, sText, m_manager.colText);
	}
#pragma endregion

#pragma region Slider
	Slider::Slider(olc::QuickGUI::Manager& manager, const olc::vf2d& posmin, const olc::vf2d& posmax, const float valmin, const float valmax, const float value)
		: BaseControl(manager)
	{
		vPosMin = posmin; vPosMax = posmax; fMin = valmin; fMax = valmax; fValue = value;
	}

	void Slider::Update(olc::PixelGameEngine* pge)
	{
		if (m_state == State::Disabled || !bVisible)
			return;

		float fElapsedTime = pge->GetElapsedTime();

		olc::vf2d vMouse = pge->GetMousePos();
		bHeld = false;
		if (m_state == State::Click)
		{
			olc::vf2d d = vPosMax - vPosMin;
			float u = d.dot(vMouse - vPosMin) / d.mag2();
			fValue = u * (fMax - fMin) + fMin;
			bHeld = true;
		}
		else
		{
			olc::vf2d vSliderPos = vPosMin + (vPosMax - vPosMin) * ((fValue - fMin) / (fMax - fMin));
			if ((vMouse - vSliderPos).mag2() <= int32_t(m_manager.fGrabRad) * int32_t(m_manager.fGrabRad))
			{
				m_fTransition += fElapsedTime * m_manager.fHoverSpeedOn;
				m_state = State::Hover;
				if (pge->GetMouse(olc::Mouse::LEFT).bPressed)
				{
					m_state = State::Click;
					bPressed = true;
				}
			}
			else
				m_state = State::Normal;
		}

		if (pge->GetMouse(olc::Mouse::LEFT).bReleased)
		{
			m_state = State::Normal;
			bReleased = true;
		}

		if (m_state == State::Normal)
		{
			m_fTransition -= fElapsedTime * m_manager.fHoverSpeedOff;
			m_state = State::Normal;
			bHeld = false;
		}

		fValue = std::clamp(fValue, fMin, fMax);
		m_fTransition = std::clamp(m_fTransition, 0.0f, 1.0f);
	}

	void Slider::Draw(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		pge->DrawLine(vPosMin, vPosMax, m_manager.colBorder);
		olc::vf2d vSliderPos = vPosMin + (vPosMax - vPosMin) * ((fValue - fMin) / (fMax - fMin));

		switch (m_state)
		{
		case State::Disabled:
			pge->FillCircle(vSliderPos, int32_t(m_manager.fGrabRad), m_manager.colDisable);
			break;
		case State::Normal:
		case State::Hover:
			pge->FillCircle(vSliderPos, int32_t(m_manager.fGrabRad), olc::PixelLerp(m_manager.colNormal, m_manager.colHover, m_fTransition));
			break;
		case State::Click:
			pge->FillCircle(vSliderPos, int32_t(m_manager.fGrabRad), m_manager.colClick);
			break;
		}


		pge->DrawCircle(vSliderPos, int32_t(m_manager.fGrabRad), m_manager.colBorder);
	}

	void Slider::DrawDecal(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		pge->DrawLineDecal(vPosMin, vPosMax, m_manager.colBorder);
		olc::vf2d vSliderPos = vPosMin + (vPosMax - vPosMin) * ((fValue - fMin) / (fMax - fMin));

		switch (m_state)
		{
		case State::Disabled:
			pge->FillRectDecal(vSliderPos - olc::vf2d(m_manager.fGrabRad, m_manager.fGrabRad), olc::vf2d(m_manager.fGrabRad, m_manager.fGrabRad) * 2.0f, m_manager.colDisable);
			break;
		case State::Normal:
		case State::Hover:
			pge->FillRectDecal(vSliderPos - olc::vf2d(m_manager.fGrabRad, m_manager.fGrabRad), olc::vf2d(m_manager.fGrabRad, m_manager.fGrabRad) * 2.0f, olc::PixelLerp(m_manager.colNormal, m_manager.colHover, m_fTransition));
			break;
		case State::Click:
			pge->FillRectDecal(vSliderPos - olc::vf2d(m_manager.fGrabRad, m_manager.fGrabRad), olc::vf2d(m_manager.fGrabRad, m_manager.fGrabRad) * 2.0f, m_manager.colClick);
			break;
		}

		pge->SetDecalMode(olc::DecalMode::WIREFRAME);
		pge->FillRectDecal(vSliderPos - olc::vf2d(m_manager.fGrabRad, m_manager.fGrabRad), olc::vf2d(m_manager.fGrabRad, m_manager.fGrabRad) * 2.0f, m_manager.colBorder);
		pge->SetDecalMode(olc::DecalMode::NORMAL);
	}


#pragma endregion

#pragma region ListBox
	ListBox::ListBox(olc::QuickGUI::Manager& manager, std::vector<std::string>& vList, const olc::vf2d& pos, const olc::vf2d& size)
		: BaseControl(manager), m_vList(vList)
	{
		m_group.CopyThemeFrom(m_manager);
		vPos = pos;
		vSize = size;
		m_pSlider = new Slider(m_group, { pos.x + size.x - m_manager.fGrabRad - 1, pos.y + m_manager.fGrabRad + 1 },
			{ pos.x + size.x - m_manager.fGrabRad - 1, pos.y + size.y - m_manager.fGrabRad - 1 }, 0, float(m_vList.size()), 0);
	}

	void ListBox::Update(olc::PixelGameEngine* pge)
	{
		if (m_state == State::Disabled || !bVisible)
			return;


		nPreviouslySelectedItem = nSelectedItem;
		olc::vf2d vMouse = pge->GetMousePos() - vPos + olc::vi2d(2, 0);
		if (pge->GetMouse(olc::Mouse::LEFT).bPressed)
		{
			if (vMouse.x >= 0 && vMouse.x < vSize.x - (m_group.fGrabRad * 2) && vMouse.y >= 0 && vMouse.y < vSize.y)
			{

				nSelectedItem = size_t(m_pSlider->fValue + vMouse.y / 10);
			}
		}

		nSelectedItem = std::clamp(nSelectedItem, size_t(0), m_vList.size() - 1);

		bSelectionChanged = nSelectedItem != nPreviouslySelectedItem;


		m_pSlider->fMax = float(m_vList.size());
		m_group.Update(pge);
	}

	void ListBox::Draw(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		if (bHasBackground)
		{
			pge->FillRect(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colNormal);
		}

		if (bHasBorder)
			pge->DrawRect(vPos, vSize - olc::vf2d(1, 1), m_manager.colBorder);


		size_t idx0 = size_t(m_pSlider->fValue);
		size_t idx1 = std::min(idx0 + size_t((vSize.y - 4) / 10), m_vList.size());

		olc::vf2d vTextPos = vPos + olc::vf2d(2, 2);
		for (size_t idx = idx0; idx < idx1; idx++)
		{
			if (idx == nSelectedItem)
				pge->FillRect(vTextPos - olc::vi2d(1, 1), { int32_t(vSize.x - m_group.fGrabRad * 2), 10 }, m_group.colHover);
			pge->DrawStringProp(vTextPos, m_vList[idx]);
			vTextPos.y += 10;
		}

		m_group.Draw(pge);
	}

	void ListBox::DrawDecal(olc::PixelGameEngine* pge)
	{
		if (!bVisible)
			return;

		if (bHasBackground)
			pge->FillRectDecal(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colNormal);

		size_t idx0 = size_t(m_pSlider->fValue);
		size_t idx1 = std::min(idx0 + size_t((vSize.y - 4) / 10), m_vList.size());

		olc::vf2d vTextPos = vPos + olc::vf2d(2, 2);
		for (size_t idx = idx0; idx < idx1; idx++)
		{
			if (idx == nSelectedItem)
				pge->FillRectDecal(vTextPos - olc::vi2d(1, 1), { vSize.x - m_group.fGrabRad * 2.0f, 10.0f }, m_group.colHover);
			pge->DrawStringPropDecal(vTextPos, m_vList[idx]);
			vTextPos.y += 10;
		}

		if (bHasBorder)
		{
			pge->SetDecalMode(olc::DecalMode::WIREFRAME);
			pge->FillRectDecal(vPos + olc::vf2d(1, 1), vSize - olc::vf2d(2, 2), m_manager.colBorder);
			pge->SetDecalMode(olc::DecalMode::NORMAL);
		}

		m_group.DrawDecal(pge);
	}
#pragma endregion



#pragma region Modal
	ModalDialog::ModalDialog() : olc::PGEX(true)
	{

		// Create File Open Dialog
		olc::vi2d vScreenSize = pge->GetScreenSize();

		m_listDirectory = new ListBox(m_manFileSelect, m_vDirectory, olc::vf2d(20, 20), olc::vf2d(300, 500));
		m_listFiles = new ListBox(m_manFileSelect, m_vFiles, olc::vf2d(330, 20), olc::vf2d(300, 500));

		m_path = "/";
		for (auto const& dir_entry : std::filesystem::directory_iterator{ m_path })
		{
			if (dir_entry.is_directory())
				m_vDirectory.push_back(dir_entry.path().filename().string());
			else
				m_vFiles.push_back(dir_entry.path().filename().string());
		}
	}

	void ModalDialog::ShowFileOpen(const std::string& sPath)
	{
		m_bShowDialog = true;
	}

	bool ModalDialog::OnBeforeUserUpdate(float& fElapsedTime)
	{
		if (!m_bShowDialog) return false;

		m_manFileSelect.Update(this->pge);

		if (pge->GetKey(olc::Key::BACK).bPressed)
		{
			m_path = m_path.parent_path().string() + "/";
			//m_listDirectory->bSelectionChanged = true;
			//m_listDirectory->nSelectedItem = 0;
		}

		if (m_listDirectory->bSelectionChanged)
		{
			std::string sDirectory = m_vDirectory[m_listDirectory->nSelectedItem];
			/*if (sDirectory == "..")
				m_path = m_path.parent_path().string() + "/";
			else
				m_path += sDirectory+ "/";*/


			m_path += sDirectory + "/";
			// Reconstruct Lists
			m_vDirectory.clear();

			m_vFiles.clear();


			for (auto const& dir_entry : std::filesystem::directory_iterator{ m_path })
			{
				if (dir_entry.is_directory() || dir_entry.is_other())
					m_vDirectory.push_back(dir_entry.path().filename().string());
				else
					m_vFiles.push_back(dir_entry.path().filename().string());
			}

			//m_vDirectory.push_back("..");

			//m_listFiles->nSelectedItem = 0;
			//m_listDirectory->nSelectedItem = 0;

		}

		pge->DrawStringDecal({ 0,0 }, m_path.string());




		m_manFileSelect.DrawDecal(this->pge);



		if (pge->GetKey(olc::Key::ESCAPE).bPressed)
		{
			m_bShowDialog = false;
			return false;
		}

		return true;
	}
#pragma endregion

}
