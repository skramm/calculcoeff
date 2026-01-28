/**
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
//#include <iomanip>

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
struct Module
{
	int              _semestre;
	std::string      _code;
	std::map<std::string,int> _coeffue; ///< coeff pour chaque UE, identifiée par son nom
	
	Module( const std::vector<std::string>& vec )
	{
		assert( vec.size()>3 );
		_semestre = std::stoi( vec[0] );
		_code = vec[1];
		std::cout << __FUNCTION__ << "() semestre=" << _semestre << " code=" << _code << std::endl;
/*		for( uint8_t i=3; i<vec.size(); i++ )
		{
		std::cout <<"i=" << i << " vec[i]=" << vec[i] << std::endl;
			_coeffue.push_back( std::stoi( vec[i] ) );
			std::cout << __FUNCTION__ << "(): code=" << _code << " i=" << i << " coeff=" << _coeffue.back() << "\n";
		}*/
	}
};

//--------------------------------------------------
/// Notes d'un étudiant dans chaque module et dans chaque UE
struct Notes
{
	std::string    _nom;
	std::string    _id;
	std::map<std::string,float> _notes; ///< notes pour chaque module
	std::map<std::string,float> _moy;   ///< moyenne pour chaque UE (résultat du calcul)
};


//--------------------------------------------------
/// Renvoie une liste d'objets de type \c Notes
auto
readCSV_notes( std::string fname, const std::vector<Module>& coeffs )
{
	auto liste = readCSV( fname );
	assert( liste.size() > 2 );
	assert( liste[0].size() > 2 ); // la ligne doit contenir plus de 2 items (num, nom, plus les notes par module)

// lecture des intitulés des modules
	std::vector<std::string> v_mod;
	for( uint16_t i=2; i<liste[0].size(); i++ )
	{
		auto mod = liste[0].at(i);
		std::cout << __FUNCTION__ << "() i=" << i << " mod=" << mod << '\n';
		v_mod.push_back( mod );
		
// vérification que les modules existent
//		auto f = std::find( coeffs
	}

// lecture des notes
	std::vector<Notes> v_notes;
	for( uint16_t i=1; i<liste.size(); i++ )
	{
		auto line = liste[i];
		assert( line.size() == coeffs.size() + 2 );
		Notes notes;
		notes._nom = line[1];
		notes._id = line[0];
		std::cout << __FUNCTION__ << "() i=" << i << " nom=" << notes._nom << "\n";
		for( uint16_t j=2; j<line.size(); j++ )
		{
			std::cout << "  j=" << j << " val=" << line[j]  << " mod=" << v_mod[j-2] << "\n";
			auto value = std::stof( line[j] );
			if( value < 0. || value > 20. )
			{
				std::cerr << "Erreur, valeur note invalide: " << value << "\n";
				std::exit(2);
			}
			notes._notes[ v_mod[j-2] ] = value;
		}
		v_notes.push_back( notes );
	}
	return v_notes;

}
//#if 0

//--------------------------------------------------
void
compute(
	const std::vector<std::string>& vUE,
	const std::vector<Module>& vcoeff,
	std::vector<Notes>&        vnotes,
	std::string fname
)
{
	std::cout << "nb etud=" << vnotes.size() << '\n';
	for( const auto& etud: vnotes )
	{
		std::cout << "etud=" << etud._nom << '\n';
		float sum_UE = 0.;
		for( const auto& ue: vUE)
		{
			std::cout << "UE=" << ue << "\n";
			for( const auto& note: etud._notes )
			{
				std::cout << "ajout note:" << note.second << " module=" << note.first << "\n";
				auto it = std::find_if(
					vcoeff.begin(),
					vcoeff.end(),
					[&note]                // lambda
					(const Module& m)
					{ return m._code == note.first;}
				);
				if( it == vcoeff.end() )
				{
					std::cerr << "Erreur, impossible de trouver " << note.first << " dans les coeffs\n";
					std::exit(3);
				}
				std::cout << "trouvé mod " << it->_code << '\n';
				
				auto c = it->_coeffue.at(note.first);
				std::cout << "note.first=" << note.first << " c=" << c << "\n";
/*				sum_UE += note.second * c;*/
			}
		}
		std::cout << "sum_UE=" << sum_UE << "\n";
//		for( const auto note: vnotes )
		
	}

}
//#endif
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
//std::pair<std::vector<std::string>,std::vector<Module>>
readCSV_coeff( std::string fname )
{
	std::vector<Module> coeffs;
	auto liste = readCSV( fname );
	assert( liste.size() > 2 );

	auto nb_UE = liste.at(0).size()-3;
	std::cout << "-lecture de " << liste.size()-1 << " modules, dans " << nb_UE << " UE\n";
	std::vector<std::string> v_UE( nb_UE );
	for( uint8_t i=0; i<nb_UE; i++ )
	{
		v_UE[i] = liste.at(0).at(3+i);
		std::cout << " -UE" << i << ": " << v_UE[i] << '\n';
	}

	for( uint16_t i=1; i<liste.size(); i++ ) // ç partir de la 2ème ligne
	{
		Module m( liste[i] );
//		std::cout << __FUNCTION__ << "(): i=" << i<< " code=" << m._code << "\n";

// vérification que chaque ligne a bien le bon nbe de valeurs
		assert( liste[i].size() == nb_UE + 3 );
		
		for( uint16_t j=3; j<liste[i].size(); j++ )
		{
//			std::cout <<__FUNCTION__ << "() j=" << j << std::endl;
			m._coeffue[ v_UE[j-3] ] = std::stoi( liste[i][j] ) ;
//			std::cout << __FUNCTION__ << "(): coeff=" << m._coeffue[ UE[j-3] ] << "\n";
		}
		
		coeffs.push_back( m );
		
	}
	return std::make_pair(v_UE,coeffs);
}
//--------------------------------------------------
void
printCoeffs( const std::vector<Module>& coeffs )
{
	for( const auto& mod: coeffs )
	{
		std::cout << "code=" << mod._code << " sem=" << mod._semestre << "\n";
		for( const auto& m: mod._coeffue )
			std::cout << "  -"<< m.first << ":" << m.second << "\n";
	}
}

//--------------------------------------------------
int main( int argc, const char* argv[] )
{
	if( argc < 3 )
	{
		std::cerr << "usage calculmoy coeff_file.csv notes.csv\n";
		return 1;
	}
	auto ue_coeffs = readCSV_coeff( std::string(argv[1]) );
	printCoeffs( ue_coeffs.second );
	auto vnotes = readCSV_notes( std::string(argv[2]), ue_coeffs.second );
	compute( ue_coeffs.first, ue_coeffs.second, vnotes, "bilan.csv" );
}

