#include "solar_system.hpp"
#include <iostream>

#define TIME_PAST 1
#define SELF_ROTATE 3

#define REST 700
#define REST_Z (REST)
#define REST_Y (-REST)

#define OFFSET 20

#define SUN_RADIUS 48.74
#define MER_RADIUS  7.32
#define VEN_RADIUS 18.15
#define EAR_RADIUS 19.13
#define MOO_RADIUS  6.15
#define MAR_RADIUS 10.19
#define JUP_RADIUS 42.90
#define SAT_RADIUS 36.16
#define URA_RADIUS 25.56
#define NEP_RADIUS 24.78

#define MER_DIS   62.06
#define VEN_DIS  115.56
#define EAR_DIS  168.00
#define MOO_DIS   26.01
#define MAR_DIS  228.00
#define JUP_DIS  333.40
#define SAT_DIS  428.10
#define URA_DIS 848.00
#define NEP_DIS 949.10

#define MER_SPEED   87.0
#define VEN_SPEED  225.0
#define EAR_SPEED  365.0
#define MOO_SPEED   30.0
#define MAR_SPEED  687.0
#define JUP_SPEED 1298.4
#define SAT_SPEED 3225.6
#define URA_SPEED 3066.4
#define NEP_SPEED 6014.8


#define SET_VALUE_3(name, v0, v1, v2) \
	do { \
		(name)[0] = (v0); \
		(name)[1] = (v1); \
		(name)[2] = (v2); \
	}while (0); 


enum STARS {
	sun,
	mercury,
	venus,
	earth,
	moon,
	mars,
	jupiter,
	saturn,
	uranus,
	neptune,

};

solar_system::solar_system() {
	std::cout << "solar_system ctor" << std::endl;

	view_x_ = 0;
	view_y_ = REST_Y;
	view_z_ = REST_Z;

	up_z_ = 1;

	GLfloat rgbcolor[3] = { 1, 0, 0 };

#define SET_VALUE_3(name, v0, v1, v2) \
	do { \
		(name)[0] = (v0); \
		(name)[1] = (v1); \
		(name)[2] = (v2); \
	}while (0); 

#define sv3(v0, v1, v2) SET_VALUE_3(rgbcolor, (v0), (v1), (v2));

#define mk_planet(name, r, d, s) stars_[name] = new planet(r, d, s, SELF_ROTATE, stars_[sun], rgbcolor);
	
	stars_[sun] = new light_planet(SUN_RADIUS, 0, 0, SELF_ROTATE, nullptr, rgbcolor);

	SET_VALUE_3(rgbcolor, .2, .2, .5);
	stars_[mercury] = new planet(MER_RADIUS, MER_DIS, MER_SPEED, SELF_ROTATE, stars_[sun], rgbcolor);

	SET_VALUE_3(rgbcolor, 1, .7, 0);
	stars_[venus] = new planet(VEN_RADIUS, VEN_DIS, VEN_SPEED, SELF_ROTATE, stars_[sun], rgbcolor);

	SET_VALUE_3(rgbcolor, 0, 1, 0);
	stars_[earth] = new planet(EAR_RADIUS, EAR_DIS, EAR_SPEED, SELF_ROTATE, stars_[sun], rgbcolor);

	SET_VALUE_3(rgbcolor, 1, 1, 0);
	stars_[moon] = new planet(MOO_RADIUS, MOO_DIS, MOO_SPEED, SELF_ROTATE, stars_[earth], rgbcolor);

	sv3(1, .5, .5);
	stars_[mars] = new planet(MAR_RADIUS, MAR_DIS, MAR_SPEED, SELF_ROTATE, stars_[sun], rgbcolor);

	sv3(1, 1, .5);
	mk_planet(jupiter, JUP_RADIUS, JUP_DIS, JUP_SPEED);

	sv3(.5, 1, .5);
	mk_planet(saturn, SAT_RADIUS, SAT_DIS, SAT_SPEED);

	sv3(.4, .4, .4);
	mk_planet(uranus, URA_RADIUS, URA_DIS, URA_SPEED);

	sv3(.5, .5, 1);
	mk_planet(neptune, NEP_RADIUS, NEP_DIS, NEP_SPEED);


#undef mk_planet
#undef sv3
#undef SET_VALUE_3

}

solar_system::~solar_system() {
	std::cout << "solar_system dtor" << std::endl;

	for (int i = 0; i < STAR_NUM; i++) {
		delete stars_[i];
	}
}

void solar_system::on_display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(.7f, .7f, .7f, .1f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(75.0f, 1.0f, 1.0f, 40000000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(view_x_, view_y_, view_z_, center_x_, center_y_, center_z_, up_x_, up_y_, up_z_);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	for (int i = 0; i < STAR_NUM; i++) {
		stars_[i]->draw();
	}

	glutSwapBuffers();
}

void solar_system::on_update() {
	for (int i = 0; i < STAR_NUM; i++) {
		stars_[i]->update(TIME_PAST);
	}

	this->on_display();
}

void solar_system::on_keyboard(unsigned char key, int x, int y) {

	switch (key) {
	case 'w': view_y_ += OFFSET; break;
	case 's': view_z_ += OFFSET; break;
	case 'S': view_z_ -= OFFSET; break;
	case 'a': view_x_ -= OFFSET; break;
	case 'd': view_x_ += OFFSET; break;
	case 'x': view_y_ -= OFFSET; break;
	case 'r':
		view_x_ = 0; view_y_ = REST_Y; view_z_ = REST_Z;
		center_x_ = center_y_ = center_z_ = 0;
		up_x_ = up_y_ = 0; up_z_ = 1;
		break;
	case 27: exit(0); break;
	default: break;
	}
}




