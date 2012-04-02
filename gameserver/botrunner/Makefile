
LIB_FILES = Arena.cpp Bot.cpp BotLanguage.cpp BotRunner.cpp BotRunnerException.cpp  Obstacle.cpp Weapon.cpp Utility.cpp
LIBOFILES = $(LIB_FILES:%.cpp=%.o)       
CXXFLAGS = -I. -O3

all	: libbotrunner.a
	make -C languages all

clean:
	rm -f *.o
	rm -f libbotrunner.a
	make -C languages clean

libbotrunner.a:	$(LIBOFILES)
	rm -f $@
	ar cq $@ $(LIBOFILES)
