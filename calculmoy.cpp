/**
\author S. Kramm

Champ fichier de notes:
- id (numéro étudiant)
- nom
- prénom
- date naissance
- notes
*/

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <cassert>
#include <fstream>
#include <map>
#include <algorithm>
//#include <chrono>
#include <iomanip>


// constantes
const auto idx_num   = 0;
const auto idx_nom   = 1;
const auto idx_pre   = 2;
const auto idx_note0 = 3;  ///< premier champ de note
const bool anonyme = false;

//--------------------------------------------------
/// Split a line of CSV file into fields
std::vector<std::string>
split_string( const std::string &s, char delim )
{
	std::vector<std::string> velems;
//	std::stringstream ss( TrimSpaces(s) );
	std::stringstream ss( s );
	std::string item;
	while( std::getline( ss, item, delim ) )
		velems.push_back(item);

	if( s.back() == delim )                 // add empty field if last char is the delim
		velems.push_back( std::string() );
	return velems;
}


//--------------------------------------------------
/// read CSV file \c filename
std::vector<std::vector<std::string>>
readCSV( std::string filename )
{
	std::ifstream file( filename );
	if( !file.is_open() )
		throw std::runtime_error( "Error, unable to open file " + filename );

	std::vector<std::vector<std::string>> out;

	std::string buff;
	int line = 0;
	while ( getline (file, buff ) )
	{
		line++;

		auto v_str = split_string( buff, ';' );
//		std::cout << "line " << line << ": " << buff << " size=" << v_str.size() << '\n';

		if( !v_str.empty() )
			if( v_str[0][0] != '#' )
				out.push_back( v_str );
	}
	return out;
}


//--------------------------------------------------
/// Module pédagogique
struct Module
{
	int              _semestre;
	std::string      _code;
	std::vector<int> _coeffue; ///< coeff pour chaque UE, identifiée par son index

	Module( const std::vector<std::string>& vec )
	{
		assert( vec.size()>3 );
		_semestre = std::stoi( vec[0] );
		_code = vec[1];
		std::cout << __FUNCTION__ << "() semestre=" << _semestre << " code=" << _code << std::endl;
	}
	void print() const
	{
		std::cout << "Module: sem=" << _semestre << " _code=" << _code << '\n';
		uint32_t sum_coeff = 0;
		int i=0;
		for( const auto& c: _coeffue )
		{
			std::cout << " -" << i++ << ":" << c << "\n";
			sum_coeff += c;
		}
		std::cout << "Total coeff de "<< _code << "=" << sum_coeff << '\n';
	}
};

//--------------------------------------------------
struct ListeModules
{
	std::vector<Module> v_liste; ///< liste de modules pédagogiques
	std::vector<uint32_t> v_totUE; ///< totaux par UE
	std::vector<std::string> v_UE; ///< noms des UE

/// Calcul totaux par UE
	void calculTotaux()
	{
		assert( v_UE.size() ); // impossible de calculer les totaux par UE.. si on a pas d'UE!
		v_totUE.resize( v_UE.size() );

		for( auto& tot: v_totUE )
			tot = 0; // on initialise tous les totaux a 0 (pour chaque UE)

		for( uint16_t i=0; i<v_UE.size(); i++ )
		{
			for( const auto& mod: v_liste )
				v_totUE[i] += mod._coeffue[i];
		}
	}
	void print()
	{
		std::cout << "*** ListeModules ***\n-liste UE:\n";
		for( const auto& ue: v_UE )
			std::cout << ue << " ";
		std::cout << "\n-Totaux:\n";
		for( const auto& tot: v_totUE )
			std::cout << "  -" << tot << "\n";
		std::cout << "-Modules:\n";
		for( const auto& m: v_liste )
			m.print();

	}
};

//--------------------------------------------------
/// Notes d'un étudiant dans chaque module et dans chaque UE
struct Notes
{
	std::string    _nom;
	std::string    _prenom;
	std::string    _id;
	std::map<std::string,float> _notes; ///< notes pour chaque module
	std::vector<float> _moyUE;   ///< moyenne pour chaque UE (résultat du calcul)
};


//--------------------------------------------------
/// Renvoie une liste d'objets de type \c Notes
auto
readCSV_notes( std::string fname, const ListeModules& listeMod )
{
	const auto& coeffs = listeMod.v_liste;
	auto liste = readCSV( fname );
	assert( liste.size() > 2 );
	assert( liste[0].size() > idx_note0 ); // la ligne doit contenir plus de 4 items (num, nom, plus les notes par module)

// lecture des intitulés des modules
	std::vector<std::string> v_mod;
	for( uint16_t i=idx_note0; i<liste[0].size(); i++ )
	{
		auto mod = liste[0].at(i);
		std::cout << __FUNCTION__ << "() i=" << i << " mod=" << mod << '\n';
		v_mod.push_back( mod );

// vérification que les modules existent
//		auto f = std::find( coeffs
	}

// lecture des notes
	std::vector<Notes> v_notes;
	for( uint16_t i=1; i<liste.size(); i++ ) // on saute la 1ere ligne
	{
		auto line = liste[i];
		assert( line.size() == coeffs.size() + idx_note0 );
		Notes notes;
		notes._nom = line[1];
		notes._id = line[0];
		std::cout << __FUNCTION__ << "() i=" << i << " nom=" << notes._nom << "\n";
		for( uint16_t j=idx_note0; j<line.size(); j++ )
		{
			std::cout << "  j=" << j << " val=" << line[j]  << " mod=" << v_mod[j-idx_note0] << "\n";
			auto value = 0.;
			if( line[j] != "ABI" )
				value = std::stof( line[j] );
			if( value < 0. || value > 20. )
			{
				std::cerr << "Erreur, valeur note invalide: " << value << "\n";
				std::exit(2);
			}
			notes._notes[ v_mod.at(j-idx_note0) ] = value;
		}
		v_notes.push_back( notes );
	}
	return v_notes;

}

//--------------------------------------------------
/// Calcul des moyennes par UE, en fonction des coefficients de chaque module
void
compute(
	const ListeModules& listeMod, ///< les modules pédagogiques
	std::vector<Notes>& vnotes    ///< les notes, auxquelles on va ajouter les moy par UE
)
{
	std::cout << __FUNCTION__ << "(): nb etud=" << vnotes.size() << '\n';

	auto nbUE = listeMod.v_UE.size();

	const auto& v_listeMod = listeMod.v_liste;
	for( auto& etud: vnotes )
	{
		std::cout << "\n* etud=" << etud._nom << '\n';
		etud._moyUE.resize( nbUE );

		for( uint16_t idxUE=0; idxUE<nbUE; idxUE++ ) // pour chaque UE
		{
			auto ue = listeMod.v_UE[idxUE];
			std::cout << "\n  * UE=" << ue << " idx=" << idxUE << "\n";
			float sum_UE = 0.;
			for( const auto& note: etud._notes ) // on itere sur chaque note
			{
				std::cout << "ajout note:" << note.second << " module=" << note.first << "\n";
				auto it = std::find_if(
					v_listeMod.begin(),
					v_listeMod.end(),
					[&note]                // lambda
					(const Module& m)
					{ return m._code == note.first;}
				);
				if( it == v_listeMod.end() )
				{
					std::cerr << "Erreur, impossible de trouver " << note.first << " dans les coeffs\n";
					std::exit(3);
				}

				auto c = it->_coeffue;

				std::cout << "mod=" << it->_code << ", coef pour " << ue << "=" << c.at(idxUE) << "\n";
				sum_UE += note.second * c.at(idxUE);
				std::cout << "sum_UE=" << sum_UE << '\n';
			}
			etud._moyUE[idxUE] = sum_UE / listeMod.v_totUE.at(idxUE);
			std::cout << "moy=" << etud._moyUE[idxUE] << '\n';
		}
	}
}
//--------------------------------------------------
/// Lecture des coefficients dans un CSV, pour chaque module et dans chaque UE
/**
Colonnes:
 - semestre
 - code
 - nom
 - UE1
 - UE2
 - UE...
*/
auto
readCSV_coeff( std::string fname )
{
	ListeModules listeMod;
	auto liste = readCSV( fname );
	assert( liste.size() > 2 );

	auto nb_UE = liste.at(0).size()-3;
	std::cout << "-lecture de " << liste.size()-1 << " modules, dans " << nb_UE << " UE\n";
	for( uint16_t i=0; i<nb_UE; i++ )
	{
		listeMod.v_UE.push_back( liste.at(0).at(3+i) );
		std::cout << " -UE" << i << ": " << listeMod.v_UE[i] << '\n';
	}

	for( uint16_t i=1; i<liste.size(); i++ ) // à partir de la 2ème ligne
	{
		Module m( liste[i] );
//		std::cout << __FUNCTION__ << "(): i=" << i<< " code=" << m._code << "\n";

// vérification que chaque ligne a bien le bon nbe de valeurs
		assert( liste[i].size() == nb_UE + 3 );

		for( uint16_t j=3; j<liste[i].size(); j++ )
		{
			std::cout <<__FUNCTION__ << "() j=" << j << " coeff=" << std::stoi( liste[i][j] ) << std::endl;
			m._coeffue.push_back( std::stoi( liste[i][j] ) );
		}
		listeMod.v_liste.push_back( m );
		m.print();
	}
	listeMod.calculTotaux();
	return listeMod;
}
//--------------------------------------------------
void
printCoeffs( const std::vector<Module>& coeffs )
{
	for( const auto& mod: coeffs )
	{
		std::cout << "code=" << mod._code << " sem=" << mod._semestre << "\n";
		int i=0;
		for( const auto& m: mod._coeffue )
			std::cout << "  -"<< i++ << ":" << m << "\n";
	}
}

//--------------------------------------------------
void
printMoyennes( const std::vector<Notes>& vnotes, const ListeModules& listeMod, std::string fout )
{
	std::ofstream f(fout);
	if( !f.is_open() )
	{
		std::cerr << "Error: cannot create output file " << fout << '\n';
		std::exit(4);
	}

	const char* sep = ";";
	f << "\nNuméro" << sep << "Nom";
	for( const auto& ue: listeMod.v_UE )
		f << sep << ue;
	f << "\n" << std::setprecision(4);

	for( const auto& etud: vnotes )
	{
		f << etud._id;
		if( !anonyme )
			f << sep << etud._nom << sep << etud._prenom;
		for( const auto& moy: etud._moyUE )
			f << sep << moy;
		f << "\n";
	}
		f << "\n";
}
//--------------------------------------------------
int
main( int argc, const char* argv[] )
{
	auto fout="moy_ue.csv";
	if( argc < 3 )
	{
		std::cerr << "usage calculmoy coeff_file.csv notes.csv [outputfile]\n";
		return 1;
	}
	if( argc > 3 )
	{
		fout = argv[3];
	}
	auto listeMod = readCSV_coeff( std::string(argv[1]) );
	listeMod.print();
//	printCoeffs( ue_coeffs.second.v_liste );
	auto vnotes = readCSV_notes( std::string(argv[2]), listeMod );
	compute( listeMod, vnotes );
	printMoyennes( vnotes, listeMod, fout );
}

