#
#  Makefile
#
#  Created by Changkun Ou on 16/06/07.
#  Copyright © 2016 Shiyanlou.com. All rights reserved.
#

CXX = g++
EXEC = solarsystem
SOURCES = main.cpp star.cpp solar_system.cpp
OBJECTS = main.o star.o solar_system.o
LDFLAGS = -lglut -lGL -lGLU -lm -std=c++11

all :
	$(CXX) $(SOURCES) $(LDFLAGS) -o $(EXEC)

clean:
	rm -f $(EXEC) *.gdb *.o


