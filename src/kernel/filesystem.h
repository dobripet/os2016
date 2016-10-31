#pragma once
#define TYPE_DIRECTORY 0
#define TYPE_FILE 1

#include <Windows.h>
#include <string>
#include <vector>
#include <algorithm>

typedef struct node {
	int type;
	std::string name;
	std::string path;
	std::string data; //TODO: data nebudou string, pøepsat na strukturu "data"
	std::vector<struct node*> children;
	struct node *parent;
} node;

struct node *createFile(int type, std::string actualDirectory, std::string path, std::string data);
HRESULT addChild(struct node **parent, struct node **child);
HRESULT deleteFile(std::string actualDirectory, std::string path);
HRESULT deleteFile(struct node *toDetele);
HRESULT getData(struct node *file, size_t startPosition, size_t size, char** buffer);
std::vector<std::string> split_string(std::string s);