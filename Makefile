
all:
	g++ -o grader grader.cpp
	./grader

clean:
	rm -rf grade
