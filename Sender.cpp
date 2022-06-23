#include<iostream>
#include<fstream>
#include<string>
#include<Windows.h>
#include"message.h"
#pragma warning(disable:4996)
using namespace std;
int main(int argc, char* argv[]) {
	HANDLE ready = OpenEvent(EVENT_MODIFY_STATE, FALSE, (to_wstring(atoi(argv[3])) + L"ready").c_str());
	HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, L"mutex");
	HANDLE writeSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"write_sem");
	HANDLE readSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"read_sem");
	if (!ready || !readSem || !writeSem || !mutex) {
		cout << "Error" << endl;
		return -1;
	}
	SetEvent(ready);
	fstream file;
	int action;
	while (true) {
		cout << "1 to write message" << endl;
		cout << "0 to exit" << endl;
		cin >> action;
		if (action != 0 && action != 1) {
			cout << "Unknown command" << endl;
			continue;
		}
		if (action == 0) {
			break;
		}
		cin.ignore();
		string text;
		cout << "Enter message text:";
		getline(cin, text);
		WaitForSingleObject(writeSem, INFINITE);
		WaitForSingleObject(mutex, INFINITE);
		message mes(text);
		file.open(argv[1], ios::binary | ios::app);
		file << mes;
		file.close();
		ReleaseMutex(mutex);
		ReleaseSemaphore(readSem, 1, NULL);
		cout << "Written succesfully" << endl;
	}
	CloseHandle(mutex);
	CloseHandle(ready);
	CloseHandle(writeSem);
	CloseHandle(readSem);
	return 0;
}
