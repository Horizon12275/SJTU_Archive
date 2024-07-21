#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_TRANSFORMEDVIEW
#include "extensions/olcPGEX_TransformedView.h"

#include "utilities/olcUTIL_Camera2D.h"

#define Screen_x 1024
#define Screen_y 1024
#define Camera_x 64
#define Camera_y 64
#define car_x 128
#define car_y 128

/*
	To demonstrate the camera, we need a world. In this case its a simle tile
	world of 80x75 tiles, and each tile is 32x32 screen pixels.

	A transformed view is used to navigate the world manually via the middle
	mouse button in "free roam" mode, or controlled by the camera. 

	Specifically a Tile Transformed View is used, which means all units for
	drawing and for the camera are specified in tile space, i.e. 1 tile is 
	1x1 units (regardless of pixel size)

	No assets are used for this application, so the world is constructed
	out of coloured squares so you can see you are moving through it.

	Pressing "TAB" key will swap between "free roam" and "play" modes. In
	free roam mode, you can use middle mouse button to pan and zoom around 
	the world. The camera's visible area to the player is highlighted in red.
	In play mode, the camera behaves as it would in a 2D game, depending upon 
	the selected mode.
*/

class myCamera2D : public olc::PixelGameEngine
{
public:
	myCamera2D()
	{
		sAppName = "Camera";
	}

	// Transformed view object to make world offsetting simple
	olc::TileTransformedView tv;

	// Conveninet constants to define tile map world
	olc::vi2d m_vWorldSize = { Screen_x, Screen_y };
	olc::vi2d m_vTileSize = { Camera_x, Camera_y };

	// The camera!
	olc::utils::Camera2D camera;

	// The point that represents the player, it is "tracked"
	// by the camera
	olc::vf2d vTrackedPoint;

	// Flag whether we are in "free roam" or "play" mode
	bool bFreeRoam = false;

	// The world map, stored as a 1D array
	std::vector<uint8_t> vWorldMap;

	std::unique_ptr<olc::Sprite> car_sprite;
	std::unique_ptr<olc::Decal> car_decal;

	olc::vf2d vVel = { 0.0f, 0.0f };

	float last_car_angle = 0.0f;

public:
	bool OnUserCreate() override
	{
		// Construct transform view
		tv = olc::TileTransformedView(GetScreenSize(), m_vTileSize);
		
		// Construct Camera
		vTrackedPoint = { 16.0f, 64.0f };		
		camera = olc::utils::Camera2D(GetScreenSize() / m_vTileSize, vTrackedPoint);

		// Configure Camera
		camera.SetTarget(vTrackedPoint);
		camera.SetMode(olc::utils::Camera2D::Mode::Simple);
		camera.SetWorldBoundary({ 0.0f, 0.0f }, m_vWorldSize);
		//camera.EnableWorldBoundary(true);

		// Create "tile map" world with just two tiles
		vWorldMap.resize(m_vWorldSize.x * m_vWorldSize.y);
		for (auto x = 0; x < Screen_x; x++) {
			for (auto y = 0; y < Screen_y; y++) {
				if (y >= 0 && y <= 6) vWorldMap[y * Screen_x + x] = 1;
				if (x >= 0 && x <= 6) vWorldMap[y * Screen_x + x] = 1;
				if (y >= Screen_y - 6 && y <= Screen_y) vWorldMap[y * Screen_x + x] = 1;
				if (x >= Screen_x - 6 && x <= Screen_x) vWorldMap[y * Screen_x + x] = 1;

				if (y >= 18 && y <= 24) vWorldMap[y * Screen_x + x] = 1;
				if (x >= 18 && x <= 24) vWorldMap[y * Screen_x + x] = 1;
				if (y >= Screen_y - 24 && y <= Screen_y - 18) vWorldMap[y * Screen_x + x] = 1;
				if (x >= Screen_x - 24 && x <= Screen_x - 18) vWorldMap[y * Screen_x + x] = 1;

				if (x == 12 && y % 4 != 0 && y % 4 != 1) vWorldMap[y * Screen_x + x] = 2;
				if (y == 12 && x % 4 != 0 && x % 4 != 1) vWorldMap[y * Screen_x + x] = 2;

			}
		}

		// Set background colour
		Clear(olc::CYAN);

		car_sprite = std::make_unique<olc::Sprite>("./resource/car1.png");
		car_decal = std::make_unique<olc::Decal>(car_sprite.get());

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// In free roam, middle mouse button pans & zooms
		if (bFreeRoam)
			tv.HandlePanAndZoom();

		// Handle player "physics" in response to key presses
		
		if (GetKey(olc::Key::W).bHeld) vVel += {0, -0.15};
		if (GetKey(olc::Key::S).bHeld) vVel += {0, +0.15};
		if (GetKey(olc::Key::A).bHeld) vVel += {-0.15, 0};
		if (GetKey(olc::Key::D).bHeld) vVel += {+0.15, 0};

		olc::vf2d fraction_accelerate;
		if (vVel.mag() != 0)
			fraction_accelerate = -vVel * 0.015 - vVel.norm() * 0.001;
		else
			fraction_accelerate = { 0, 0 };

		if (vVel.mag() < 0.02) vVel = { 0, 0 };

		vVel += fraction_accelerate;

		
		
		olc::vf2d x_norm = { 1 , 0 };
		float car_angle;

		if (vVel.mag() < 0.02) car_angle = last_car_angle;
		else {
			car_angle = std::max(acos(vVel.dot(x_norm) / vVel.mag()), 0.0f);
			last_car_angle = car_angle;
		}

		if (vVel.y < 0) car_angle = -car_angle;

		// Calculate where ball should be, if no collision
		olc::vf2d vPotentialcarPos = vTrackedPoint + vVel * 8.0f * fElapsedTime;

		// Test for hits 4 points around ball
		olc::vf2d vTilecarRadialDims = { 1.2, 1.2 };

		auto TestResolveCollisionPoint = [&](const olc::vf2d& point)
		{
			olc::vi2d vTestPoint = vPotentialcarPos + vTilecarRadialDims * point;
			auto& tile = vWorldMap[vTestPoint.y * Screen_x + vTestPoint.x];
			if (tile == 0 || tile == 2)
			{
				// Do Nothing, no collision
				return false;
			}
			else
			{
				// Ball has collided with a tile
				bool bTileHit = (tile == 1);
				//if (bTileHit) tile--;

				// Collision response
				if (point.x == 0.0f)
					vVel.y *= -0.0f;
				if (point.y == 0.0f)
					vVel.x *= -0.0f;
				return bTileHit;
			}
		};

		bool bHasHitTile = false;
		bHasHitTile |= TestResolveCollisionPoint(olc::vf2d(0, -1));
		bHasHitTile |= TestResolveCollisionPoint(olc::vf2d(0, +1));
		bHasHitTile |= TestResolveCollisionPoint(olc::vf2d(-1, 0));
		bHasHitTile |= TestResolveCollisionPoint(olc::vf2d(+1, 0));
		bHasHitTile |= TestResolveCollisionPoint(olc::vf2d(-0.7, -0.7));
		bHasHitTile |= TestResolveCollisionPoint(olc::vf2d(+0.7, -0.7));
		bHasHitTile |= TestResolveCollisionPoint(olc::vf2d(+0.7, +0.7));
		bHasHitTile |= TestResolveCollisionPoint(olc::vf2d(-0.7, +0.7));


		vTrackedPoint += vVel * 8.0f * fElapsedTime;

		// Switch between "free roam" and "play" mode with TAB key
		if (GetKey(olc::Key::TAB).bPressed)
		{
			// Always setup camera to play mode when tab key pressed
			tv.SetWorldOffset(camera.GetViewPosition());
			tv.SetWorldScale(m_vTileSize);
			bFreeRoam = !bFreeRoam;
		}
		
		// Switch camera mode in operation
		if (GetKey(olc::Key::K1).bPressed) 
			camera.SetMode(olc::utils::Camera2D::Mode::Simple);
		//if (GetKey(olc::Key::K2).bPressed) 
			//camera.SetMode(olc::utils::Camera2D::Mode::EdgeMove);
		if (GetKey(olc::Key::K2).bPressed) 
			camera.SetMode(olc::utils::Camera2D::Mode::LazyFollow);
		//if (GetKey(olc::Key::K4).bPressed) 
			//camera.SetMode(olc::utils::Camera2D::Mode::FixedScreens);

		// Update the camera, if teh tracked object remains visible, 
		// true is returned
		bool bOnScreen = camera.Update(fElapsedTime);

		// In "play" mode, set the transformed view to that required by
		// the camera
		if (!bFreeRoam)
			tv.SetWorldOffset(camera.GetViewPosition());

		// Render "tile map", by getting visible tiles
		olc::vi2d vTileTL = tv.GetTopLeftTile().max({ 0,0 });
		olc::vi2d vTileBR = tv.GetBottomRightTile().min(m_vWorldSize);
		olc::vi2d vTile;
		// Then looping through them and drawing them
		for (vTile.y = vTileTL.y; vTile.y < vTileBR.y; vTile.y++)
			for (vTile.x = vTileTL.x; vTile.x < vTileBR.x; vTile.x++)
			{
				int idx = vTile.y * m_vWorldSize.x + vTile.x;
				
				if (vWorldMap[idx] == 0)
					tv.FillRectDecal(vTile, { 1.0f, 1.0f }, olc::Pixel(40, 40, 40));
				
				if (vWorldMap[idx] == 1)
					tv.FillRectDecal(vTile, { 1.0f, 1.0f }, olc::Pixel(60, 60, 60));			

				if (vWorldMap[idx] == 2)
					tv.FillRectDecal(vTile, { 1.0f, 1.0f }, olc::WHITE);
			}

		// Draw the "player" as a 1x1 cell
		//tv.FillRectDecal(vTrackedPoint - olc::vf2d(0.5f, 0.5f), {1.0f, 1.0f}, olc::BLUE);

		

		// Overlay with information
		if (bFreeRoam)
		{
			tv.FillRectDecal(camera.GetViewPosition(), camera.GetViewSize(), olc::PixelF(1.0f, 0.0f, 0.0f, 0.5f));
			DrawStringPropDecal({ 2, 2 }, "TAB: Free Mode, M-Btn to Pan & Zoom", olc::YELLOW);
		}
		else {
			DrawStringPropDecal({ 2,2 }, "TAB: Play Mode", olc::YELLOW);
			DrawRotatedDecal({ Screen_x / 2 ,Screen_y / 2 }, car_decal.get(), car_angle + 3.14 / 2, { car_x / 2, car_y / 2 });
		}
			

		DrawStringPropDecal({ 2,12 }, "WASD  : Move", olc::YELLOW);
		DrawStringPropDecal({ 2,22 }, "CAMERA: 1) Simple  2) LazyFollow", olc::YELLOW);		
		DrawStringPropDecal({ 2,42 }, vTrackedPoint.str(), olc::YELLOW);

		//DrawSprite(vTrackedPoint, car_sprite.get());
		

		return true;
	}

};

int main()
{
	myCamera2D demo;
	if (demo.Construct(Screen_x, Screen_y, 1, 1, false, true))
		demo.Start();
	return 0;
}