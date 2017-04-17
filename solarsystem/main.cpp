#include "solar_system.hpp"
#include <iostream>



#define WINDOW_X_POS 50
#define WINDOW_Y_POS 50
#define WINDOW_CX 700
#define WINDOW_CY 700

solar_system ss;

void on_display(void) {
	ss.on_display();
}

void on_update(void) {
	ss.on_update();
}


void on_keyboard(unsigned char key, int x, int y) {
	ss.on_keyboard(key, x, y);
}

using namespace std;

int main(int argc, char** argv) {
	cout << "main in" << endl;

	glutInit(&argc, argv);
	cout << "glutInit ok" << endl;

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	cout << "glutInitDisplayMode ok" << endl;

	glutInitWindowPosition(WINDOW_X_POS, WINDOW_Y_POS);
	cout << "glutInitWindowPosition ok" << endl;

	glutCreateWindow("SolarSystem at Shiyanlou");
	cout << "glutCreateWindow ok" << endl;

	glutDisplayFunc(on_display);
	glutIdleFunc(on_update);
	glutKeyboardFunc(on_keyboard);
	glutMainLoop();



	return 0;
}

