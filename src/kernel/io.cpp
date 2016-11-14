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

/*
void closeSystemIO() {
	delete opened_files_table_instances[0];
	delete opened_files_table_instances[1];
	delete opened_files_table_instances[2];
	opened_files_table_instances[0] = nullptr;
	opened_files_table_instances[1] = nullptr;
	opened_files_table_instances[2] = nullptr;
	delete opened_files_table[0];
	delete opened_files_table[1];
	delete opened_files_table[2];
	opened_files_table[0] = nullptr;
	opened_files_table[1] = nullptr;
	opened_files_table[2] = nullptr;
}*/

void HandleIO(CONTEXT &regs) {

	switch (Get_AL((__int16) regs.Rax)) {

		case scCreatePipe: 

			FDHandle w, r;
			regs.Rax = (decltype(regs.Rax))open_pipe(&w, &r);
			regs.Rbx = (decltype(regs.Rbx))w;
			regs.Rcx = (decltype(regs.Rcx))r;
			Set_Error(regs.Rax == 0, regs);
			break;

		case scOpenFile: 

			FDHandle ho;
			regs.Rax = (decltype(regs.Rax))open_file((char *)regs.Rdx, (int)regs.Rcx, &ho);
			regs.Rbx = (decltype(regs.Rbx))ho;
			Set_Error(regs.Rax == 0, regs);		
			break;

		case scCloseFile:
			Set_Error(close_file((FDHandle)regs.Rdx) == 0, regs);
			break;

		case scReadFile:
			regs.Rax = (decltype(regs.Rax))read_file((FDHandle)regs.Rdx, (int)regs.Rcx, (char*)regs.Rbx);
			Set_Error(regs.Rax == 0, regs);
			break;	

		case scDuplicateHandle: 
			FDHandle hd;
			regs.Rax = (decltype(regs.Rax))duplicate_handle((FDHandle)regs.Rcx, &hd);
			regs.Rbx = (decltype(regs.Rbx))hd;
			Set_Error(regs.Rax == 0, regs);
			break;

		/*
		case scCreateFile: {
				regs.Rax = (decltype(regs.Rax)) CreateFileA((char*)regs.Rdx, GENERIC_READ | GENERIC_WRITE , (DWORD) regs.Rcx, 0, OPEN_EXISTING, 0, 0);
				//zde je treba podle Rxc doresit shared_read, shared_write, OPEN_EXISING, etc. podle potreby
				Set_Error(regs.Rax == 0, regs);				
			}
		break;	//scCreateFile
		*/
			
		case scWriteFile:
			regs.Rax = (decltype(regs.Rax))write_file((FDHandle)regs.Rbx, (int)regs.Rdx, (char*)regs.Rcx);
			Set_Error(regs.Rax == 0, regs);
		break;


		/*
		case scWriteFile: {
				DWORD written;
				const bool failed = !WriteFile((HANDLE)regs.Rdx, (void*)regs.Rdi, (DWORD)regs.Rcx, &written, NULL);
				Set_Error(failed, regs);
				if (!failed) regs.Rax = written;
			}
			break; //scWriteFile
		*/
		/*
		case scCloseFile: {
			Set_Error(!CloseHandle((HANDLE)regs.Rdx), regs);			
			}
			break;	//CloseFile
			*/
	}
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
	//&pos?
	{
		std::lock_guard<std::mutex> lock(files_table_mtx);
		opened_files_table[opened_files_table_instances[orig_handle]->file]->openCount++;
	}
	*duplicated_handle = new_h;
	return 1;
}

// co se slozkama?????????????????
//path must be \0 terminated!
//If MODE is not READ and file of specified path already exists, its content gets deleted.
int open_file(char *path, int MODE, FDHandle * handle) {

	FDHandle H;

	/*
	Tady musi bejt PCB[pid]->current_node misto nullptr !!!! (no anebo current_node poslanej pres syscall)
	Pokud furt nechceme posilat do jadra pid procesu (resp. pouzit na identifikaci procesu nativni thread ID),
	tak musime do syscallu posilat current_node z uzivatelskyho procesu - coz by nemel bejt asi takovej problem?.
	*/
	node *n = openFile(TYPE_FILE, path, MODE != F_MODE_READ, nullptr /* !! */ );

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
	/*if (fd->FILE_TYPE == F_TYPE_STD) {
		return 0;
	}*/
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

size_t read_file(FDHandle handle, size_t howMuch, char * buf) {

	opened_file_instance *file_inst = opened_files_table_instances[handle];
	opened_file *file = opened_files_table[file_inst->file];
	
	if (file_inst->mode == F_MODE_WRITE) {
		return 0;
	}

	size_t read = 0;
	BOOL success = false;

	switch (file->FILE_TYPE) {

	case F_TYPE_STD:
		unsigned long r;
		success = ReadFile(file->std, buf, howMuch, &r, nullptr);
		if (r == 0 && success) {
			r = 1;
			buf[0] = EOF;
		}
		read = r;
		break;

	case F_TYPE_PIPE:
		success = file->pipe->read(howMuch, buf, &read);
		break;

	case F_TYPE_FILE:
		//TODO
		//cist z node
		if (getData(&(file->node), file_inst->pos, howMuch, &buf, &read) == 0) success = 1;
		else success = 0;
		break;
	}

	if (success) return read;
	else {
		SetLastError(ERROR_READ_FAULT);
		return 0;
	}
}

size_t write_file(FDHandle handle, size_t howMuch, char * buf) {

	opened_file_instance *file_inst = opened_files_table_instances[handle];
	opened_file *file = opened_files_table[file_inst->file];

	size_t written;
	BOOL success = false;

	if (file_inst->mode == F_MODE_READ) {
		return 0;
	}

	switch (file->FILE_TYPE) {

	case F_TYPE_STD:
		success = WriteFile(file->std, buf, howMuch, (LPDWORD) &written, nullptr);
		break;

	case F_TYPE_PIPE:
		success = (file->pipe)->write(buf, howMuch, &written);
		break;

	case F_TYPE_FILE:
		if (setData(&(file->node), file_inst->pos, howMuch, buf) == 0) success = 1;
		else success = 0;
		//TODO
		//zapsat do node
		break;
	}

	if (success) return written;
	else {
		SetLastError(ERROR_WRITE_FAULT);
		return 0;
	}
}