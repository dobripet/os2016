#include "filesystem.h"
#include <iostream>
#include <iterator>
#include <regex>

struct node *cecko = mkdir(nullptr, "C://");

node *mkdir(struct node *currentDir, char *path) {
	struct node *newFile = new node;
	newFile->type = TYPE_DIRECTORY;
	newFile->name = "C:";
	newFile->parent = nullptr;
	return newFile;
}
node *getCecko() {
	return cecko;
}

node *getNodeFromPath(char *path) {
	return nullptr;
}

node *openFile(int type, char *path, bool rewrite, node *currentDir) {
	std::string pathString(path);
	std::vector<std::string> absolutePath;
	std::string absolutePathStr;
	struct node *temp;
	size_t i = 0;
	if (pathString.substr(0, 4) == "C://") {
		i = 1;
		temp = cecko;
	}
	else {
		temp = currentDir;
	}

	//temp = uzel, kde mam zaèít prohledávat zadanou cestu pathString
	absolutePath = split_string(pathString);
	struct node *walker;
	
	for (; i < absolutePath.size(); i++) //prolezení všech èástí [i] cesty
	{
		walker = temp;
		for (size_t j = 0; j < temp->children.size(); j++) //hledání potomka v aktuálním uzlu podle cesty [i]
		{
			if (absolutePath[i] == "..") { //skok o adresáø výše
				if (temp->parent == nullptr) {
					std::cout << "Path does not exist" << std::endl; //už jsme v céèku, výš skoèit nejde
					return nullptr;
				}
				else {
					temp = temp->parent;
				}
			}
			else if (absolutePath[i] == temp->children[j]->name) { //nalezli jsme správného potomka
				temp = temp->children[j];
				std::cout << i << " " << j << std::endl;
				break;
			}
		}
		if (walker == temp) { //potomek nebyl nalezen
			if (i + 1 == absolutePath.size()) {
				//vytvoø soubor
				struct node *newFile = new node;
				newFile->type = TYPE_DIRECTORY;
				newFile->name = absolutePath[absolutePath.size() - 1];
				newFile->parent = nullptr;
				addChild(&temp, &newFile);
				return newFile;
			}
			else {
				//cesta neexistuje
			}
			break;
		}
	}

	return temp;
}

HRESULT addChild(struct node **parent, struct node **child) {
	if (!*parent || !*child) return S_FALSE;
	if ((*parent)->type == TYPE_FILE) return S_FALSE;

	for (size_t i = 0; i < (*parent)->children.size(); i++)
	{
		if ((*parent)->children[i]->name == (*child)->name) {
			(*parent)->children[i]->data = (*child)->data; //TODO: zakomentovat, pokud nechcem pøepisovat data existujícího souboru, ale jenom oznámit, že soubor existuje a má smùlu
			std::cout << "File \"" << (*child)->name << "\" already exists. Data were overwritten." << std::endl;
			return S_FALSE;
		}
	}

	((*parent)->children).push_back(*child);
	(*child)->parent = *parent;

	return S_OK;
}

HRESULT deleteFile(std::string actualDirectory, std::string path) {
	std::vector<std::string> absolutePath;
	std::string absolutePathStr;
	if (path.substr(0, 4) == "C://") {
		absolutePathStr = path;
	}
	else {
		absolutePathStr = actualDirectory + (((path.substr(0, 1) == "/") || (actualDirectory.length() == 4)) ? "" : "/") + path;
	}

	absolutePath = split_string(absolutePathStr);

	std::vector<std::string> name = split_string(path);

	struct node *temp = cecko;
	for (size_t i = 1; i < absolutePath.size(); i++)
	{
		for (size_t j = 0; j < temp->children.size(); j++)
		{
			if (absolutePath[i] == "..") {
				if (temp->parent == nullptr) {
					std::cout << "Path does not exist" << std::endl;
					return S_FALSE;
				}
				else {
					temp = temp->parent;
				}
			}
			else if (absolutePath[i] == temp->children[j]->name) {
				temp = temp->children[j];
			}
		}
	}

	if (temp != cecko) {
		//testovací volání nastavení/získání dat na konkrétní pozici

		setData(&temp, 8, 5, "testy");
		char* buffer;
		getData(&temp, 0, 11, &buffer);
		std::cout << temp->data << " - " << buffer << std::endl;

		deleteFile(temp);
	}
	else {
		std::cout << "Path " << absolutePathStr << " does not exist" << std::endl;
	}

	return S_OK;
}

HRESULT deleteFile(struct node *toDelete) {

	struct node *parent = toDelete->parent;

	if (toDelete->type == TYPE_DIRECTORY) {
		if (!(toDelete->children).empty()) {
			size_t last = (toDelete->children).size();
			for (size_t i = 0; i < last; i++)
			{
				deleteFile(toDelete->children[0]);
			}
		}

		for (size_t i = 0; i < parent->children.size(); i++) {
			if (parent->children[i] == toDelete) {
				parent->children.erase(parent->children.begin() + i);
				std::cout << "Folder \"" << toDelete->name << "\" deleted." << std::endl;
				//todo: delete node (free memory)
				return S_OK;
			}
		}
		std::cout << "Folder \"" << toDelete->name << "\" wasn't found." << std::endl;

	}

	if (toDelete->type == TYPE_FILE) {
		for (size_t i = 0; i < parent->children.size(); i++)
		{
			if (parent->children[i] == toDelete) {

				parent->children.erase(parent->children.begin() + i);
				std::cout << "File \"" << toDelete->name << "\" deleted." << std::endl;
				//todo: delete node (free memory)
				return S_OK;
			}
		}
		std::cout << "File \"" << toDelete->name << "\" wasn't found." << std::endl;
	}

	return S_FALSE;
}


std::vector<std::string> split_string(std::string s) {
	std::vector<std::string> path;
	std::string rgx_str = "/+";
	std::regex rgx(rgx_str);
	std::sregex_token_iterator iter(s.begin(), s.end(), rgx, -1);
	std::sregex_token_iterator end;

	while (iter != end) {
		path.push_back(*iter);
		iter++;
	}
	return path;
}

HRESULT getData(struct node **file, size_t startPosition, int size, char** buffer) {
	if (!file) return S_FALSE;
	if (size <= 0) return S_FALSE;
	if (startPosition < 0 || startPosition >(*file)->data.size()) return S_FALSE;
	(*buffer) = (char*)malloc(sizeof(char) * (size + 1));
	for (size_t i = startPosition; i < startPosition + size; i++) {
		(*buffer)[i - startPosition] = (*file)->data.at(i);
	}
	(*buffer)[size] = '\0';
	return S_OK;
}

HRESULT setData(struct node **file, size_t startPosition, int size, char* buffer) {
	if (!file) return S_FALSE;
	if (size <= 0) return S_FALSE;
	if (startPosition < 0 || startPosition >(*file)->data.size()) return S_FALSE;

	if ((*file)->data.length() == startPosition) {
		(*file)->data.append(buffer);
	}
	else {
		for (size_t i = startPosition; i < startPosition + size && i < (*file)->data.length(); i++) {
			(*file)->data[i] = buffer[i - startPosition];
		}

		if ((*file)->data.length() < startPosition + size) {
			int fileSize = (*file)->data.length();
			//doøešit
			(*file)->data.append((buffer + (size - (startPosition - fileSize))));
		}
	}

	return S_OK;
}