#pragma once

void AssertEqual(game_screen_position P1, game_screen_position P2) {
	Assert(P1.X == P2.X && P1.Y == P2.Y && P1.Z == P2.Z);
}

void AssertEqual(v3 P1, v3 P2) {
	Assert(P1.X == P2.X && P1.Y == P2.Y && P1.Z == P2.Z);
}

void AssertEqual(tile_position P1, tile_position P2) {
	Assert(P1.Col == P2.Col && P1.Row == P2.Row);
}

void Tests() {
	// Coordinates
	camera Camera = { 0 };
	Camera.Height = 100;
	Camera.Width = 100;
	Camera.Position = { 0 };
	Camera.Velocity = { 0 };
	Camera.Zoom = 1;
	
	v3 WorldCoord = { 0 };
	game_screen_position ScreenCoord = { Camera.Width / 2, Camera.Height / 2, 0 };
	tile_position TilePosition = { 0, 0 };
	AssertEqual(ToScreenCoord(WorldCoord, Camera), ScreenCoord);
	AssertEqual(ToWorldCoord(ScreenCoord, Camera), WorldCoord);
	AssertEqual(ToTilePosition(ScreenCoord, Camera), TilePosition);
	AssertEqual(ToTilePosition(WorldCoord), TilePosition);

	Camera.Position = { 1000.0, 1000.0, 0.0 };
	AssertEqual(ToScreenCoord(Camera.Position, Camera), ScreenCoord);

	TilePosition = {10, 10};
	AssertEqual(ToWorldCoord(TilePosition), {10*TILESIZE, 10*TILESIZE});
	AssertEqual(ToScreenCoord(TilePosition, Camera), ToScreenCoord(ToWorldCoord(TilePosition), Camera));
}
