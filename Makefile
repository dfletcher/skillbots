
all:
	make -C gameserver all

clean:
	make -C gameserver clean

run: all
	make -C gameserver run
