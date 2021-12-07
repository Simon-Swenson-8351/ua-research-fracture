LIBS = -lAlembic -lIlmImf -lIlmImfUtil -ljsoncpp
INCLUDE_DIRS = -I /usr/include/OpenEXR/

alembic-pointcloud-converter: alembic-pointcloud-converter.cpp
	g++ $? $(LIBS) $(INCLUDE_DIRS) -o $@

clean: 
	rm alembic-pointcloud-converter