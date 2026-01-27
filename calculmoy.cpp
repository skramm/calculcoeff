/**
*/

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <cassert>
#include <fstream>
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

//--------------------------------------------------
struct Module
{
	int              _semestre;
	std::string      _code;
	std::vector<int> _vcoeffue;
	Module( std::vector<string> vec )
	{
		std::assert( vec.size>3);
		_semestre = std::atoi( vec[0] );
		_code = vec[1];
		for( uint8_t i=2; i<vec.size(); i++ )
			_vcoeffue.push_back( std::atoi( vec[i] ) );
	}
};

//--------------------------------------------------
auto
readCSV_notes( std::string fname )
{
}
//--------------------------------------------------
/// Lecture des coefficients
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
	std::vector<Module> coeffs;
	auto rcoeffs = readCSV( fname );
	auto nb_UE = rcoeffs.at(0).size()-3;
	std::assert( rcoeffs.size() > 2 );
	std::cout << "-lecture de " << rcoeffs.size()-1 << " modules, dans " << nb_UE << " UE\n";
	std::vector<std::string> UE( nb_UE );
	for( uint8_t i=0; i<nb_UE; i++ )
	{
		UE[i] = rcoeffs.at(0).at(3+i);
		std::cout << " -UE" << i << ": " << UE[i] << '\n';
	}
	for( uint16_t i=1; i<rcoeffs.size(); i++ )
	{
		Module m(rcoeff[i);
		coeffs.push_back( m );
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
	auto coeff = readCSV_coeff( std::string(argv[1]) );
	auto notes = readCSV_notes( std::string(argv[2]) );
	compute( coeff, notes, "bilan.csv" );
}

