#include "filesystem.h"
#include <iostream>
#include <iterator>
#include <regex>

node *mkroot() {
	struct node *newFile = new node;
	newFile->type = TYPE_DIRECTORY;
	newFile->name = "C:";
	newFile->parent = nullptr;
	return newFile;
}

struct node *root = mkroot();

HRESULT mkdir(node **dir, char *path, node *currentDir) {
	node *parent;
	getNodeFromPath(path, false, currentDir, &parent);

	if (parent != nullptr) {
		std::string pathString(path);
		std::vector<std::string> absolutePath = split_string(pathString);
		for (size_t j = 0; j < parent->children.size(); j++) //hled�n� potomka v aktu�ln�m uzlu podle cesty [i]
		{
			if (absolutePath[absolutePath.size() - 1] == "..") { //skok o adres�� v��e
				std::cout << "Path does not exist" << std::endl; //bul�it cesta, nem��em kon�it ..
				(*dir) = nullptr;
				return S_FALSE;
			}
			else if (absolutePath[absolutePath.size() - 1] == parent->children[j]->name) {
				//slo�ka existuje
				(*dir) = nullptr;
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

	(*dir) = nullptr;
	return S_FALSE;
}

node *getRoot() {
	return root;
}

HRESULT getNodeFromPath(char *path, node *currentDir, node **file) {
	struct node *parent;
	getNodeFromPath(path, false, currentDir, &parent);
	if (parent != nullptr) {
		std::string pathString(path);
		std::vector<std::string> absolutePath = split_string(pathString);
		for (size_t j = 0; j < parent->children.size(); j++) //hled�n� souboru v aktu�ln�m uzlu
		{
			
			if (absolutePath[absolutePath.size() - 1] == "..") { //posledn� kus cesty nem��e b�t skok o �rove� v��
				std::cout << "Path does not exist" << std::endl;
				(*file) = nullptr;
				//SetLastError(ERR_IO_PATH_NOEXIST);
				return S_FALSE;
			}
			
			else if (absolutePath[absolutePath.size() - 1] == parent->children[j]->name) { //nalezli jsme spr�vn�ho potomka
				(*file) = parent->children[j];
				return S_OK;
			}
		}
	}

	(*file) = nullptr;
	return S_FALSE;
}

HRESULT getNodeFromPath(char *path, bool last, node *currentDir, node **node) {
	std::string pathString(path);
	std::vector<std::string> absolutePath;
	std::string absolutePathStr;
	struct node *temp;
	size_t i = 0;
	(*node) = nullptr;
	if (pathString.substr(0, 4) == "C://") {
		i = 1;
		temp = root;
	}
	else {
		temp = currentDir;
	}

	//temp = uzel, kde mam za��t prohled�vat zadanou cestu pathString
	absolutePath = split_string(pathString);
	struct node *walker;

	for (; i < absolutePath.size() - (last ? 0 : 1); i++) //prolezen� v�ech ��st� [i] cesty
	{
		walker = temp;
		if (absolutePath[i] == "..") { //skok o adres�� v��e
			if (temp->parent == nullptr) {
				std::cout << "Path does not exist" << std::endl; //u� jsme v c��ku, v�� sko�it nejde
				SetLastError(ERR_IO_PATH_NOEXIST); //TODO: upravit tuhle chybu
				return S_FALSE;
			}
			else {
				temp = temp->parent;
			}
		}

		for (size_t j = 0; j < temp->children.size(); j++) //hled�n� potomka v aktu�ln�m uzlu podle cesty [i]
		{
			if (absolutePath[i] == temp->children[j]->name && temp->children[j]->type == TYPE_DIRECTORY) { //nalezli jsme spr�vn�ho potomka
				temp = temp->children[j];
				break;
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
	node *parent;
	getNodeFromPath(path, false, currentDir, &parent);
	(*file) = nullptr;

	if (parent != nullptr) {
		std::string pathString(path);
		std::vector<std::string> absolutePath = split_string(pathString);
		for (size_t j = 0; j < parent->children.size(); j++) //hled�n� souboru v aktu�ln�m uzlu
		{
			if (absolutePath[absolutePath.size() - 1] == "..") { //posledn� kus cesty nem��e b�t skok o �rove� v��
				std::cout << "Path does not exist" << std::endl;
				SetLastError(ERR_IO_FILE_ISFOLDER); //TODO: mo�n� existuje p��pad, kdy to soubor neni, ale prost� neexistuje cesta?
				return S_FALSE;
			}
			else if (absolutePath[absolutePath.size() - 1] == parent->children[j]->name && parent->children[j]->type == TYPE_FILE) { //nalezli jsme spr�vn�ho potomka
				//soubor existuje
				if (rewrite) {
					//chceme p�epsat data
					parent->children[j]->data.clear();
				}
				(*file) = parent->children[j];
				return S_OK;
			}
		}

		if (create) {
			//na�li jsme soubor a chceme ho vytvo�it
			struct node *newFile = new node;
			newFile->name = absolutePath[absolutePath.size() - 1];
			newFile->type = TYPE_FILE;
			newFile->parent = parent;
			addChild(&parent, &newFile);
			(*file) = newFile;
			return S_OK;
		}
		else {
			//nena�li jsme soubor (nebo cesta ukazovala na slo�ku) a ani ho nechceme vytvo�it
			SetLastError(ERR_IO_PATH_NOEXIST);
			return S_FALSE;
		}
	}

	//cesta neexistuje
	(*file) = nullptr;
	return S_FALSE;
}

HRESULT addChild(struct node **parent, struct node **child) {
	if (!*parent || !*child) return S_FALSE;
	if ((*parent)->type == TYPE_FILE) return S_FALSE;


	//asi redundantn� k�d?? �e�� nejsp� u� openFile
	for (size_t i = 0; i < (*parent)->children.size(); i++)
	{
		if ((*parent)->children[i]->name == (*child)->name) {
			(*parent)->children[i]->data = (*child)->data; //TODO: zakomentovat, pokud nechcem p�episovat data existuj�c�ho souboru, ale jenom ozn�mit, �e soubor existuje a m� sm�lu
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

	struct node *temp = root;
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

	if (temp != root) {
		deleteNode(temp);
	}
	else {
		std::cout << "Path " << absolutePathStr << " does not exist" << std::endl;
	}

	return S_OK;
}

HRESULT deleteNode(struct node *toDelete) {

	struct node *parent = toDelete->parent;

	if (toDelete->type == TYPE_DIRECTORY) {
		if (!(toDelete->children).empty()) {
			return S_FALSE;
			/* //rekurzivn� maz�n�
			size_t	 = (toDelete->children).size();
			for (size_t i = 0; i < last; i++)
			{
				deleteFile(toDelete->children[0]);
			}
			*/
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

HRESULT getData(struct node **file, size_t startPosition, size_t size, char** buffer, size_t *filled) {
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
		(*buffer)[*filled] = '\0';
	}
	return S_OK;
}

HRESULT setData(struct node **file, size_t startPosition, size_t size, char* buffer) {
	if (!file) return S_FALSE;
	if (size <= 0) return S_FALSE;
	if ((*file)->type == TYPE_DIRECTORY) return S_FALSE;
	if (startPosition < 0 || startPosition >(*file)->data.size()) return S_FALSE;

	if ((*file)->data.length() == startPosition) {
		(*file)->data.append(buffer);
	}
	else {
		for (size_t i = startPosition; i < startPosition + size && i < (*file)->data.length(); i++) {
			(*file)->data[i] = buffer[i - startPosition];
		}

		if ((*file)->data.length() < startPosition + size) {
			size_t fileSize = (*file)->data.length();
			//do�e�it
			(*file)->data.append((buffer + (size - (startPosition - fileSize))));
		}
	}

	return S_OK;
}