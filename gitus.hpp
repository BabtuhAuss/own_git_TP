#ifndef GITUS_H
#define GITUS_H
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <boost/lambda/lambda.hpp>
#include <stdlib.h>


//#include <boost/filesystem/fstream.hpp>
#include <iterator>
#include <algorithm>
#include <boost/algorithm/string.hpp>

#include <chrono>  // chrono::system_clock
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time

void init_cmd(void);
void add_file_cmd(boost::filesystem::path fichier);

void commit_cmd(std::string msg, std::string author);
int get_taille(std::string nomFichier);


std::ofstream create_object_file(std::string hash);
std::string get_hash(boost::filesystem::path fichier, std::string name_file);


std::string return_current_time_and_date(void);


void checkout_cmd(std::string hash_commit_dst);
#endif // GITUS_H
