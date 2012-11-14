
all: debug

release:
	( test -d build || (mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release) )
	( cd build && make )


debug:
	( test -d build || (mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug) )
	( cd build && make )

test:
	( cd testing && btest )

clean:
	rm -rf build
