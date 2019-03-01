#pragma once
class m_rs
{
public:
	int width_stereo_module = 1280;
	int height_stereo_module = 800;
	int fps_stereo_module = 15;

	int width_rgb_camera = 1280;
	int height_rgb_camera = 720;
	int fps_rgb_camera = 15;
	m_rs();
	~m_rs();

	static int sum(int x, int y);
};

