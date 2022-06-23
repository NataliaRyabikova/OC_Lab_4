#include<iostream>
#include<fstream>
#include<string>
#include<Windows.h>
#include"message.h"
#include<vector>
#pragma warning(disable:4996)
using namespace std;
HANDLE RunProcess(wstring cmd) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	if (!CreateProcess(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		return NULL;
	}
	CloseHandle(pi.hThread);
	return pi.hProcess;
}
int main() {
	wstring fileName;
	cout << "Enter file name:";
	wcin >> fileName;
	fstream file(fileName, ios::binary | ios::out);
	int numberOfRecords;
	cout << "Enter number of records:";
	cin >> numberOfRecords;
	if (!file.is_open()) {
		cout << "Error while creating file!";
		return 0;
	}
	file.close();
	int numberOfSenders;
	cout << "Enter number of Senders:";
	cin >> numberOfSenders;
	HANDLE* senders = new HANDLE[numberOfSenders];
	HANDLE* events = new HANDLE[numberOfSenders];
	HANDLE mutex = CreateMutex(NULL, FALSE, L"mutex");
	HANDLE writeSem = CreateSemaphore(NULL, numberOfRecords, numberOfRecords, L"write_sem");
	HANDLE readSem = CreateSemaphore(NULL, 0, numberOfRecords, L"read_sem");
	if (!mutex || !writeSem || !readSem) {
		cout << "Error";
		return -1;
	}
	for (int i = 0; i < numberOfSenders; ++i) {
		wstring cmd = L"Sender.exe " + fileName + L" " + to_wstring(numberOfRecords) + L" " + to_wstring(i);
		HANDLE event = CreateEvent(NULL, FALSE, FALSE, (to_wstring(i) + L"ready").c_str());
		events[i] = event;
		senders[i] = RunProcess(cmd);
		if (senders[i] == NULL) {
			cout << "Error while creating process";
			return -1;
		}
	}
	WaitForMultipleObjects(numberOfSenders, events, TRUE, INFINITE);
	int action;
	while (true) {
		cout << "1 to read message" << endl;
		cout << "0 to exit" << endl;
		cin >> action;
		if (action != 0 && action != 1) {
			cout << "Unknown command";
			continue;
		}
		if (action == 0) {
			break;
		}
		WaitForSingleObject(readSem, INFINITE);
		WaitForSingleObject(mutex, INFINITE);
		file.open(fileName, ios::binary | ios::in);
		message mes;
		file >> mes;
		cout << "new message:" << mes.getText() << endl;
		vector <message> fileText;
		while (file >> mes) {
			fileText.push_back(mes);
		}
		file.close();
		file.open(fileName, ios::binary | ios::out);
		for (int i = 0; i < fileText.size(); ++i) {
			file << fileText[i];
		}
		file.close();
		ReleaseMutex(mutex);
		ReleaseSemaphore(writeSem, 1, NULL);
	}
	for (int i = 0; i < numberOfSenders; ++i) {
		CloseHandle(events[i]);
		CloseHandle(senders[i]);
	}
	delete[] events;
	delete[] senders;
	CloseHandle(mutex);
	CloseHandle(readSem);
	CloseHandle(writeSem);
	return 0;
}
