#include "filesystem.h"
#include <iostream>
#include <iterator>
#include <regex>

struct node *cecko = createFile(TYPE_DIRECTORY, "C://", "C://", "");

struct node *createFile(int type, std::string actualDirectory, std::string path, std::string data) {
	node *newNode = new node;
	if (!newNode) return nullptr;

	std::vector<std::string> absolutePath;
	std::string absolutePathStr;
	if (path.substr(0, 4) == "C://") {
		absolutePathStr = path;
	} else {
		absolutePathStr = actualDirectory + (((path.substr(0,1) == "/") || (actualDirectory.length() == 4)) ? "" : "/") + path;
	}

	absolutePath = split_string(absolutePathStr);
	
	newNode->type = type;
	std::vector<std::string> name = split_string(path);
	newNode->name = name[name.size() - 1];
	newNode->parent = nullptr;
	newNode->path = absolutePathStr;
	newNode->data = data;

	if (path == "C://")  {
		newNode->name = "C://";
		return newNode;
	}
	
	struct node *temp = cecko;
	for (size_t i = 1; i < absolutePath.size(); i++)
	{
		for (size_t j = 0; j < temp->children.size(); j++)
		{
			if (absolutePath[i] == "..") {
				if (temp->parent == nullptr) {
					std::cout << "Path does not exist" << std::endl;
					return nullptr;
				} else {
					temp = temp->parent;
				}
			} else if (absolutePath[i] == temp->children[j]->name && newNode->name != temp->children[j]->name) {
				temp = temp->children[j];
			}
		}
	}

	addChild(&temp, &newNode);
	
	return newNode;
}

HRESULT addChild(struct node **parent, struct node **child) {
	if (!*parent || !*child) return S_FALSE;
	if ((*parent)->type == TYPE_FILE) return S_FALSE;

	for (size_t i = 0; i < (*parent)->children.size(); i++)
	{
		if ((*parent)->children[i]->name == (*child)->name)  {
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
		char* buffer = new char[6];
		getData(temp, 2, 6, &buffer);
		std::cout << buffer << std::endl;
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

HRESULT getData(struct node *file, size_t startPosition, size_t size, char** buffer) {
	if (!file) return S_FALSE;
	if (size <= 0) return S_FALSE;
	if (startPosition < 0 || startPosition > file->data.size()) return S_FALSE;
	for (size_t i = startPosition; i < startPosition + size; i++) {
		(*buffer)[i] = file->data.at(i);
	}
	return S_OK;
}