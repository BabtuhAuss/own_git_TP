#include "gitus.hpp"


//stdchrono
//#include <dummy.h>


using namespace std;
using namespace boost::lambda;
using namespace boost::filesystem;
using namespace boost::algorithm;

//Pollution du namespaceglobal: -5% (IGL601)◦Exemple: Utilisation de using namespace std; WHY ??

// https://git-scm.com/book/en/v2/Git-Internals-Git-Objects
// https://git-scm.com/docs/index-format
int main(int argc, char* argv[])
{
	//cout << argv[1] << endl;
	
	// deux facons pour comparer une chaine de caractere : 
	// - utiliser la library boost avec iquals(str1,str2)
	// - le classique == avec string("str"), cast obligatoire en string argv[1] ne suffit pas :(
	
	if(argc == 1 || iequals(argv[1],"--help")) {
		cout << "usage: gitus <command> |<args>| \n" << endl;
		cout << "These are common gitus commands used in various situations \n" << endl;
		cout << "init \t Create an empty Git repository or reinitialize an existing one \n" << endl;
		cout << "add \t Add file contents to the index\n"  << endl;	
		cout << "commit \t Record changes to the repository" << endl;
	}
	else if(argc > 1) {
		if(argv[1] == string("init")) {
			//cout << argc << endl;
			init_cmd();
			cout << "ajout des fichiers terminé" << endl;
			
		}
		else if(argv[1] == string("add")) {
			if(argc == 2 || argv[2] == string("--help")) {
				cout << "usage: gitus add <pathspec>" << endl;
			}
			else if(argc == 3) { // on verifie bien qu'il n'y a que deux arguments envoye
				
				cout << "nom du fichier : " << argv[2] << "\n" << endl;
				path fichier(argv[2]);
				if(exists(fichier)){ // est ce que le chemin du fichier existe vrmt ?
					if(is_regular_file(fichier)) {
						add_file_cmd(fichier);
					}
					else {
						cout << "[ERROR] Only files are allowed \n" << endl;
					}
				}
				else {
					cout << "[ERROR] File does not exists" << endl;
				}
			} 
		}
		
		//commit 
		else if(argv[1] == string("commit")) {
			if(argc == 2 || argv[2] == string("--help")) {
				cout << "usage: gitus commit <msg> <author>" << endl;
			}
			else if(argc != 4) { // on verifie bien qu'il n'y a que deux arguments envoye
				cout << "[ERROR] Not enough parameters, try \"gitus commit <msg> <author>\"" << endl;
			} 
			else{
				std::string msg(argv[2]), author(argv[3]);
				commit_cmd(msg,author);
			}
		}
		else if(argv[1] == string("checkout"))
		{
			if(argc != 3)
			{
				cout << "[ERROR] wrong usage of command, try \"./gitus checkout <hash_commit>\" to go to the specific commit" << endl;
			}
			else
			{
				checkout_cmd(argv[2]);
			}
		}
		
	}
	return 0;
}


void init_cmd()
{
	cout << "ajout des fichiers en cours" << endl ;
	
	std::string folder = ".git/objects";

	boost::filesystem::path path(folder);
	
	boost::filesystem::create_directories(path);

	std::ofstream index(".git/index" ,ios::app); 
	std::ofstream head(".git/HEAD" ,ios::app); 
	return;
}

void add_file_cmd(boost::filesystem::path fichier){
	//cout << "regular file : ok" << endl;

	//git hash-object -w FILE -> return the key that git would use and stores it in .git/objects 

	//pour la generation des objets
	//0 générer le hash de l'objet avec git hash-object et mettre dans index
	std::string file_name(fichier.string().c_str());

	std::string hash(get_hash(fichier,"index"));
	std::string temp;
	
	cout << "hash file : " << hash << endl;

	//1 . créer un dossier avec les deux premières lettres du hash
	std::ofstream object_file(create_object_file(hash));
		
	//3 à l'interieur, ajouter les infos
	std::string content(fichier.string()
	+ " "
	+ std::to_string(get_taille(file_name))
	+ "\n"
	);
	
	std::ifstream file_read(file_name);
	while (std::getline(file_read, temp)) {
		content += temp + "\n";
	}
	file_read.close();

	object_file << content << endl;	
}


int get_taille(string nomFichier)
{
	std::ifstream fichier(nomFichier); //On ouvre le fichier
	fichier.seekg(0, ios::end);           //On se déplace à la fin du fichier

	int taille;
	taille = fichier.tellg();
	//On récupère la position qui correspond donc a la taille du fichier ! merci Openclassroom
	return taille;
}


//Info a ajouter dans le read me : le sha 1 de boost n'est pas installé par défault, donc on créer un fichier temp le temps de faire le hash de git hash-object pour avoir un résultat cohérent.

void commit_cmd(std::string msg, std::string author)
{
	
	//1 récupère tous le contenu d'index 
	std::ofstream file_write;
	std::ifstream file_read;
	std::ifstream index(".git/index");
	std::string tree_file_content,temp,tree_hash, commit_file_content, commit_hash, cmd, head_hash_parent_commit;
	
	
	//utile pour récupérer les hashs
	boost::filesystem::path path_index(".git/index");
	file_read.open(".git/index");
	if(get_taille(".git/index") == 0){
		cout << "nothing to commit, try adding some files" << endl; 
		return;
	}
	while(std::getline(file_read,temp))
	{
		tree_file_content+=temp + "\n" ;
	}
	tree_hash = get_hash(path_index, "temp");
	cout << "hash tree : " << tree_hash << endl;
	file_read.close();
	
	
	//2 . création de l'arbre (tree) 
	std::ofstream tree_object(
		create_object_file(tree_hash)
	);
	tree_object << tree_file_content;
	tree_object.close();
	
	//3 . créer le commit object
	//pour connaitre le parent, voir le hash contenu dans head
	std::ifstream head_file_read(".git/HEAD");
	
	file_read.open(".git/HEAD");
	std::getline(file_read, head_hash_parent_commit);
	file_read.close();
	
	
	commit_file_content+="tree\t" + string(tree_hash)+"\n"
	+ "parent\t"+std::string(head_hash_parent_commit) +"\n"
	+ "date\t" + return_current_time_and_date() + "\n"
	+ "msg\t" + string(msg) +"\n"
	+ "author\t" + string(author) +"\n";


	cmd = "echo \"" + commit_file_content + "\" | git hash-object --stdin > .git/temp";
	std::system(cmd.c_str());
	
	//std::ifstream temp_file(".git/temp");
	file_read.open(".git/temp");
	std::getline(file_read,commit_hash);
	file_read.close();
	
	cout << "hash commit : " << commit_hash << endl;

	//on supprime le contenu de temp
	std::ofstream temp_file(".git/temp",std::ofstream::trunc);
	temp_file.close();
	
	std::ofstream commit_object(
		create_object_file(commit_hash)
	);
	commit_object << commit_file_content;
	commit_object.close();
	
	//supprimer ce qu'il y a dans .git/index, et remplacer .git/HEAD par le commit_hash (hashé)

	std::ofstream index_w(".git/index",std::ofstream::trunc);
	index_w.close();
	
	std::ofstream head(".git/HEAD", std::ofstream::trunc);
	head << commit_hash;
	head.close();
	
	return;
}

std::string return_current_time_and_date()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

//créer l'objet dans la base de donnnées .git/objects que ce soit un tree ou un add
std::ofstream create_object_file(std::string hash)
{
	//1 . créer un dossier avec les deux premières lettres du hash
	std::string path_file = ".git/objects/"+hash.substr(0,2);
	boost::filesystem::path path(path_file);
	boost::filesystem::create_directory(path);

	//2 . créer un fichier dans le dossier, avec le reste du hash
	path_file = path_file+"/"+hash.substr(2,hash.size()-1);
	std::ofstream object_file(path_file);
	
	return object_file;
}

std::string get_hash(boost::filesystem::path fichier, std::string name_file)
{
	std::string cmd = "git hash-object " + std::string(fichier.string().c_str()) + " >> .git/"+name_file;
	std::string file_name(fichier.string().c_str());
	
	std::system(cmd.c_str());

	std::ifstream file_read(".git/"+name_file); 
	
	std::string hash;
	std::string temp;
	while (std::getline(file_read, temp)) {
		  if(!file_read.eof()){
			hash = temp;
		}
	}
	file_read.close();
	if(name_file != string("index"))
	{
		std::ofstream temp_file(".git/"+name_file, std::ofstream::trunc);
		temp_file.close();
	}
	
	return hash;
}


void checkout_cmd(std::string hash_commit_dst)
{
	//1 on récupère l'objet commit (avec le hash, 2 premiers chars = dossier, le reste = fichier;
	std::string commit_folder_name(hash_commit_dst.substr(0,2)),
	 commit_file_name(hash_commit_dst.substr(2,hash_commit_dst.size()-1)),
	 tree_hash, tree_folder_name, tree_file_name,
	 added_file_hash, added_file_name, added_folder_name,
	 meta_content;
	
	std::ifstream file_reading, added_file_in;
	
	std::ofstream file_output;
	
	//contient les informations du fichier (nom, taille, contenu)
	std::vector<std::string> meta_info_file;
	
	
	file_reading.open(".git/objects/" + commit_folder_name + "/" + commit_file_name);
	
	//2 on retrouve l'arborescence généré avec les ajouts
	std::getline(file_reading, tree_hash);
	file_reading.close();
	tree_hash = tree_hash.substr(tree_hash.find("\t")+1, tree_hash.size()-1);
	
	
	
	//3 pour chaque ligne de l'objet tree 
		tree_folder_name = tree_hash.substr(0,2);
		tree_file_name = tree_hash.substr(2,tree_hash.size()-1);
		
		file_reading.open(".git/objects/" + tree_folder_name + "/" + tree_file_name);
		// 3.1 si le fichier est ajouté après le commit, on le supprime,
		while(std::getline(file_reading, added_file_hash))
		{
		
			cout << added_file_hash << endl;
			//on retrouve le fichier
			
			added_folder_name = added_file_hash.substr(0,2);
			added_file_name = added_file_hash.substr(2,tree_hash.size()-1);
			
			//on lit les informations
			added_file_in.open(".git/objects/"+added_folder_name+"/"+added_file_name);
			std::getline(added_file_in, meta_content);
		
			//on réecrit dans le fichier
			boost::split(meta_info_file, meta_content, boost::is_any_of(" ")); 
			cout << meta_info_file[0] << endl;
			file_output.open(meta_info_file[0], std::ofstream::trunc);
			
			
			//lecture maintenant du contenu de texte
			while(std::getline(added_file_in, meta_content))
			{
				cout << meta_content << endl;
				file_output << meta_content << endl;
			}
			added_file_in.close();
			file_output.close();

		}
		file_reading.close();
		
		//TODO réflechir a une possible fonction pour renvoyer un tableau qui a comme premier argument folder , et en deuxieme file
	
}

