all: tuples matrix canvas bank

tuples: tuples.feature
	make -C .. tests/tuples.report
matrix: matrix.feature
	make -C .. tests/matrix.report
canvas: canvas.feature
	make -C .. tests/canvas.report
bank: bank.feature
	make -C .. tests/bank.report
clean:
	rm -f tuples matrix canvas bank

.PHONY: tuples matrix canvas bank
