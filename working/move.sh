#!/bin/sh
for f in $(ls *.irr 2>/dev/null); do
	d=${f%.*}
	mkdir -p levels/$d
	mv $f levels/$d
done;

for f in $(ls *.obj 2>/dev/null); do
	../bin/Release/colmesh $f
	d=${f%.*}
	mkdir -p levels/$d
	mv $d.col levels/$d
	rm $f
done;

rm -f irrb.log
