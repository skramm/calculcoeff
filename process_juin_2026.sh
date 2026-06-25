
echo "Build"
g++ -o calculmoy calculmoy.cpp
if [ $? = 0 ]; then
	echo "Traitement S2"
	./calculmoy coefficients.csv resultat_s2_p.csv calculmoy_s2.ini >out/stdout_S2
	echo "Traitement S4"
	./calculmoy coefficients.csv resultat_s4_p.csv calculmoy_s4.ini >out/stdout_S4
	echo "Traitement S6"
	./calculmoy coefficients.csv resultat_s6.csv calculmoy_s6.ini >out/stdout_S6
else
	echo "echec compilation"
fi

