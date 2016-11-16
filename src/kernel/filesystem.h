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
	std::string data; //TODO: data nebudou string, pøepsat na strukturu "data"
	std::vector<struct node*> children;
	struct node *parent;
} node;

node *getRoot();

HRESULT mkdir(node **dir, char *path, node *currentDir);
HRESULT getNodeFromPath(char *path, bool last, node *currentDir, node **node);
HRESULT getPathFromNode(node *currentDir, std::string *path);
HRESULT openFile(node **file, char *path, bool rewrite, bool create, node *node);
HRESULT addChild(node **parent, struct node **child);
HRESULT deleteFile(std::string actualDirectory, std::string path);
HRESULT deleteFile(node *toDetele);
HRESULT getData(node **file, size_t startPosition, size_t size, char** buffer, size_t *filled);
HRESULT setData(node **file, size_t startPosition, size_t size, char* buffer);
std::vector<std::string> split_string(std::string s);