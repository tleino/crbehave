all: tuples matrix canvas bank

tuples: tuples.feature
	make -C .. tests/tuples
	./tuples
matrix: matrix.feature
	make -C .. tests/matrix
	./matrix
canvas: canvas.feature
	make -C .. tests/canvas
	./canvas
bank: bank.feature
	make -C .. tests/bank
	./bank
clean:
	rm -f tuples matrix canvas bank

.PHONY: tuples matrix canvas bank
