
LIB_FILES = Arena.cpp Bot.cpp BotRunner.cpp Obstacle.cpp Weapon.cpp
LIBOFILES = $(LIB_FILES:%.cpp=%.o)       

all	: libbotrunner.a
	make -C languages all

clean:
	rm -f *.o
	rm -f libbotrunner.a
	make -C languages clean

libbotrunner.a:	$(LIBOFILES)
	rm -f $@
	ar cq $@ $(LIBOFILES)
