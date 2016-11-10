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
	std::string data; //TODO: data nebudou string, p�epsat na strukturu "data"
	std::vector<struct node*> children;
	struct node *parent;
} node;

node *getCecko();
node *openFile(int type, char *path, bool rewrite, node *node);
struct node *findFile(struct node *currentDir, char *path, char **name);
node *createFile(int type, std::string actualDirectory, std::string path, std::string data);
HRESULT addChild(node **parent, struct node **child);
HRESULT deleteFile(std::string actualDirectory, std::string path);
HRESULT deleteFile(node *toDetele);
HRESULT getData(node **file, size_t startPosition, int size, char** buffer);
HRESULT setData(node **file, size_t startPosition, int size, char* buffer);
std::vector<std::string> split_string(std::string s);