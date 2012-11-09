
all:
	clang -g -O0 driver.c paraglob.c multifast-ac/ahocorasick.c multifast-ac/node.c -o paraglob

clean:
	rm paraglob
