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
#include <chrono>
#include <iomanip>

//#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

//-------------------------------------------------------------------
enum SortCrit: char
{
	SC_none,   ///< pas de tri
	SC_alpha,  ///< par nom
	SC_num,    ///< par numéro étudiant
	SC_rankHL, ///< par rang (classement)
	SC_rankLH  ///< par rang (classement)
};

const char* getStr( SortCrit i )
{
	switch(i)
	{
		case SC_none:   return "none";   break;
		case SC_alpha:  return "alpha";  break;
		case SC_num:    return "num";    break;
		case SC_rankHL: return "rankHL"; break;
		case SC_rankLH: return "rankLH"; break;
	}
	assert(0);
	return ""; // to avoid a build warning
}
//-------------------------------------------------------------------
/// Identifiant textuel du critere de tri, à lire dans le fichier de configuration \c .ini,
/// dans la section \c [sorting], champ \c sortCrit
std::map<std::string,SortCrit> g_sortCritStr = {
	{ "none",  SC_none  },
	{ "alpha", SC_alpha },
	{ "idnum", SC_num,  },
	{ "rankHL",  SC_rankHL  },
	{ "rankLH",  SC_rankLH  }
};

//-------------------------------------------------------------------
/// Identifiant des champs à lire dans les fichiers CSV de notes
enum ColIndex : char
{
	CI_numero,  ///< index colonne numéro étudiant
	CI_nom,
	CI_prenom,
	CI_note1  ///< index colonne contenant la 1ere note
};

const char* getStr( ColIndex i )
{
	switch(i)
	{
		case CI_numero: return "numero"; break;
		case CI_nom:    return "nom";    break;
		case CI_prenom: return "prenom"; break;
		case CI_note1:  return "note1";  break;
	}
	assert(0);
	return ""; // to avoid a build warning
}
//-------------------------------------------------------------------
/// Identifiant textuel des colonnes dans le fichier de configuration .ini, dans la section [columns]
std::map<ColIndex,std::string> g_colIndexStr = {
	{ CI_numero,  "numero" },
	{ CI_nom,     "nom"    },
	{ CI_prenom,  "prenom" },
	{ CI_note1,   "note1"  }
};


//--------------------------------------------------
/// from https://stackoverflow.com/questions/17223096/
auto
return_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%X");
    return ss.str();
}

//-------------------------------------------------------------------
/// Parametres de fonctionnement
struct Params
{
//	char delimiter_in = ';';
//	char commentChar = '#';
	std::map<ColIndex,int> colIndex;
	SortCrit    sortCriterion;
	bool        timestamp = false;
	std::string date;
	bool anonyme = true;

private:
	boost::property_tree::ptree _ptree;

public:
	/// Constructor, assigns default values
	Params()
	{
		colIndex[CI_numero]   = 0;
		colIndex[CI_nom]      = 1;
		colIndex[CI_prenom]   = 2;
		colIndex[CI_note1]    = 4;
		date = return_current_time_and_date();
	}

	/// constructor, calls main constructor an tries to read .ini file
	Params( std::string filename ): Params()
	{
		bool hasIniFile = true;
		try
		{
			boost::property_tree::ini_parser::read_ini( filename, _ptree );
		}
		catch( const std::exception& e )
		{
			std::cerr << "No file " << filename << ", keeping defaults\n";
			hasIniFile = false;
		}
		if( hasIniFile )
		{
			std::cout << "reading data in config file " << filename << '\n';
			std::string sortCritName;
			sortCritName = _ptree.get<std::string>( "sorting.sortCrit", sortCritName );
			std::cerr << "sortCritName=" << sortCritName << '\n';

			auto f = g_sortCritStr.find(sortCritName);
			if( f == g_sortCritStr.end() )
				throw std::runtime_error( "Error, invalid item value " + sortCritName + " in .ini file" );
			sortCriterion = g_sortCritStr[sortCritName];

			anonyme = _ptree.get<bool>( "misc.anonyme", anonyme );
//			anonyme = (bool)_ptree.get<int>( "misc.anonyme", anonyme );

/*			groupKey1 = (bool)_ptree.get<int>( "grouping.groupKey1", groupKey1 );
			groupKey2 = (bool)_ptree.get<int>( "grouping.groupKey2", groupKey2 );

			groupKey1_name = _ptree.get<std::string>( "grouping.groupKey1_name", groupKey1_name );
			groupKey2_name = _ptree.get<std::string>( "grouping.groupKey2_name", groupKey2_name );

			groupKey1_pos = _ptree.get<int>( "grouping.groupKey1_pos", groupKey1_pos );
			groupKey2_pos = _ptree.get<int>( "grouping.groupKey2_pos", groupKey2_pos );

			std::string pairs_1 = _ptree.get<std::string>( "grouping.groupKey1_pairs", std::string() );
			std::string pairs_2 = _ptree.get<std::string>( "grouping.groupKey2_pairs", std::string() );

			if( !pairs_1.empty() )
				groupKey1_pairs = extractPairs( pairs_1 );
			if( !pairs_2.empty() )
				groupKey2_pairs = extractPairs( pairs_2 );
*/
			for( const auto& map_key: g_colIndexStr)
				colIndex[map_key.first] = _ptree.get<int>( "columns." + map_key.second, colIndex[map_key.first] );
		}
	}

	friend std::ostream& operator << ( std::ostream& f, const Params& p )
	{
		f << "Parametres:\n - index des colonnes:\n";
		for (const auto& any : p.colIndex)
		{	f << getStr(any.first) << ":" << any.second << '\n';
		}
		return f;
	}
};

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
		std::cout << "Module: sem=" << _semestre << " code=" << _code << '\n';
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
	std::map<std::string,double> _notes; ///< notes pour chaque module
	std::vector<double> _moyUE;   ///< moyenne pour chaque UE (résultat du calcul)
	double _moy;   ///< moyenne générale

	Notes( const std::vector<std::string>& line, const Params& p )
		:_nom{line[p.colIndex.at(CI_nom)]}, _prenom{line[p.colIndex.at(CI_prenom)]}, _id{line[p.colIndex.at(CI_numero)]}
	{}
};

//--------------------------------------------------
/// Renvoie une liste d'objets de type \c Notes
auto
readCSV_notes( std::string fname, const ListeModules& listeMod, const Params& par )
{
	const auto& coeffs = listeMod.v_liste;
	auto liste = readCSV( fname );
	assert( liste.size() > 2 );
	assert( liste[0].size() > par.colIndex.at(CI_note1) ); // la ligne doit contenir plus de 4 items (num, nom, plus les notes par module)

// lecture des intitulés des modules
	std::vector<std::string> v_mod;
	for( uint16_t i=par.colIndex.at(CI_note1); i<liste[0].size(); i++ )
	{
		auto mod = liste[0].at(i);
		std::cout << __FUNCTION__ << "() i=" << i << " mod=" << mod << '\n';
		if( mod.size() )
			v_mod.push_back( mod );

// vérification que les modules existent
//		auto f = std::find( coeffs
	}
	std::cout << "Fichier de notes: " << v_mod.size() << " modules en 1ere ligne\n";

// lecture des notes
	std::vector<Notes> v_notes;
	for( uint16_t i=1; i<liste.size(); i++ ) // on saute la 1ere ligne
	{
		auto line = liste[i];
//		std::cout << "line size="<< line.size() << " coeffs.size()=" << coeffs.size() << "\n";

		if( line.size() > coeffs.size() + par.colIndex.at(CI_note1) )
		{
			std::cerr << "Erreur ligne " << i << ": " << line.size() - par.colIndex.at(CI_note1) << " notes présentes, au lieu de " << coeffs.size() << " attendues\n";
			auto ii=0;
			for( const auto& l: line )
				std::cout << " -" << ii++ << ":'" << l << "' len=" << l.size() << '\n';

			std::exit(5);
		}
		Notes notes( line, par );
		std::cout << __FUNCTION__ << "() i=" << i << " nom=" << notes._nom << "\n";
		for( uint16_t j=par.colIndex.at(CI_note1); j<line.size(); j++ )
		{
			std::cout << "  j=" << j << " val=" << line[j]  << " mod=" << v_mod[j-par.colIndex.at(CI_note1)] << "\n";
			auto value = 0.;
			if( line[j] != "ABI" && line[j].size() != 0 )
				value = std::stof( line[j] );
			if( value < 0. || value > 20. )
			{
				std::cerr << "Erreur, valeur note invalide: " << value << "\n";
				std::exit(2);
			}
			notes._notes[ v_mod.at(j-par.colIndex.at(CI_note1)) ] = value;
		}
		v_notes.push_back( notes );
	}
	return v_notes;

}

//--------------------------------------------------
/// Calcul des moyennes par UE, en fonction des coefficients de chaque module
void
compute(
	const ListeModules& listeMod,  ///< les modules pédagogiques
	std::vector<Notes>& vnotes,    ///< les notes, auxquelles on va ajouter les moy par UE
	const Params& par              ///< parametres
)
{
	std::cout << __FUNCTION__ << "(): nb etud=" << vnotes.size() << '\n';

	auto nbUE = listeMod.v_UE.size();

	const auto& v_listeMod = listeMod.v_liste;
	for( auto& etud: vnotes )
	{
		std::cout << "\n* etud=" << etud._nom << '\n';
		etud._moyUE.resize( nbUE );
		auto sum = 0.;
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
					std::cerr << "Erreur, impossible de trouver le module '" << note.first << "' dans les coeffs\n";
					std::exit(3);
				}

				auto c = it->_coeffue;

				std::cout << "mod=" << it->_code << ", coef pour " << ue << "=" << c.at(idxUE) << "\n";
				sum_UE += note.second * c.at(idxUE);
				std::cout << "sum_UE=" << sum_UE << '\n';
			}
			etud._moyUE[idxUE] = sum_UE / listeMod.v_totUE.at(idxUE);
			std::cout << "moy=" << etud._moyUE[idxUE] << '\n';
			sum += etud._moyUE[idxUE];
		}
		etud._moy = sum / nbUE;
	}

// TODO peut-être plus pertinent de faire une lambda par type de tri demandé ?
	if( par.sortCriterion != SC_none )
	{
		std::cout << "sorting !\n";
		std::sort(
			vnotes.begin(),
			vnotes.end(),
			[&par]                               // lambda
			(const Notes& n1, const Notes& n2)
			{
				switch( par.sortCriterion )
				{
					case SC_alpha: return n1._nom < n2._nom;
					case SC_num:   return n1._id  < n2._id;
					case SC_rankLH:  return n1._moy < n2._moy;
					case SC_rankHL:  return n2._moy < n1._moy;
					default: assert(0);
				}
			}
		);
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
		std::cout << "liste size="<< liste[i].size() << "\n";
		assert( liste[i].size() == nb_UE + 3 );

		for( uint16_t j=3; j<liste[i].size(); j++ )
		{
//			std::cout <<__FUNCTION__ << "() j=" << j << " coeff=" << std::stoi( liste[i][j] ) << std::endl;
			auto value = 0;
			if( liste[i][j].size() )
				value = std::stoi( liste[i][j] );
			m._coeffue.push_back( value );
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
auto
openfile( std::string name, const Params& par, std::string ext )
{
	std::ostringstream oss;
	oss << name;
	if( par.timestamp )
		oss << "_" << par.date;
	oss << "." << ext;
	std::string fname = oss.str();
	std::ofstream f(fname);
	if( !f.is_open() )
	{
		std::cerr << "Erreur: impossible de créer fichier '" << fname << "'\n";
		std::exit(4);
	}
	std::cout << "fichier " << fname << " ouvert\n";
	return f;
}

//--------------------------------------------------
void
printMoyennesHtml( const std::vector<Notes>& vnotes, const ListeModules& listeMod, std::string fout, const Params& par )
{
	auto tdo = "<td>";
	auto tdc = "</td>";
	auto f = openfile( fout, par, "html" );

	f << std::setprecision(4);

	f << "<!doctype html>\n<html><head>\n"
		<< "<title>Moyennes par UE</title>\n"
		<< "<meta charset='utf-8'>\n"
		<< "<link rel='stylesheet' href='ue.css'>\n"
		<< "</head><body>\n";

	f << "<h1>Moyennes par UE</h1>\n";

	f << "<table>\n"
		<< "<tr><th></th><th>Numéro</th>";
	if( !par.anonyme )
		f << "<th>Nom</th><th>Prenom</th>\n";
	for( const auto& ue: listeMod.v_UE )
		f << "<th>" << ue << "</th>\n";
	f << "<th>MOY</th>\n";
	f << "</tr>\n";

	auto nbUE = listeMod.v_UE.size();
	uint16_t i=0;
	for( const auto& etud: vnotes )
	{
		f << "<tr>" << tdo << ++i << tdc << tdo << etud._id << tdc;
		if( !par.anonyme )
			f << tdo << etud._nom << tdc << tdo << etud._prenom << tdc;

		for( const auto& moy: etud._moyUE )
			f << tdo << moy << tdc;
		f << "<td class='bold'>" << etud._moy << tdc;

		f << "</tr>\n";
	}
	f << "\n";

	f << "</table>\n<p>" << par.date << "</p>\n";
	f << "</body></html>\n";

}
//--------------------------------------------------
void
printMoyennesCsv( const std::vector<Notes>& vnotes, const ListeModules& listeMod, std::string fout, const Params& par )
{
	auto f = openfile( fout, par, "csv" );

	const char* sep = ";";
	f << "Numéro";
	if( !par.anonyme )
		f << sep << "Nom" << sep << "Prenom";
	for( const auto& ue: listeMod.v_UE )
		f << sep << ue;
	f << "\n" << std::setprecision(4);

	for( const auto& etud: vnotes )
	{
		f << etud._id;
		if( !par.anonyme )
			f << sep << etud._nom << sep << etud._prenom;
		for( const auto& moy: etud._moyUE )
			f << sep << moy;
		f << sep << etud._moy << "\n";
	}
	f << "\n";
}


//--------------------------------------------------
/// main. Requires 2 args, plus one optional
/**
-1: fichier CSV contenant les coefficients
-2: fichier CSV contenant les notes par module pédagogique, 1 ligne par étudiant
-3: nom du fichier de sortie


calculmoy [-t] [-a] coeffs.csv notes.csv [outfile]
*/
int
main( int argc, const char* argv[] )
{
	Params params( "calculmoy.ini" ); // nom du fichier de configuration
	std::cout << params;
//	std::exit(0);
	auto fout="out/moy_ue";
	if( argc < 3 )
	{
		std::cerr << "usage calculmoy coeff_file.csv notes.csv [outputfile]\n";
		return 1;
	}
	if( argc > 3 )
	{
		fout = argv[3];
	}

	if( argc > 1 )
	{
		for( int i=1; i<argc; i++ )
		{
			if( std::string(argv[i]) == "-t" )
				params.timestamp = true;
		}
	}


	auto listeMod = readCSV_coeff( std::string(argv[1]) );
	listeMod.print();
//	printCoeffs( ue_coeffs.second.v_liste );
	auto vnotes = readCSV_notes( std::string(argv[2]), listeMod, params );
	compute( listeMod, vnotes, params );


	printMoyennesCsv(  vnotes, listeMod, fout, params );
	printMoyennesHtml( vnotes, listeMod, fout, params );
	std::cout << "\nRésultats, voir fichier " << fout << '\n';
}

