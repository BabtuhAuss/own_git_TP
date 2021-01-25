#include "dummy.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/uuid/detail/sha1.hpp>



// utilisée pour les tests
int GetAnswerToLifeUniverseAndEverything()
{
	return 42;
}

// cette fonction est dédiée à être éliminée -- seulement de test
void TestDeTrucs() throw(boost::filesystem::filesystem_error) // le throw ici est "deprecated". Il est juste conserver à titre de documentation
{
	/*
		Le but de la fonction est de présenter les techniques utiles pour la
		réalisation du travail
		Ceci inclus:
				l'utilisation de boost::filesystem
				la génération de sha1
				l'utilisation de zlib (de boost)

		Il manque beaucoup de trucs ici comme, par exemple, la gestion des erreurs
		Ce n'est pas le but de ce fichier de faire ce genre de détail
	*/

	using boost::uuids::detail::sha1;
	using std::ifstream;
	using std::string;

	const auto filePath = "truc.txt";

	// lecture du fichier
	ifstream file(filePath);	// pas besoin de gerer l'acces
								// mais il faut s'assurer que le fichier existe
	string content{	std::istreambuf_iterator<char>(file),
					std::istreambuf_iterator<char>() };

	//---------------------------------------------
	// le sha
	// https://pragmaticjoe.blogspot.com/2015/02/how-to-generate-sha1-hash-in-c.html
	sha1 sha;
	sha.process_bytes(content.c_str(), content.length());

	// bof... would prefer array...but the digest does not support it
	unsigned int hash[5];
	sha.get_digest(hash);

	std::stringstream stream;

	std::string result;
	// there is something missing here...but what?
	for (int i = 0; i < 5; ++i) {
		stream << std::hex << hash[i]; 
	}
	content += stream.str();

	//---------------------------------------------------------------
	// poutine syntaxique pour créer une compression en zlib
	// https://www.boost.org/doc/libs/1_71_0/libs/iostreams/doc/classes/zlib.html
	boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
	in.push(boost::iostreams::zlib_compressor());
	in.push(boost::make_iterator_range(content));
	std::string compressed;
	boost::iostreams::copy(in, boost::iostreams::back_inserter(compressed));

	//--------------------------------------------------------
	// filesystem
	const auto fileName = "testResultat.txt";

	auto path = boost::filesystem::current_path();	// ou sommes nous?
	const auto pathFolder = path.append("folder");	// ajouter un fichier de la manière décidée par la plateforme
													// comme ca, on aura jamais de problème de plateforme
	
	if (!boost::filesystem::exists(pathFolder))  // peut lancer une exception
	{
		// si le répertoire n'existe pas, on le crée
		boost::filesystem::create_directory(pathFolder); // exception possible
	}

	boost::system::error_code code;
	if (!boost::filesystem::exists(pathFolder, code)) // ne lance pas d'exception
	{
		if (code.failed())
		{
			std::cout << "Something bad happened but we didn't throw anything...so thats good :)" << std::endl;
		}
	}


	//---------------------------------------------------------
	// ofstream tout normal et gentil
	std::ofstream outputFile(path.append(fileName).c_str());

	// on écrit la compression, deux sauts de ligne et le contenu
	outputFile << compressed << std::endl << std::endl << content;

	// il faudrait valider l'écriture ici...

	outputFile.close(); // on aime etre propre alors on ferme le fichier
						// théoriquement, le RAII devrait le faire pour nous
}
