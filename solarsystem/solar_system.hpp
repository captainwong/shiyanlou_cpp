#pragma once

#include <GL/glut.h>
#include "star.hpp"

#define STAR_NUM 10

class solar_system {
public:
	solar_system();
	~solar_system();

	void on_display();
	void on_update();
	void on_keyboard(unsigned char key, int x, int y);

private:
	star* stars_[STAR_NUM] = {};
	GLdouble view_x_ = {}, view_y_ = {}, view_z_ = {};
	GLdouble center_x_ = {}, center_y_ = {}, center_z_ = {};
	GLdouble up_x_ = {}, up_y_ = {}, up_z_ = {};

};

