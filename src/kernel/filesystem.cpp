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
	for (size_t i = 2; i < absolutePath.size(); i++)
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
	for (size_t i = 2; i < absolutePath.size(); i++)
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
	struct node *parent = temp->parent;

	if (temp->type == TYPE_DIRECTORY) {
		if ((temp->children).size() > 0) {
			//TODO: co udìlat, když složka neni prázdná? smazat na základì nìjakých práv, nebo nedovolit vùbec?
		}
		else {
			for (size_t i = 0; i < parent->children.size(); i++)
			{
				if (parent->children[i] == temp) {

					parent->children.erase(parent->children.begin() + i);
					std::cout << "Folder \"" << temp->name << "\" deleted." << std::endl;
					//todo: delete node (free memory)
					return S_OK;
				}
			}
			std::cout << "Folder \"" << temp->name << "\" wasn't found." << std::endl;
		}
	}

	if (temp->type == TYPE_FILE) {
		for (size_t i = 0; i < parent->children.size(); i++)
		{
			if (parent->children[i] == temp) {

				parent->children.erase(parent->children.begin() + i);
				std::cout << "File \"" << temp->name << "\" deleted." << std::endl;
				//todo: delete node (free memory)
				return S_OK;
			}
		}
		std::cout << "File \"" << temp->name << "\" wasn't found." << std::endl;
	}
	
	return S_FALSE;
}

std::vector<std::string> split_string(std::string s) {
	std::vector<std::string> path;
	std::string rgx_str = "/";
	std::regex rgx(rgx_str);
	std::sregex_token_iterator iter(s.begin(), s.end(), rgx, -1);
	std::sregex_token_iterator end;

	while (iter != end) {
		path.push_back(*iter);
		iter++;
	}
	return path;
}