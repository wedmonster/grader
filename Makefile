
all:
	g++ -o grader grader.cpp
	g++ -o inserter inserter.cpp

clean:
	rm -rf grade
	rm -rf inserter
