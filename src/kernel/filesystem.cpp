#include "filesystem.h"
#include <iostream>
#include <iterator>
#include <regex>
#include <cctype>
#include <mutex>


std::mutex FS_mtx;

bool char_equals_lowerCase(unsigned char a, unsigned char b) {
	return std::tolower(a) == std::tolower(b);
}
bool str_equals(std::string const& a, std::string const& b) {
	return a.length() == b.length() && std::equal(b.begin(), b.end(),a.begin(), char_equals_lowerCase);
}

node *mkroot() {
	struct node *newFile = new node;
	newFile->type = TYPE_DIRECTORY;
	newFile->name = "C:";
	newFile->parent = nullptr;
	return newFile;
}

struct node *root = mkroot();

HRESULT mkdir(node **dir, char *path, node *currentDir) {
	std::lock_guard<std::mutex> lock(FS_mtx);
	node *parent;
	getNodeFromPath(path, false, currentDir, &parent);
	(*dir) = nullptr;

	if (parent != nullptr) {
		std::string pathString(path);
		std::vector<std::string> absolutePath = split_string(pathString);
		if (absolutePath[absolutePath.size() - 1] == ".." || absolutePath[absolutePath.size() - 1] == ".") { //cesta pro vytvoreni slozky nemuze koncit ".." nebo "."
			SetLastError(ERR_IO_PATH_NOEXIST);
			return S_FALSE;
		}

		for (size_t j = 0; j < parent->children.size(); j++) //hledani potomka v aktualnim uzlu podle nazvu
		{
			if (absolutePath[absolutePath.size() - 1] == parent->children[j]->name) {
				SetLastError(ERR_IO_FOLDER_EXISTS);
				return S_FALSE;
			}
		}
		
		struct node *newFile = new node;
		newFile->name = absolutePath[absolutePath.size() - 1];
		newFile->type = TYPE_DIRECTORY;
		newFile->parent = parent;
		addChild(&parent, &newFile);
		(*dir) = newFile;
		return S_OK;
	}

	return S_FALSE;
}

node *getRoot() {
	return root;
}

HRESULT getNodeFromPath(char *path, bool last, node *currentDir, node **node) {
	std::string pathString(path);
	std::vector<std::string> absolutePath;
	std::string absolutePathStr;
	struct node *temp;
	size_t i = 0;
	(*node) = nullptr;
	//temp = nastaveni zacatku prohledavani adresarove struktury (absolutni/relativni cesta)
	if (str_equals(pathString.substr(0, 4), "C://")) {
		i = 1;
		temp = root;
	}
	else {
		temp = currentDir;
	}
	
	absolutePath = split_string(pathString);
	struct node *walker;

	for (; i < absolutePath.size() - (last ? 0 : 1); i++)
	{
		if (absolutePath[i] == ".") {
			continue;
		}

		walker = temp;
		if (absolutePath[i] == "..") { //skok o adresar vyse
			if (temp->parent == nullptr) {
				SetLastError(ERR_IO_PATH_NOEXIST);
				return S_FALSE;
			}
			else {
				temp = temp->parent;
			}
		}

		for (size_t j = 0; j < temp->children.size(); j++) 
		{
			if (absolutePath[i] == temp->children[j]->name) { //nalezli jsme spravneho potomka
				if (i == (absolutePath.size() - 1) || temp->children[j]->type == TYPE_DIRECTORY) { //pouze posledni nemusi byt slozka
					temp = temp->children[j];
					break;
				}
			} 
		}
		if (walker == temp) { //potomek nebyl nalezen
			SetLastError(ERR_IO_PATH_NOEXIST);
			return S_FALSE;
		}
	}

	(*node) = temp;
	return S_OK;
}

HRESULT getPathFromNode(node *currentDir, std::string *path) {
	if (currentDir == nullptr) return S_FALSE;
	std::vector<std::string> pathStr;
	std::string absolutePath("C://");
	node *temp = currentDir;
	while (temp->parent != nullptr) {
		pathStr.push_back(temp->name);
		temp = temp->parent;
	}
	for (int i = (int)pathStr.size() - 1; i >= 0; i--) {
		absolutePath += pathStr[i] + "/";
	}
	(*path) = absolutePath;
	return S_OK;
}

HRESULT openFile(node **file, char *path, bool rewrite, bool create, node *currentDir) {
	std::lock_guard<std::mutex> lock(FS_mtx);
	node *parent;
	getNodeFromPath(path, false, currentDir, &parent);
	(*file) = nullptr;

	if (parent != nullptr) {
		std::string pathString(path);
		std::vector<std::string> absolutePath = split_string(pathString);
		if (absolutePath[absolutePath.size() - 1] == ".." || absolutePath[absolutePath.size() - 1] == ".") { //cesta k souboru nesmi koncit "." nebo ".."
			SetLastError(ERR_IO_FILE_ISNOTFILE);
			return S_FALSE;
		}
		for (size_t j = 0; j < parent->children.size(); j++)
		{
			
			if (absolutePath[absolutePath.size() - 1] == parent->children[j]->name) { 
				if (parent->children[j]->type == TYPE_FILE) { //nalazeny potomek muze pouze typu soubor
					if (rewrite) {
						//chceme prepsat data
						parent->children[j]->data.clear();
					}
					(*file) = parent->children[j];
					return S_OK;
				} else { 
					SetLastError(ERR_IO_FILE_ISNOTFILE);
					return S_FALSE;
				}
			}
		}

		if (create) {
			//nasli jsme soubor a chceme ho vytvorit
			struct node *newFile = new node();
			newFile->name = absolutePath[absolutePath.size() - 1];
			newFile->type = TYPE_FILE;
			newFile->parent = parent;
			addChild(&parent, &newFile);
			(*file) = newFile;
			return S_OK;
		}
		else {
			//nenasli jsme soubor a ani ho nechceme vytvorit
			SetLastError(ERR_IO_PATH_NOEXIST);
			return S_FALSE;
		}
	}

	//cesta neexistuje
	SetLastError(ERR_IO_PATH_NOEXIST);
	return S_FALSE;
}

HRESULT addChild(struct node **parent, struct node **child) {

	if (!*parent || !*child) return S_FALSE;
	if ((*parent)->type == TYPE_FILE) return S_FALSE;

	((*parent)->children).push_back(*child);
	(*child)->parent = *parent;

	return S_OK;
}

HRESULT deleteNode(struct node *toDelete) {

	std::lock_guard<std::mutex> lock(FS_mtx);
	struct node *parent = toDelete->parent;

	if (toDelete->type == TYPE_DIRECTORY) {
		if (!(toDelete->children).empty()) {
			SetLastError(ERR_IO_FILE_NOTEMPTY);
			return S_FALSE;
		}

		for (size_t i = 0; i < parent->children.size(); i++) {
			if (parent->children[i] == toDelete) {
				parent->children.erase(parent->children.begin() + i);
				return S_OK;
			}
		}
	}

	if (toDelete->type == TYPE_FILE) {
		for (size_t i = 0; i < parent->children.size(); i++)
		{
			if (parent->children[i] == toDelete) {

				parent->children.erase(parent->children.begin() + i);
				return S_OK;
			}
		}
	}

	SetLastError(ERR_IO_PATH_NOEXIST);
	return S_FALSE;
}


std::vector<std::string> split_string(std::string s) {
	std::vector<std::string> path;
	std::string rgx_str = "[\\\\/]+";
	std::regex rgx(rgx_str);
	std::sregex_token_iterator iter(s.begin(), s.end(), rgx, -1);
	std::sregex_token_iterator end;

	while (iter != end) {
		path.push_back(*iter);
		iter++;
	}
	return path;
}

HRESULT getData(struct node **file, size_t startPosition, size_t size, char** buffer, size_t *filled) {
	std::lock_guard<std::mutex> lock(FS_mtx);
	*filled = 0;
	if (!file) return S_FALSE; 
	if (size <= 0) return S_FALSE;
	if ((*file)->type == TYPE_DIRECTORY) return S_FALSE;
	if (startPosition < 0 || startPosition >(*file)->data.size()) return S_FALSE;
	
	for (size_t i = startPosition; i < startPosition + size && i < (*file)->data.length(); i++) {
		(*buffer)[i - startPosition] = (*file)->data.at(i);
		(*filled)++;
	}
	if (*filled != size) {
		(*buffer)[*filled] = EOF;
	}
	return S_OK;
}

HRESULT setData(struct node **file, char* buffer, size_t write) {

	std::lock_guard<std::mutex> lock(FS_mtx);

	if ((*file)->type == TYPE_DIRECTORY) {
		SetLastError(ERR_IO_FILE_ISNOTFOLDER);
		return S_FALSE;
	}
	std::string tw("");
	for (size_t w = 0; w < write; w++) {
		tw += buffer[w];
	}

	(*file)->data.append(tw);
	
	return S_OK;
}