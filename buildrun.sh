set -x
g++ -o calculmoy calculmoy.cpp
if [ $? = 0 ]
then
	./calculmoy coeffs.csv liste_notes.csv
else
	echo "Compilation error!"
fi

