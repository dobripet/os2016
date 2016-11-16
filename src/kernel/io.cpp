#include "io.h"
#include "kernel.h"
#include "process.h"

#include <iostream>

std::mutex files_table_mtx;
opened_file * opened_files_table[OPEN_FILES_TABLE_SIZE] = { nullptr };
opened_file_instance * opened_files_table_instances[OPEN_INSTANCES_TABLE_SIZE] = { nullptr };
/*
Ta druha tabulka by mozna mohla/mela byt nekde u procesu v PCB.
Ale vznika tam trochu problem - takhle od procesu pro otevreni souburu potrebuju
jenom cestu (a jestli chce slozku/soubor a pro cteni/zapis/oboje).
Kdyby mìla obsluha otevirani souboru pridavat otevrenej soubor do PCB, bude se muset 
do syscallu otevirani souboru poslat i PID a to nevim jestli neni lepsi bez toho.

Dale je dobry, ze proces potrebuje znat (aby mohl zachazet s otevrenym souborem) jenom
FDHandle (index to tabulky otevrenych instanci) a nic vic.
*/

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
		Set_Error(regs.Rax == 0, regs);
		break;
	}

	case scOpenFile: {
		FDHandle ho;
		regs.Rax = (decltype(regs.Rax))open_file((char *)regs.Rdx, (int)regs.Rcx, &ho);
		regs.Rbx = (decltype(regs.Rbx))ho;
		Set_Error(regs.Rax == 0, regs);
		break;
	}

	case scCloseFile: {
		Set_Error(close_file((FDHandle)regs.Rdx) == 0, regs);
		break;
	}

					  /*case scPeekFile: {
						  size_t available;
						  regs.Rax = (decltype(regs.Rax))peek_file((FDHandle)regs.Rdx, &available);
						  regs.Rbx = (decltype(regs.Rax))available;
						  Set_Error(regs.Rax != 0, regs);
						  break;
					  }*/

	case scReadFile: {
		regs.Rax = (decltype(regs.Rax))read_file((FDHandle)regs.Rdx, (int)regs.Rcx, (char*)regs.Rbx);
		Set_Error(regs.Rax == 0, regs);
		break;
	}

	case scDuplicateHandle: {
		FDHandle hd;
		regs.Rax = (decltype(regs.Rax))duplicate_handle((FDHandle)regs.Rcx, &hd);
		regs.Rbx = (decltype(regs.Rbx))hd;
		Set_Error(regs.Rax == 0, regs);
		break;
	}

	case scWriteFile: {
		regs.Rax = (decltype(regs.Rax))write_file((FDHandle)regs.Rbx, (int)regs.Rdx, (char*)regs.Rcx);
		Set_Error(regs.Rax == 0, regs);
		break;
	}

	case scMakeDir: {
		regs.Rax = (decltype(regs.Rax))mkdir((char*)regs.Rbx);
		Set_Error(regs.Rax != 0, regs);
		break;
	}

	case scChangeDir: {
		regs.Rax = (decltype(regs.Rax))change_dir((char*)regs.Rbx);
		Set_Error(regs.Rax != 0, regs);
		break;
	}

	case scRemoveDir: {
		regs.Rax = (decltype(regs.Rax))remove_dir((char*)regs.Rbx);
		Set_Error(regs.Rax != 0, regs);
		break;
	}

	}//end switch
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


int change_dir(char * path) {

	opened_file_instance *currentInst = opened_files_table_instances[process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors[3]];
	node * currentNode = opened_files_table[currentInst->file]->node;

	node *n;
	HRESULT ok = getNodeFromPath(path, true, currentNode, &n);
	if (ok != S_OK) {
		return 1;
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
				//error
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
	return 0;
}


int open_pipe(FDHandle * whandle, FDHandle * rhandle) {

	FDHandle _h;
	int ih = takeFirstEmptyPlaceInFileTable();
	if (ih < 0) {
		return 0; 
		//ERROR
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
		return 0;
	}

	int rh = takeFirstEmptyPlaceInInstanceTable();
	if (rh < 0) {
		std::lock_guard<std::mutex> lock(files_table_mtx);
		delete opened_files_table[_h];
		opened_files_table[_h] = nullptr;
		delete opened_files_table_instances[wh];
		opened_files_table_instances[wh] = nullptr;
		return 0;
	}

	opened_files_table_instances[wh]->file = _h;
	opened_files_table_instances[wh]->mode = F_MODE_WRITE;
	opened_files_table_instances[rh]->file = _h;
	opened_files_table_instances[rh]->mode = F_MODE_READ;

	*whandle = wh;
	*rhandle = rh;

	return 1;
}


int duplicate_handle(FDHandle orig_handle, FDHandle * duplicated_handle) {

	int new_h = takeFirstEmptyPlaceInInstanceTable();
	if (new_h < 0) {
		//fail
		return 0;
	}
	opened_files_table_instances[new_h]->file = opened_files_table_instances[orig_handle]->file;
	opened_files_table_instances[new_h]->mode = opened_files_table_instances[orig_handle]->mode;
	opened_files_table_instances[new_h]->pos = opened_files_table_instances[orig_handle]->pos; //?
	{
		std::lock_guard<std::mutex> lock(files_table_mtx);
		opened_files_table[opened_files_table_instances[orig_handle]->file]->openCount++;
	}
	*duplicated_handle = new_h;
	return 1;
}


int open_file(char *path, int MODE, FDHandle * handle) {

	FDHandle H;

	const int pid = TIDtoPID[std::this_thread::get_id()]; 
	const FDHandle inst_h = process_table[pid]->IO_descriptors[3]; 
	const FDHandle file_h = opened_files_table_instances[inst_h]->file; 
	node * current = opened_files_table[file_h]->node;

	node *n;
	openFile(&n, path, MODE != F_MODE_READ, MODE != F_MODE_READ, current);

	if (!findIfOpenedFileExists(n, &H)) {
		int _h = takeFirstEmptyPlaceInFileTable();
		if (_h < 0) {
			return 0;
			//ERROR
		}
		opened_files_table[_h]->FILE_TYPE = F_TYPE_FILE;
		opened_files_table[_h]->node = n;
		H = _h;
	}

	int _h = takeFirstEmptyPlaceInInstanceTable();
	if (_h < 0) {
		return 0;
		//ERROR
	}
	{
		std::lock_guard<std::mutex> lock(files_table_mtx);
		opened_files_table[H]->openCount++;
	}

	opened_files_table_instances[_h]->mode = MODE;
	opened_files_table_instances[_h]->file = H;
	*handle = _h;

	return 1;
}

int close_file(FDHandle handle) {
	
	FD_instance * inst = opened_files_table_instances[handle];
	FD * fd = opened_files_table[inst->file];

	{
		std::lock_guard<std::mutex> lock(files_table_mtx);
		fd->openCount--;
	}

	if (fd->FILE_TYPE == F_TYPE_PIPE) {
		if (inst->mode == F_MODE_WRITE) {
			fd->pipe->close_write();
		} else {
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

	return 1;
}

/*
int peek_file(FDHandle handle, size_t *available) {

	opened_file_instance *file_inst = opened_files_table_instances[handle];
	opened_file *file = opened_files_table[file_inst->file];

	switch (file->FILE_TYPE) {

	case F_TYPE_STD: {
		DWORD lpTotalBytesAvail = 0;
		PeekNamedPipe(file->std, NULL, 0, NULL, &lpTotalBytesAvail, NULL);
		*available = lpTotalBytesAvail;
		break;
	}
	case F_TYPE_PIPE: {
		file->pipe->peek(available);
		break;
	}
	case F_TYPE_FILE: {
		*available = 1 + file->node->data.length() - file_inst->pos;
		//+1 for EOF
		break;
	}
	}
	return 0;
}
*/

size_t read_file(FDHandle handle, size_t howMuch, char * buf) {

	opened_file_instance *file_inst = opened_files_table_instances[handle];
	opened_file *file = opened_files_table[file_inst->file];
	
	if (file_inst->mode == F_MODE_WRITE) {
		return 0;
	}

	size_t read = 0;
	bool success = false;

	switch (file->FILE_TYPE) {

	case F_TYPE_STD: {
		unsigned long r;
		success = (ReadFile(file->std, buf, (DWORD)howMuch, &r, nullptr) != FALSE);
		if (r == 0 && success) {
			r = 1;
			buf[0] = EOF;
		} else if (!success) {
			//?
		}
		read = r;
		break;

	}

	case F_TYPE_PIPE: {
		success = file->pipe->read(howMuch, buf, &read);
		break;
	}
	case F_TYPE_FILE: {
		success = (S_OK == getData(&(file->node), file_inst->pos, howMuch, &buf, &read));
		break;
	}

	}//end switch

	if (success) return read;
	else {
		SetLastError(ERROR_READ_FAULT);
		return 0;
	}
}

size_t write_file(FDHandle handle, size_t howMuch, char * buf) {

	opened_file_instance *file_inst = opened_files_table_instances[handle];
	opened_file *file = opened_files_table[file_inst->file];

	size_t written = 0;
	bool success = false;

	if (file_inst->mode == F_MODE_READ) {
		return 0;
	}

	switch (file->FILE_TYPE) {

	case F_TYPE_STD: {
		success = (WriteFile(file->std, buf, (DWORD)howMuch, (LPDWORD)&written, nullptr) != FALSE);
		if (!success) {
			//?
		}
		break;
	}

	case F_TYPE_PIPE: {
		success = (file->pipe)->write(buf, howMuch, &written);
		break;
	}

	case F_TYPE_FILE: {
		success = (setData(&(file->node), file_inst->pos, howMuch, buf) == S_OK);
		break;
	}

	} //end switch

	if (success) {
		return written;
	}
	else {
		SetLastError(ERROR_WRITE_FAULT);
		return 0;
	}	
}

int mkdir(char * path) {
	const int pid = TIDtoPID[std::this_thread::get_id()];
	const FDHandle inst_h = process_table[pid]->IO_descriptors[3];
	const FDHandle file_h = opened_files_table_instances[inst_h]->file;
	node * current = opened_files_table[file_h]->node;
	node *dir; //zde uložena vytvoøená složka, pokud již existuje, uloží nullptr a vrátí false
	return mkdir(&dir, path, current);
}

int remove_dir(char * path) {
	opened_file_instance *currentInst = opened_files_table_instances[process_table[TIDtoPID[std::this_thread::get_id()]]->IO_descriptors[3]];
	node * currentNode = opened_files_table[currentInst->file]->node;

	node *n;
	HRESULT ok = getNodeFromPath(path, currentNode, &n);
	if (ok != S_OK) {
		/*not found*/
		SetLastError(ERR_IO_PATH_NOEXIST); //tuhle chybu bude nastavovat FS nejspi
		return 1;
	}

	if (n->type != TYPE_DIRECTORY) {
		/*not directory*/
		SetLastError(ERR_IO_FILE_ISFILE);
		return 1;
	}
	FDHandle openedHandle;
	bool exists = findIfOpenedFileExists(n, &openedHandle);
	if (exists) {
		/*dir in use*/
		SetLastError(ERR_IO_FILE_ISOPENED);
		return 1;
	}
	if (deleteNode(n) != S_OK) {
		/*has children*/
		SetLastError(ERR_IO_FILE_NOTEMPTY); //tuhle chybu bude nastavovat FS nejspi
		return 1;
	}
	return 0;
}
