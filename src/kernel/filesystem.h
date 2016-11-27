#pragma once

#include <Windows.h>
#include "../common/api.h"
#include "kernel.h"
#include <string>
#include <vector>
#include <algorithm>

//struktura pro ulozeni souboru - uzel ve stromu
typedef struct node {
	int type; //slozka nebo soubor
	std::string name; //nazev
	std::string data; //data
	std::vector<struct node*> children; //vektor potomku ve stromu
	struct node *parent; //rodic ve stromu
} node;

//vraci korenovy uzel filesystemu (C://)
node *getRoot();

//vytvori slozku a prida ji na pozadovanou adresu do adresarove struktury
HRESULT mkdir(node **dir, char *path, node *currentDir);

//vraci odkaz na uzel (soubor, nebo slozka) ze zadane adresy (relativni i absolutni) a aktualniho pracovniho adresare
HRESULT getNodeFromPath(char *path, bool last, node *currentDir, node **node);

//vraci absolutni cestu pro zadany (existujici) uzel v adresarove strukture
HRESULT getPathFromNode(node *currentDir, std::string *path);

//vraci odkaz na soubor, ktery chceme otevrit pro cteni/zapis (u slozky pro nastaveni aktualniho adresare nejakeho procesu)
HRESULT openFile(node **file, char *path, bool rewrite, bool create, node *node);

//pridava uzel do adresarove struktury
HRESULT addChild(node **parent, struct node **child);

//smaze uzel z adresarove struktury
HRESULT deleteNode(node *toDelete);

//vraci pozadovana data ze zadaneho uzlu
HRESULT getData(node **file, size_t startPosition, size_t size, char** buffer, size_t *filled);

//zapise data do pozadovaneho souboru
HRESULT setData(node **file, char* buffer, size_t write);

//rozdeluje zadanou cestu na tokeny
std::vector<std::string> split_string(std::string s);