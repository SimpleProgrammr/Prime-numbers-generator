run: mfout primes
	./mfout/primes -e 10000000000 -q -o output.txt
	mv ./mfout/output.txt ./ready-primes.txt

mfout:
	mkdir mfout

primes:
	gcc -pthread prime-gen.c -lm -O3 -o ./mfout/primes

clean:
	rm -fr mfout