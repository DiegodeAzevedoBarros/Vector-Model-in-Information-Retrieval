#include <iostream>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <cmath>
#include <limits.h>
#include <strstream>
#include <sstream>
#include <locale>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctime>

using namespace std;

//Hash dos documentos
//<termo <documento, frequencia>>
map<string, map<string, int> > hash_terms;

//Armazenar a norma dos termos para cada documento
map<string,long double> hash_norms;

//armazenar o nome dos documentos
vector<string>  name_docs;

//Armazenar a quantidade de documentos da colecao
int amountDocs = 0;

vector<string> stop_words;

struct Tupla {
	long double s;
	string d;
};

struct sim {
	string nome;
	long double sim;
};

std::string remove_letter_easy( std::string str, char c ) {
	str.erase( std::remove( str.begin(), str.end(), c ), str.end() ) ;
	return str ;
}

string clean_string(string s) {
	s = remove_letter_easy(s,'*');
	s = remove_letter_easy(s,'.');
	s = remove_letter_easy(s,',');
	s = remove_letter_easy(s,'!');
	s = remove_letter_easy(s,'\"');
	s = remove_letter_easy(s,'\'');
	s = remove_letter_easy(s,'(');
	s = remove_letter_easy(s,')');
	s = remove_letter_easy(s,'/');
	s = remove_letter_easy(s,'+');
	s = remove_letter_easy(s,'-');
	s = remove_letter_easy(s,'@');
	s = remove_letter_easy(s,'&');
	s = remove_letter_easy(s,'$');
	s = remove_letter_easy(s,'#');
	s = remove_letter_easy(s,'%');
	s = remove_letter_easy(s,'=');
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);

	return s;
}

vector<string> remove_stop_words(string line) {
	string token;
	vector<string> tokens;
	std::istringstream s(line);
	vector<string>::iterator it;
	bool name_doc = true;
	while (s >> token) {
		if(name_doc) {
			name_doc = false;
			name_docs.push_back(token);
			tokens.push_back(token);
		} else {
			token = clean_string(token);

			if(token.length() > 0) {
				it = find(stop_words.begin(), stop_words.end(), token) ;

				if (it == stop_words.end()) {
					tokens.push_back(token);
				}
			}
		}//fim do else;
	}//fim do while
	return tokens;
}

void calculateNorms(map<string, map<string, int>> hash_terms) {
	map<string, map<string, int> >::iterator it_terms;
	int amount_freq_term = 0;
	map<string, long double>::iterator it_norms;

	long double idf;
	long double weight;
	long double aux;

	/*Percorre todos os termos da base - vocabulary*/
	for (it_terms = hash_terms.begin(); it_terms != hash_terms.end() ; ++it_terms) {
		for(map<string, int>::iterator it = (it_terms->second).begin() ; it != (it_terms->second).end(); ++it){
			aux = ((long double)amountDocs / (long double) hash_terms[it_terms->first].size());
			idf = log10l(aux);
			weight = (long double)(it->second * idf);
			hash_norms[it->first] += pow(weight,2.0);
		}
	}
}

long double calculate_weight_query (string term ,string document) {

	long double aux = ((long double) amountDocs / (long double) hash_terms[term].size());
	long double idf = log10l(aux);
	long double weight = (long double)idf * (long double)hash_terms[term][document];
	return weight;
}

long double calculate_similarity(string query, int amount, int counter_file_query, int cont) {
	map<string, map<string, int>>::iterator it_term;
	map<string,int>::iterator it;
	map<string,int> words_key;
	map<string,int>::iterator wk;
	vector<string> query_words;
	vector<string>::iterator it_q_w;
	vector<string>::iterator it_name_docs;

	long double idf;
	long double weight_query;
	long double aux;
	long double norm = 0;

	vector<vector<long double>> rank_docks;
	vector<vector<long double>>::iterator it_rank_docks;
	Tupla t;
	vector<Tupla> tupla ;

	time_t start, stop;

	time(&start);
	query_words = remove_stop_words(query);

	if(query_words.size() > 0) {
		for (int i = 0; i < query_words.size(); i++) {
			it_term = hash_terms.find(query_words[i]);
			if (it_term != hash_terms.end()) {
				it = words_key.find(query_words[i]);
				if (it != words_key.end()) {
					words_key[query_words[i]]++;
				} else {
					words_key[query_words[i]] = 1;
				}
			}
		}
	}

	//calcula norma da consulta
	for (wk = words_key.begin(); wk != words_key.end() ; ++wk){

		aux = ((long double)amountDocs / (long double)hash_terms[wk->first].size());
		idf = log10l(aux);
		weight_query = (long double) (wk->second * idf);
		norm += pow(weight_query,2.0);
	}
	norm = sqrt(norm);

	map<string,long double> accumulators;
	for (int i = 0 ; i < name_docs.size(); i++) {
		string a = name_docs[i];
		accumulators[a] = 0.0;
	}

	for(wk = words_key.begin();wk != words_key.end(); ++wk) {

		for(map<string,int>::iterator auxIT = hash_terms[wk->first].begin() ; auxIT != hash_terms[wk->first].end(); auxIT++ ) {
			long double aux_idf = ((long double)amountDocs / (long double)hash_terms[wk->first].size());
			long double idf_q = log10l(aux_idf);
			long double weight_Q = (long double) (wk->second  * idf_q);
			accumulators[auxIT->first] = accumulators[auxIT->first] + weight_Q * calculate_weight_query(wk->first,auxIT->first);
		}
	}

	map<int, string> int_docs_names;
	for (int i = 0 ; i < name_docs.size(); i++) {
		int_docs_names[i] = name_docs[i];
	}

	vector<sim> sims (name_docs.size());
	for (int i = 0 ; i < name_docs.size(); i++) {
		sim aux;
		aux.nome = int_docs_names[i];
		aux.sim = (long double)( accumulators[int_docs_names[i]] / (long double)( hash_norms[int_docs_names[i]] * norm));
		sims[i] = aux;
	}

	//Ordena os documentos de acordo com a maior similaridades
	for (int i = 0 ; i < sims.size(); i++) {
		for (int j = i ; j < sims.size(); j++) {
			sim aux;
			if(sims[j].sim > sims[i].sim){
				aux = sims[i];
				sims[i] = sims[j];
				sims[j] = aux;
			}
		}
	}

	cout<<"similaridades para consulta da query: "<<counter_file_query<<endl;
	for (int i = 0 ; i < amount; i++) {
		cout << sims[i].nome <<  " --- "<< sims[i].sim << endl;
	}
	time(&stop);
	cout << "Teste executado em: " << difftime(stop, start) * 1000<< " milisegundos" << endl;

	/*verifica nos arquivos de relevantes,
	os documentos retornados no p@10 para uma data consulta,
	afim de verificar a precisão.
	*/
	int quant = 0;
	int conti = 1;
	long double precision;
	string t_line;
	ifstream f;
	char indice[20];
	char extension [20]= ".txt";
	char *name_temp_arq = extension;
	char indice_temp[50] = "./file_relevants/" ;
	char *indice_tmp = indice_temp;
	sprintf(indice, "%d", cont);

	indice_tmp = strcat(indice_tmp,indice);
	name_temp_arq = strcat(indice_tmp, name_temp_arq);

	f.open(name_temp_arq,ios::in | ios::binary);
	cout << "Arquivo: " << cont << endl;
	cout << "Nome do arquivo: " << name_temp_arq << endl;
	if(f == NULL){
		cerr << "Erro ao tentar abrir arquivo..." << ends;
	}

	while (f.eof() == false) {
		getline(f, t_line);
		for (int i = 0 ; i < amount; i++) {
			if(t_line == sims[i].nome){
				quant += 1;
			}
		}
	}

	f.close();
	precision = ((long double)quant / 10.0);
	cout << "Precision: " << precision << "\n" << endl;
	return precision;
	/*
	Utilizado para quando tiver o diretorio de imagens
	html das imagens
	ofstream file_out;
	file_out.open("/home/barros/workspace/RI_TRABALHO1/file_out.html");
	file_out << "<html><body>";
	for (int i =0 ; i<amount; i++) {
	file_out << "<img src=\"/home/barros/workspace/RI_TRABALHO1/colecaoDafitiPosthaus/" << sims[i].nome << "\" alt=\"Result\" style=\"width:230px;height:230px\">";
}
file_out << "</body></html>";
*/
}//fim da funcao similaridade

int main() {

	map<string, map<string, int>>::iterator it_term;

	vector<string>::iterator it_stop_words;

	vector<string> words;

	ifstream file_in, file_stop_word, file_query;

	string line,line_clean,word,query;

	int counter_file_query = 1;
	int cont = 1; //armazenar o numero do arquivo corrente a ser lido (arquivo de relevantes)

	long double value_precision = 0;
	long double value_map = 0;

	file_stop_word.open("./file_stop_words/file_stop_words.txt", ios::in | ios::binary);
	cout << "Carregando o arquivo de stop words...\n" << endl;
	while (file_stop_word.eof() == false) {
		file_stop_word >> word;
		stop_words.push_back(word);
	}

	file_in.open("./file_base/textDescDafitiPosthaus.txt", ios::in);
	cout << "Criando o vocabulario e a lista invertida...\n" << endl;
	while (file_in.eof() == false) {
		amountDocs++;
		getline(file_in, line);
		vector<string> words = remove_stop_words(line);

		for (int i = 1; i < words.size(); i++) {
			it_term = hash_terms.find(words[i]);
			if (it_term == hash_terms.end()) {
				hash_terms[words[i]] = map<string, int>();
				hash_terms[words[i]][words[0]] = 1;
			} else {
				if (hash_terms[words[i]][words[0]]) {
					hash_terms[words[i]][words[0]]++;
				} else {
					hash_terms[words[i]][words[0]] = 1;
				}
			}
		}//fim do for
	}//fim do while

	amountDocs = amountDocs -1;

	cout<<"#####Consultas#####\n";

	//processa todas as consultas armazenadas no arquivo
	file_query.open("./file_queries/queries.txt", ios::in | ios::binary);
	while (file_query.eof() == false){
		getline(file_query, query);
		if(!query.empty()){
			cout << "Query: " << query << endl;
			calculateNorms(hash_terms);
			value_precision += calculate_similarity(query,10,counter_file_query,cont);
			counter_file_query ++;
			cont ++;
		} else {
			break;
		}
	}

	cout<<"\nAvarage of Precision: "<<value_precision<<endl;
	value_map = (long double)((value_precision / 50) * 100);
	cout<<"\nValue of Map: "<<value_map<<endl;
	file_in.close();
	file_stop_word.close();
	file_query.close();
	return 0;
}
