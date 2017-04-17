#include "star.hpp"
#include <cmath>

using namespace std;

#define PI 3.1415926535

star::star(GLfloat radius, GLfloat distance,
		   GLfloat speed, GLfloat self_speed, star* parent)
	:radius_(radius), distance_(distance), self_speed_(self_speed), parent_star_(parent)
{
	for (int i = 0; i < 4; i++) {
		rgba_color_[i] = 1.0f;
	}

	if (speed > 0) {
		speed_ = 360.0f / speed;
	} else {
		speed_ = 0.0f;
	}


}

star::~star() {
	
}

void star::draw_star() {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);

	int n = 1440;

	glPushMatrix();

	{
		// 
		if (parent_star_ && parent_star_->distance_ > 0) {
			glRotatef(parent_star_->alpha_, 0, 0, 1);
			glTranslatef(parent_star_->distance_, 0, 0);
		}

		glBegin(GL_LINES);
		for (int i = 0; i < n; i++) {
			glVertex2f(distance_ * cos(2 * PI * i / n),
					   distance_ * sin(2 * PI * i / n));
		}
		glEnd();

		glRotatef(alpha_, 0, 0, 1);
		glTranslatef(distance_, 0, 0);
		glRotatef(alpha_self_, 0, 0, 1);
		glColor3f(rgba_color_[0], rgba_color_[1], rgba_color_[2]);
		glutSolidSphere(radius_, 40, 32);
	}

	glPopMatrix();
}

void star::update(long time_span) {
	alpha_ += time_span * speed_;
	alpha_self_ += self_speed_;
}

planet::planet(GLfloat radius, GLfloat distance,
			   GLfloat speed, GLfloat self_speed,
			   star* parent, GLfloat rgba_color[3])
	: star(radius, distance, speed, self_speed, parent) 
{

	for (int i = 0; i < 3; i++) {
		rgba_color_[i] = rgba_color[i];
	}

	rgba_color_[3] = 1.0f;
}

void planet::draw_planet() {
	GLfloat mat_ambient[] = { 0.0f, 0.0f, 0.5f, 1.0f };
	GLfloat mat_diffuse[] = { 0.0f, 0.0f, 0.5f, 1.0f };
	GLfloat mat_specular[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	GLfloat mat_emission[] = { rgba_color_[0], rgba_color_[1], rgba_color_[2], rgba_color_[3] };
	GLfloat mat_shininess = 90.0f;

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
}

light_planet::light_planet(GLfloat radius, GLfloat distance,
						   GLfloat speed, GLfloat self_speed,
						   star* parent, GLfloat rgba_color[3])
	: planet(radius, distance, speed, self_speed, parent, rgba_color)
{

}

void light_planet::draw_light() {
	GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);	// 指定零号光源的位置
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);	// 表示各种光线照射到该材质上，经过很多次反射后追踪遗留在环境中的光线强度
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);	// 漫反射后的光照强度
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);	// 镜面反射后的光照强度
}





