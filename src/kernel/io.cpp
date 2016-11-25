#include "io.h"
#include "kernel.h"
#include "process.h"

#include <iostream>

std::mutex files_table_mtx;
opened_file * opened_files_table[OPEN_FILES_TABLE_SIZE] = { nullptr };
opened_file_instance * opened_files_table_instances[OPEN_INSTANCES_TABLE_SIZE] = { nullptr };


void initSystemIO() {

	/*
	system in
	*/
	opened_files_table[0] = new opened_file();
	opened_files_table[0]->std = GetStdHandle(STD_INPUT_HANDLE);
	opened_files_table[0]->openCount = 1;
	opened_files_table[0]->FILE_TYPE = F_TYPE_STD;

	opened_files_table_instances[0] = new opened_file_instance();
	opened_files_table_instances[0]->file = 0;
	opened_files_table_instances[0]->mode = F_MODE_READ;

	/*
	system out
	*/
	opened_files_table[1] = new opened_file();
	opened_files_table[1]->std = GetStdHandle(STD_OUTPUT_HANDLE);
	opened_files_table[1]->openCount = 1;
	opened_files_table[1]->FILE_TYPE = F_TYPE_STD;

	opened_files_table_instances[1] = new opened_file_instance();
	opened_files_table_instances[1]->file = 1;
	opened_files_table_instances[1]->mode = F_MODE_WRITE;

	/*
	system err
	*/
	opened_files_table[2] = new opened_file();
	opened_files_table[2]->std = GetStdHandle(STD_ERROR_HANDLE);
	opened_files_table[2]->openCount = 1;
	opened_files_table[2]->FILE_TYPE = F_TYPE_STD;

	opened_files_table_instances[2] = new opened_file_instance();
	opened_files_table_instances[2]->file = 2;
	opened_files_table_instances[2]->mode = F_MODE_WRITE;

	/*
	working directory (FS root) for the first shell
	*/
	opened_files_table[3] = new opened_file();
	opened_files_table[3]->node = getRoot();
	opened_files_table[3]->openCount = 1;
	opened_files_table[3]->FILE_TYPE = F_TYPE_FILE;

	opened_files_table_instances[3] = new opened_file_instance();
	opened_files_table_instances[3]->file = 3;
	opened_files_table_instances[3]->mode = F_MODE_BOTH;
}					

void HandleIO(CONTEXT &regs) {

	switch (Get_AL((__int16)regs.Rax)) {
	case scCreatePipe: {
		FDHandle w, r;
		regs.Rax = (decltype(regs.Rax))open_pipe(&w, &r);
		regs.Rbx = (decltype(regs.Rbx))w;
		regs.Rcx = (decltype(regs.Rcx))r;
		break;
	}
	case scOpenFile: {
		FDHandle ho;
		regs.Rax = (decltype(regs.Rax))open_file((char *)regs.Rdx, (int)regs.Rcx, *((bool*)(regs.Rbx)), &ho);
		regs.Rbx = (decltype(regs.Rbx))ho;
		break;
	}
	case scCloseFile: {
		regs.Rax = (decltype(regs.Rax))close_file((FDHandle)regs.Rdx);
		break;
	}
	case scPeekFile: {
		size_t available;
		regs.Rax = (decltype(regs.Rax))peek_file((FDHandle)regs.Rdx, &available);
		regs.Rbx = (decltype(regs.Rax))available;
		break;
	}
	case scReadFile: {
		size_t read;
		regs.Rax = (decltype(regs.Rax))read_file((FDHandle)regs.Rdx, (int)regs.Rcx, (char*)regs.Rbx, &read);
		regs.Rbx = (decltype(regs.Rbx))read;
		break;
	}
	case scDuplicateHandle: {
		FDHandle hd;
		regs.Rax = (decltype(regs.Rax))duplicate_handle((FDHandle)regs.Rcx, &hd);
		regs.Rbx = (decltype(regs.Rbx))hd;
		break;
	}
	case scWriteFile: {
		size_t written = 0;
		regs.Rax = (decltype(regs.Rax))write_file((FDHandle)regs.Rbx, (int)regs.Rdx, (char*)regs.Rcx, &written);
		regs.Rbx = (decltype(regs.Rbx))written;
		break;
	}
	case scMakeDir: {
		regs.Rax = (decltype(regs.Rax))mkdir((char*)regs.Rbx);
		break;
	}
	case scChangeDir: {
		regs.Rax = (decltype(regs.Rax))change_dir((char*)regs.Rbx);
		break;
	}
	case scRemoveDir: {
		regs.Rax = (decltype(regs.Rax))remove_dir((char*)regs.Rbx);
		break;
	}
	case scRemoveFile: {
		regs.Rax = (decltype(regs.Rax))remove_file((char*)regs.Rbx);
		break;
	}
	case scGetDirNodes: {
		regs.Rax = (decltype(regs.Rax))getDirNodes((std::vector<node_info*>*)regs.Rbx, (char*)regs.Rcx);
		Set_Error(regs.Rax == S_FALSE, regs);
		break;
	}

	}//end switch

	//nastala chyba?
	Set_Error(regs.Rax == S_FALSE, regs);
}

bool findIfOpenedFileExists(node * n, FDHandle * handle) {
	std::lock_guard<std::mutex> lock(files_table_mtx);
	for (int i = 0; i < OPEN_FILES_TABLE_SIZE; i++) {
		if (opened_files_table[i] != nullptr && opened_files_table[i]->node == n) {
			*handle = i;
			return true;
		}
	}
	return false;
}

bool findIfSomeoneElseHasPipeOpened(FDHandle instanceToExclude, FDHandle pipeToFind, int openMode) {
	std::lock_guard<std::mutex> lock(files_table_mtx);
	for (int i = 0; i < OPEN_INSTANCES_TABLE_SIZE; i++) {

		if (i != instanceToExclude &&
			opened_files_table_instances[i] != nullptr &&
			opened_files_table_instances[i]->file == pipeToFind &&
			opened_files_table_instances[i]->mode == openMode) 
		{

			return true;
		}
	}
	return false;
}

bool findIfSomeoneElseHasPipeOpenedForReading(FDHandle instanceToExclude, FDHandle pipeToFind) {
	return findIfSomeoneElseHasPipeOpened(instanceToExclude, pipeToFind, F_MODE_READ);
}
bool findIfSomeoneElseHasPipeOpenedForWriting(FDHandle instanceToExclude, FDHandle pipeToFind) {
	return findIfSomeoneElseHasPipeOpened(instanceToExclude, pipeToFind, F_MODE_WRITE);
}

int takeFirstEmptyPlaceInFileTable() {
	int _h = -1;
	std::lock_guard<std::mutex> lock(files_table_mtx);
	for (int i = 0; i < OPEN_FILES_TABLE_SIZE; i++) {
		if (opened_files_table[i] == nullptr) {
			opened_files_table[i] = new opened_file();
			_h = i;
			break;
		}
	}
	return _h;
}

int takeFirstEmptyPlaceInInstanceTable() {
	int _h = -1;
	std::lock_guard<std::mutex> lock(files_table_mtx);
	for (int i = 0; i < OPEN_INSTANCES_TABLE_SIZE; i++) {
		if (opened_files_table_instances[i] == nullptr) {
			opened_files_table_instances[i] = new opened_file_instance();
			_h = i;
			break;
		}
	}
	return _h;
}


HRESULT change_dir(char * path) {

	opened_file_instance *currentInst = opened_files_table_instances[process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors[3]];
	node * currentNode = opened_files_table[currentInst->file]->node;

	node *n;
	HRESULT ok = getNodeFromPath(path, true, currentNode, &n);
	if (ok != S_OK) {
		SetLastError(ERR_IO_PATH_NOEXIST);
		return S_FALSE;
	}
	else if (n->type != TYPE_DIRECTORY) {
		SetLastError(ERR_IO_FILE_ISNOTFOLDER);
		return S_FALSE;
	}
	
	else{
		FDHandle openedHandle;
		bool exists = findIfOpenedFileExists(n, &openedHandle);
		if (exists) {
			std::lock_guard<std::mutex> lock(files_table_mtx);
			opened_files_table[openedHandle]->openCount++;
			currentInst->file = openedHandle;		
		}
		else {
			{
				std::lock_guard<std::mutex> lock(files_table_mtx);
				opened_files_table[currentInst->file]->openCount--;
				if (opened_files_table[currentInst->file]->openCount < 1) {
					delete opened_files_table[currentInst->file];
					opened_files_table[currentInst->file] = nullptr;
				}
			}
			openedHandle = takeFirstEmptyPlaceInFileTable();
			if (openedHandle < 0) {
				SetLastError(ERR_IO_FILE_CREATE);
				return S_FALSE;
			}
			else {
				std::lock_guard<std::mutex> lock(files_table_mtx);
				currentInst->file = openedHandle;
				opened_files_table[currentInst->file]->openCount = F_TYPE_FILE;
				opened_files_table[currentInst->file]->openCount = 1;
				opened_files_table[currentInst->file]->node = n;
			}
		}

		//zmena soucasne slozky volajicimu shellu
		getPathFromNode(n, &(process_table[TIDtoPID[std::this_thread::get_id()]]->currentPath));
	}
	return S_OK;
}


HRESULT open_pipe(FDHandle * whandle, FDHandle * rhandle) {

	FDHandle _h;
	int ih = takeFirstEmptyPlaceInFileTable();
	if (ih < 0) {
		SetLastError(ERR_IO_FILE_CREATE);
		return S_FALSE;
	}
	_h = ih;

	opened_files_table[_h]->FILE_TYPE = F_TYPE_PIPE;
	opened_files_table[_h]->pipe = new pipe();
	opened_files_table[_h]->openCount = 2;

	int wh = takeFirstEmptyPlaceInInstanceTable();
	if (wh < 0) {
		std::lock_guard<std::mutex> lock(files_table_mtx);
		delete opened_files_table[_h];
		opened_files_table[_h] = nullptr;
		return S_FALSE;
	}

	int rh = takeFirstEmptyPlaceInInstanceTable();
	if (rh < 0) {
		std::lock_guard<std::mutex> lock(files_table_mtx);
		delete opened_files_table[_h];
		opened_files_table[_h] = nullptr;
		delete opened_files_table_instances[wh];
		opened_files_table_instances[wh] = nullptr;
		return S_FALSE;
	}

	opened_files_table_instances[wh]->file = _h;
	opened_files_table_instances[wh]->mode = F_MODE_WRITE;
	opened_files_table_instances[rh]->file = _h;
	opened_files_table_instances[rh]->mode = F_MODE_READ;

	*whandle = wh;
	*rhandle = rh;

	process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors.push_back(wh);
	process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors.push_back(rh);

	return S_OK;
}


HRESULT duplicate_handle(FDHandle orig_handle, FDHandle * duplicated_handle) {

	int new_h = takeFirstEmptyPlaceInInstanceTable();
	if (new_h < 0) {
		SetLastError(ERR_IO_FILE_CREATE);
		return S_FALSE;
	}
	opened_files_table_instances[new_h]->file = opened_files_table_instances[orig_handle]->file;
	opened_files_table_instances[new_h]->mode = opened_files_table_instances[orig_handle]->mode;
	opened_files_table_instances[new_h]->pos = opened_files_table_instances[orig_handle]->pos; //?
	{
		std::lock_guard<std::mutex> lock(files_table_mtx);
		opened_files_table[opened_files_table_instances[orig_handle]->file]->openCount++;
	}
	*duplicated_handle = new_h;
	process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors.push_back(new_h);
	return S_OK;
}


//jen pro soubory
HRESULT open_file(char *path, int MODE, bool rewrite, FDHandle * handle) {

	FDHandle H;

	const int pid = TIDtoPID[std::this_thread::get_id()]; 
	const FDHandle inst_h = process_table[pid]->IO_descriptors[3]; 
	const FDHandle file_h = opened_files_table_instances[inst_h]->file; 
	node * current = opened_files_table[file_h]->node;

	node *n;
	HRESULT ok = openFile(&n, path, rewrite, MODE != F_MODE_READ, current);
	if (ok == S_FALSE) {
		return S_FALSE;
	}

	if (!findIfOpenedFileExists(n, &H)) {
		int _h = takeFirstEmptyPlaceInFileTable();
		if (_h < 0) {
			SetLastError(ERR_IO_FILE_CREATE);
			return S_FALSE;
		}
		opened_files_table[_h]->FILE_TYPE = F_TYPE_FILE;
		opened_files_table[_h]->node = n;
		H = _h;
	}

	int _h = takeFirstEmptyPlaceInInstanceTable();
	if (_h < 0) {
		SetLastError(ERR_IO_FILE_CREATE);
		return S_FALSE;
	}
	{
		std::lock_guard<std::mutex> lock(files_table_mtx);
		opened_files_table[H]->openCount++;
	}

	opened_files_table_instances[_h]->mode = MODE;
	opened_files_table_instances[_h]->file = H;
	*handle = _h;
	process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors.push_back(_h);
	return S_OK;
}

HRESULT close_file(FDHandle handle) {

	std::vector<FDHandle> &handles = process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors;
	handles.erase(std::remove(handles.begin(), handles.end(), handle), handles.end()); //smazat z PCB
	
	FD_instance * inst = opened_files_table_instances[handle];
	FD * fd = opened_files_table[inst->file];

	{
		std::lock_guard<std::mutex> lock(files_table_mtx);
		fd->openCount--;
	}

	if (fd->FILE_TYPE == F_TYPE_PIPE) {
		if (inst->mode == F_MODE_WRITE && !findIfSomeoneElseHasPipeOpenedForWriting(handle, inst->file)) {
			fd->pipe->close_write();
		}
		else if (!findIfSomeoneElseHasPipeOpenedForReading(handle, inst->file)) {
			fd->pipe->close_read();
		}
	}

	std::lock_guard<std::mutex> lock(files_table_mtx);
	if (fd->openCount <= 0) {	
		if (fd->FILE_TYPE == F_TYPE_PIPE) {
			delete opened_files_table[inst->file]->pipe;
		}
		delete opened_files_table[inst->file];
		opened_files_table[inst->file] = nullptr;
	}
	delete opened_files_table_instances[handle];
	opened_files_table_instances[handle] = nullptr;

	return S_OK;
}


HRESULT peek_file(FDHandle handle, size_t *available) {

	opened_file_instance *file_inst = opened_files_table_instances[handle];
	opened_file *file = opened_files_table[file_inst->file];

	switch (file->FILE_TYPE) {

	case F_TYPE_STD: {
		/*DWORD lpTotalBytesAvail = 0;
		PeekNamedPipe(file->std, NULL, 0, NULL, &lpTotalBytesAvail, NULL);
		*available = lpTotalBytesAvail;*/
		*available = 0;
		break;
	}
	case F_TYPE_PIPE: {
		file->pipe->peek(available);
		break;
	}
	case F_TYPE_FILE: {
		int a = (int)file->node->data.length() - (int)file_inst->pos;
		//+1 for EOF
		*available = a < 0 ? 1 : a + 1; //kdyby nahodou... do size_t nesmime dat nic zapornyho
		*available = *available > 1000 ? 1000 : *available; //nechceme aby nam proces cetl celej velkej soubor naraz
		break;
	}
	}
	return S_OK;
}

//Pokud narazi na konec souboru (Ctrl+Z z konzole, roura zavrena pro zapis, konec dat v node),
//tj. precte mene, nez bylo pozadovano (howMuch), bude EOF v buf[read].
//Nedava na konec \0, to si musi uzivatelsky program pohlidat sam.
HRESULT read_file(FDHandle handle, size_t howMuch, char * buf, size_t * read) {

	opened_file_instance *file_inst = opened_files_table_instances[handle];
	opened_file *file = opened_files_table[file_inst->file];
	
	if (file_inst->mode == F_MODE_WRITE) {
		//TODO set error ze neni pro cteni
		return 0;
	}
	switch (file->FILE_TYPE) {
	case F_TYPE_STD: {
		unsigned long r;
		bool success = (ReadFile(file->std, buf, (DWORD)howMuch, &r, nullptr) != FALSE);
		if (r == 0 && success) {
			buf[0] = EOF;
		} else if (!success) {
			*read = 0;
			SetLastError(ERR_IO_READ_STD);
			return S_FALSE;
		}
		*read = (size_t)r;
		break;
	}
	case F_TYPE_PIPE: {
		file->pipe->read(howMuch, buf, read);
		break;
	}
	case F_TYPE_FILE: {
		bool success = (S_OK == getData(&(file->node), file_inst->pos, howMuch, &buf, read));
		file_inst->pos += *read;
		break;
	}
	}//end switch
	return S_OK;
}

HRESULT write_file(FDHandle handle, size_t howMuch, char * buf, size_t *written) {

	opened_file_instance *file_inst = opened_files_table_instances[handle];
	opened_file *file = opened_files_table[file_inst->file];

	if (file_inst->mode == F_MODE_READ) {
		//TODO set error ze neni pro zapis
		return S_FALSE;
	}
	switch (file->FILE_TYPE) {
	case F_TYPE_STD: {
		if (WriteFile(file->std, buf, (DWORD)howMuch, (LPDWORD)written, nullptr) == FALSE) {
			*written = 0;
			SetLastError(ERR_IO_WRITE_STD);
			return S_FALSE;
		}
		break;
	}
	case F_TYPE_PIPE: {
		if (!((file->pipe)->write(buf, howMuch, written))) {
			return S_FALSE;
		}
		break;
	}
	case F_TYPE_FILE: {
		if (setData(&(file->node), buf) == S_OK) {
			*written = howMuch;
		} else {
			*written = 0;
			return S_FALSE;
		}
		break;
	}
	} //end switch
	return S_OK;
}

HRESULT mkdir(char * path) {
	const int pid = TIDtoPID[std::this_thread::get_id()];
	const FDHandle inst_h = process_table[pid]->IO_descriptors[3];
	const FDHandle file_h = opened_files_table_instances[inst_h]->file;
	node * current = opened_files_table[file_h]->node;
	node *dir; //zde uložena vytvoøená složka, pokud již existuje, FS uloží nullptr a vrátí false
	return mkdir(&dir, path, current);
}

HRESULT remove_dir(char * path) {

	opened_file_instance *currentInst = opened_files_table_instances[process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors[3]];
	node * currentNode = opened_files_table[currentInst->file]->node;

	node *n;
	HRESULT ok = getNodeFromPath(path, true, currentNode, &n);
	if (ok != S_OK) {
		/*not found, chybu nastavuje FS*/
		return S_FALSE;
	}
	if (n->type != TYPE_DIRECTORY) {
		/*not directory*/
		SetLastError(ERR_IO_FILE_ISNOTFOLDER);
		return S_FALSE;
	}
	FDHandle openedHandle;
	bool exists = findIfOpenedFileExists(n, &openedHandle);
	if (exists) {
		/*dir is being used by another process*/
		SetLastError(ERR_IO_FILE_ISOPENED);
		return S_FALSE;
	}
	return deleteNode(n);
}
HRESULT remove_file(char * path) {

	opened_file_instance *currentInst = opened_files_table_instances[process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors[3]];
	node * currentNode = opened_files_table[currentInst->file]->node;

	node *n;
	HRESULT ok = getNodeFromPath(path, true, currentNode, &n);
	if (ok != S_OK) {
		/*not found, chybu nastavuje FS*/
		return S_FALSE;
	}
	if (n->type != TYPE_FILE) {
		/*it's not a file*/
		SetLastError(ERR_IO_FILE_ISNOTFILE);
		return S_FALSE;
	}
	FDHandle openedHandle;
	bool exists = findIfOpenedFileExists(n, &openedHandle);
	if (exists) {
		/*file is being used by another process*/
		SetLastError(ERR_IO_FILE_ISOPENED);
		return S_FALSE;
	}
	return deleteNode(n);
}
/*On zero position is always current dir*/
HRESULT getDirNodes(std::vector<node_info*> *all_info, char *path) {
	opened_file_instance *currentInst = opened_files_table_instances[process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors[3]];
	node * currentNode = opened_files_table[currentInst->file]->node;
	node *n; 
	if (path == nullptr){
		n = currentNode;
	}
	else {
		if (getNodeFromPath(path, true, currentNode, &n) != S_OK) {
			/*not found, erro nastavuje FS*/
			return S_FALSE;
		} ;
	}
	node_info *info = new node_info;
	info->name = n->name;
	info->type = n->type;
	info->size = n->data.size();
	getPathFromNode(n, &(info->path));
	if (n->type == TYPE_FILE) {
		getPathFromNode(n->parent, &(info->pathParent));
	}
	all_info->push_back(info);
	for (size_t i = 0; i < n->children.size(); i++) {
		if (n->children[i] != nullptr) {
			node_info *info = new node_info;
			info->name = n->children[i]->name;
			info->type = n->children[i]->type;
			info->size = n->children[i]->data.size();
			all_info->push_back(info);
		}
	}
	return S_OK;
}