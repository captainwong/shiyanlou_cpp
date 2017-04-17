#pragma once

#include <GL/glut.h>

class star {
public:
	GLfloat radius_ = {};
	GLfloat speed_ = {}, self_speed_ = {};
	GLfloat distance_ = {};
	GLfloat rgba_color_[4] = {};
	star* parent_star_ = nullptr;

	star(GLfloat radius, GLfloat distance, GLfloat speed, GLfloat self_speed, star* parent);

	virtual ~star();

	void draw_star();

	virtual void draw() { draw_star(); }

	virtual void update(long time_span);

protected:
	GLfloat alpha_self_ = {}, alpha_ = {};

};


class planet : public star {
public:
	planet(GLfloat radius, GLfloat distance, GLfloat speed, GLfloat self_speed,
		   star* parent, GLfloat rgba_color[3]);

	void draw_planet();

	virtual void draw() override { draw_planet(); draw_star(); }

};



class light_planet : public planet {
public:
	light_planet(GLfloat radius, GLfloat distance, GLfloat speed,
				 GLfloat self_speed, star* parent, GLfloat rgba_color[3]);

	void draw_light();
	virtual void draw() override { draw_light(); draw_planet(); draw_star(); }




};


