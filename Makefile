
all:
	clang driver.c paraglob.c -o paraglob

clean:
	rm paraglob
