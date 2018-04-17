#define _CRT_SECURE_NO_WARNINGS

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>
#include <conio.h>

//#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") //Winsock Library

#define MAX_USER_LEN 20U
#define MAX_MESSAGE_LEN 100U
#define DEFAULT_NAME "piotrGTX"
#define DEFAULT_DELETE_TIME 10U
#define SHOW_LIMIT 10U

#define MAX_CLIENTS 20
#define IP_ADDRES "42.0.117.0"

HANDLE *watek;
HANDLE watek_servera;
SOCKET mySocket;

// Server
unsigned unsigned int client_index = 0;
struct Client {
	HANDLE watek;
	SOCKET clientSocket;
	char clientName[MAX_USER_LEN];
} clients[MAX_CLIENTS];

static unsigned int countOfThreads = 1;
static unsigned int maxID = 0;

char isChatMod = 0;
char isOnlineMod = 0;
char isServer = 0;

struct Log {
	char user[MAX_USER_LEN];
	char message[MAX_MESSAGE_LEN];
	time_t date;
	unsigned int id;

	struct Log *prev;
} *root = NULL;

char your_name[MAX_USER_LEN] = DEFAULT_NAME;
size_t delete_time = DEFAULT_DELETE_TIME;

void copyText(const char* source, char* desc, size_t limit) {
	size_t i = 0;
	while (i < (limit - 1) && source[i] != '\0') {
		desc[i] = source[i];
		i++;
	}
	desc[limit - 1] = desc[i] = '\0';
}

void addNewMessage(const char* user, const char* message) {
	struct Log* new_message = malloc(sizeof(struct Log));

	copyText(user, new_message->user, MAX_USER_LEN);
	copyText(message, new_message->message, MAX_MESSAGE_LEN);
	new_message->date = time(NULL);
	new_message->id = maxID;

	maxID++;

	new_message->prev = root;
	root = new_message;
}

void removeFrom(struct Log* from) {
	// Nie dba o poprzedni element !
	while (from) {
		struct Log* to_delete = from;
		from = from->prev;
		free(to_delete);
	}
}

void removeAll() {
	removeFrom(root);
	root = NULL;
}

void showMessage(const struct Log* log) {
	if (log) {
		printf("%s %s \t\t %s\n", ctime(&log->date), log->user, log->message);
	}
}

void showMessages(size_t limit) {
	const struct Log* log = root;
	for (size_t i = 0; log && i < limit; log = log->prev, i++) {
		showMessage(log);
	}
}

void showEveryMessages() {
	for (const struct Log* log = root; log; log = log->prev) {
		showMessage(log);
	}
}

void showAllMessages() {
	system("cls");
	printf("Enter aby powrocic do menu\n\n");

	if (!root) {
		printf("Brak wiadomosci... \n");
	}
	else {
		// Poka¿ wszystkie !
		showEveryMessages();
	}

	printf("\n");
	getchar();
	getchar();
}

void changeName() {
	printf("Twoja nowa nazwa: ");
	scanf("%s", your_name);
}

void changeDeleteTime() {
	printf("Nowy czas usuwania wiadomosci: ");
	scanf("%u", &delete_time);
}

void changeThreadCount() {
	do {
		printf("Nowa ilosc watkow: ");
		scanf("%u", &countOfThreads);
	} while (countOfThreads == 0);
}

void chatMod() {
	isChatMod = 1;
	char new_message[MAX_MESSAGE_LEN] = { '\0' };

	while (1) {
		system("cls");
		printf("0 - Powrot do menu\n\n");
		showMessages(SHOW_LIMIT);
		printf("\n");

		printf("%s: ", your_name);
		getchar();
		scanf("%[^\n]", new_message);

		if (new_message[0] == '0' && new_message[1] == '\0') {
			isChatMod = 0;
			break;
		}

		addNewMessage(your_name, new_message);
		if (isOnlineMod == 1) {
			send(mySocket, your_name, strlen(your_name) + 1, 0);
			send(mySocket, new_message, strlen(new_message) + 1, 0);
			if (isServer == 1) {
				// Serwer musi rozesłać wiadomość 
				for (unsigned int i = 0; i < MAX_CLIENTS; i++) {
					send(clients[i].clientSocket, your_name, strlen(your_name) + 1, 0);
					send(clients[i].clientSocket, new_message, strlen(new_message) + 1, 0);
				}
			}
		}
	}
}

void removeOlder() {
	if (root) {
		const time_t limit = time(NULL) - delete_time;
		if (root->date <= limit) {
			removeAll();
		}
		else {
			for (struct Log* elem = root; elem->prev; elem = elem->prev) {
				if (elem->prev->date <= limit) {
					removeFrom(elem->prev);
					elem->prev = NULL;
					break;
				}
			}
		}
	}
}

struct ThreadData {
	const char *text;
	unsigned int start;
} *new_data;

DWORD WINAPI threadFindInLogs(void* args) {

	if (!root) {
		return 0;
	}

	struct ThreadData *myArgs = (struct ThreadData*)(args);

	unsigned int start = myArgs->start;
	const char* text = myArgs->text;

	const struct Log* startLog = root;
	for (unsigned int i = 0; i < start; i++) {
		if (startLog->prev) {
			startLog = startLog->prev;
		}
		else {
			return 0; // Zbyt mało elementów
		}
	}

	unsigned int ilosc = 0, licznik = 0;
	for (const struct Log* log = startLog; log; log = log->prev) {
		if (licznik == 0) {

			for (size_t fake_X = 0; fake_X < 100; fake_X++) // Sztuczna blokada
				strstr(log->message, text);

			if (strstr(log->message, text) != 0) {
				//showMessage(log);
				ilosc++;
			}
		}

		licznik++;
		if (licznik > (countOfThreads - 1)) {
			licznik = 0;
		}
	}

	printf("Watek \"%i\" znalazl %i\n", start, ilosc);

	return 0;
}

void findInLogsInit(const char* text) {

	for (unsigned int i = 0; i < countOfThreads; i++) {
		DWORD id;
		new_data[i].text = text;
		new_data[i].start = i;
		watek[i] = CreateThread(
			NULL,
			0,
			threadFindInLogs,
			(void*)&new_data[i],
			0,
			&id
		);
	}

}

void FindInLogs() {

	watek = (HANDLE*) malloc(sizeof(HANDLE)*countOfThreads);
	new_data = (struct ThreadData*) malloc(sizeof(struct ThreadData)*countOfThreads);

	char text[10];
	printf("Podaj fraze do wyszukania: ");
	scanf("%s", text);

	time_t start = clock();
	findInLogsInit(text);
	WaitForMultipleObjects(countOfThreads, watek, TRUE, INFINITE);
	time_t stop = clock() - start;

	free(new_data);
	free(watek);
		
	printf("Czas wyszukiwanie na %i watkach: %i\n", countOfThreads, stop);

	system("PAUSE");
}



void createRandomMessages(unsigned int ilosc) {
	while (ilosc--) {
		char wiadomosc[40];
		int l = floor(rand() % 20) + 20;
		for (int i = 0; i < l; i++) {
			char new_char = floor(rand() % ('z' - 'a' + 1)) + 'a';
			if (rand() % 40 == 0 && i != 0 && i != l-1 && wiadomosc[i-1] != ' ') {
				new_char = ' ';
			}
			wiadomosc[i] = new_char;
		}
		wiadomosc[l - 1] = '\0';
		addNewMessage("Boot", wiadomosc);
	}
}

void createRandomMessageThread(void *args) {
	createRandomMessages(*(int*)(args));
	return 0;
}

void createRandomMessagesInit() {
	int count;
	do {
		printf("Ilosc wiadomosci: ");
		scanf("%i", &count);
	} while (count == 0);

	watek = (HANDLE*)malloc(sizeof(HANDLE)*countOfThreads);
	int *this_count = (int*)malloc(sizeof(int)*countOfThreads);

	time_t start = clock();
	for (unsigned int i = 0; i < countOfThreads; i++) {

		this_count[i] = floor(count / countOfThreads);
		if (i == 0) {
			this_count[i] += (count % countOfThreads);
		}

		DWORD id;
		watek[i] = CreateThread(
			NULL,
			0,
			createRandomMessageThread,
			(void*)&this_count[i],
			0,
			&id
		);
	}
	WaitForMultipleObjects(countOfThreads, watek, TRUE, INFINITE);
	time_t stop = clock() - start;

	free(this_count);
	free(watek);

	printf("Czas tworzenia %i wiadomosc na %i watkach: %i\n", count, countOfThreads, stop);
	system("PAUSE");
}

void pastMessageFromClipboard() {
	if (!IsClipboardFormatAvailable(CF_TEXT)) {
		printf("Schowek jest pusty !\n");
		system("PAUSE");

		return;
	}
	
	HWND hwnd = GetConsoleWindow();
	OpenClipboard(hwnd);
	char *text = GetClipboardData(CF_TEXT);
	if (text == NULL) {
		CloseClipboard();
		printf("Schowek jest pusty !\n");
		system("PAUSE");

		return;
	}

	addNewMessage("Copy", text);

	CloseClipboard();
	printf("Skopiowano !\n");
	system("PAUSE");
}

struct Log* findLogByID(size_t id) {
	for (struct Log* log = root; log; log = log->prev) {
		if (log->id == id) {
			return log;
		}
	}
	return NULL;
}

void selectMessageToCopy() {

	if (!root) {
		printf("Brak wiadomosci \n");
		system("PAUSE");
		return;
	}

	size_t selected_ID = 0;
	const struct Log* selected_Log = NULL;
	do {
		printf("Podaj ID wiadomosci: ");
		scanf("%u", &selected_ID);

		if (selected_ID >= maxID) {
			printf("Wiadomosc o podanym ID nie istnieje !\n");
			continue;
		}

		selected_Log = findLogByID(selected_ID);
		if (selected_Log == NULL) {
			printf("Wiadomosc o podanym ID nie istnieje !\n");
			continue;
		}

	} while (selected_ID >= maxID || selected_Log == NULL);

	const size_t len = strlen(selected_Log->message) + 1;
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), selected_Log->message, len);
	GlobalUnlock(hMem);

	HWND hwnd = GetConsoleWindow();
	OpenClipboard(hwnd);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();

	printf("Skopiowano wiadomosc !\n");
	system("PAUSE");
}

struct reciveFromClientStruct {
	unsigned int index;
} *reciveFromStruct;

DWORD WINAPI receveFromClientThread(void* args) {
	char buffer[MAX_MESSAGE_LEN];

	unsigned int curret_index = ((struct reciveFromClientStruct*)args)->index;
	struct Client *current_client = &clients[curret_index];

	while (1) {

		buffer[0] = '\0';
		recv(current_client->clientSocket, buffer, MAX_MESSAGE_LEN, 0);
		copyText(buffer, current_client->clientName, MAX_USER_LEN);

		if (buffer[0] == '\0') {
			break;
		}

		buffer[0] = '\0';
		recv(current_client->clientSocket, buffer, MAX_MESSAGE_LEN, 0);

		if (buffer[0] == '\0') {
			break;
		}

		const char* user_name = current_client->clientName;
		for (unsigned int i = 0; i < MAX_CLIENTS; i++) {
			if (i != curret_index) {
				send(clients[i].clientSocket, user_name, strlen(user_name) + 1, 0);
				send(clients[i].clientSocket, buffer, strlen(buffer) + 1, 0);
			}
		}

		addNewMessage(user_name, buffer);
		if (isChatMod == 1) {
			system("cls");
			printf("0 - Powrot do menu\n\n");
			showMessages(SHOW_LIMIT);
			printf("\n");

			printf("%s: ", your_name);
		}
	}

	closesocket(current_client->clientSocket);
	current_client->clientName[0] = '\0';

	return 0;
}

DWORD WINAPI startServerThread() {

	WSADATA wsaData;
	struct sockaddr_in local;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Error WSAStartup");
		isOnlineMod = 0;
		WSACleanup();
		return 1;
	}

	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(3000);

	mySocket = socket(AF_INET, SOCK_STREAM, 0);

	if (mySocket == INVALID_SOCKET) {
		printf("INVALID_SOCKET");
		isOnlineMod = 0;
		WSACleanup();
		return 1;
	}

	if (bind(mySocket, (struct sockaddr*)&local, sizeof(local)) != 0)  {
		printf("BIND ERROR");
		isOnlineMod = 0;
		WSACleanup();
		return 1;
	}

	if (listen(mySocket, 10) != 0) 	{
		printf("LISTEN ERROR");
		isOnlineMod = 0;
		WSACleanup();
		return 1;
	}

	struct sockaddr_in from;
	int fromlen = sizeof(from);

	isOnlineMod = isServer = 1;
	printf("Server jest uruchomiony ! \n");

	while (client_index < MAX_CLIENTS) {

		unsigned int curret_index = (client_index++);

		clients[curret_index].clientSocket = accept(mySocket, &from, &fromlen);
		
		reciveFromStruct = malloc(sizeof(struct reciveFromClientStruct));
		reciveFromStruct->index = curret_index;

		DWORD id;
		clients[curret_index].watek = CreateThread(
			NULL,
			0,
			receveFromClientThread,
			(void*)reciveFromStruct,
			0,
			&id
		);

	}

	closesocket(mySocket);

	return 0;
}

void startServer() {
	DWORD id;
	watek_servera = CreateThread(
		NULL,
		0,
		startServerThread,
		(void*)NULL,
		0,
		&id
	);
	system("PAUSE");
}

DWORD WINAPI startClientThread() {

	WSADATA wsaData;
	SOCKADDR_IN ServerAddr, ThisSenderInfo;

	// Initialize Winsock version 2.2
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	printf("Client: Winsock DLL status is %s.\n", wsaData.szSystemStatus);

	mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mySocket == INVALID_SOCKET) {
		printf("Client: socket() failed! Error code: %ld\n", WSAGetLastError());
		isOnlineMod = 0;
		WSACleanup();
		return 1;
	}

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(3000);
	ServerAddr.sin_addr.s_addr = inet_addr(IP_ADDRES);

	if (connect(mySocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr)) != 0) {
		printf("Client: connect() failed! Error code: %ld\n", WSAGetLastError());
		closesocket(mySocket);
		isOnlineMod = 0;
		WSACleanup();
		return 1;
	}

	printf("Client jest uruchomiony ! \n");

	isOnlineMod = 1;
	char buffer_name[MAX_MESSAGE_LEN];
	char buffer[MAX_MESSAGE_LEN];

	while (1) {

		buffer_name[0] = '\0';
		recv(mySocket, buffer_name, MAX_USER_LEN, 0);

		if (buffer_name[0] == '\0') {
			closesocket(mySocket);
			isOnlineMod = 0;
			break;
		}

		buffer[0] = '\0';
		recv(mySocket, buffer, MAX_MESSAGE_LEN, 0);

		if (buffer[0] == '\0') {
			closesocket(mySocket);
			isOnlineMod = 0;
			break;
		}

		addNewMessage(buffer_name, buffer);
		if (isChatMod == 1) {
			system("cls");
			printf("0 - Powrot do menu\n\n");
			showMessages(SHOW_LIMIT);
			printf("\n");

			printf("%s: ", your_name);
		}
	}

	return 0;
}


void startClient() {
	DWORD id;
	watek_servera = CreateThread(
		NULL,
		0,
		startClientThread,
		(void*)NULL,
		0,
		&id
	);
	system("PAUSE");
}

void goodEnd() {
	removeAll();
	closesocket(mySocket);
	for (unsigned int i = 0; i < MAX_CLIENTS; i++) {
		closesocket(clients[i].clientSocket);
	}
	WSACleanup();
	exit(0);
}
	
void drawMenu() {
	system("cls");
	printf("1 - Otworz chat \n");
	printf("2 - Zmien swoja nazwe (%s) \n", your_name);
	printf("3 - Historia \n");
	printf("4 - Zmien czas usuwania wiadomosci \n");
	printf("5 - Usun wiadomosci starsze niz %u sekund \n", delete_time);
	printf("6 - Zmien ilosc watkow (%u) \n", countOfThreads);
	printf("7 - Wyszukaj \n");
	printf("8 - Stworz losowe wiadomosci \n");

	printf("---------------- \n");
	printf("a - Dodaj wiadomosc ze schowka \n");
	printf("b - Skopiuj wiadomosc do schowka \n");

	printf("---------------- \n");
	if (isOnlineMod == 0) {
		printf("s - Uruchom serwer \n");
		printf("c - Polacz sie z serwerem \n");
	}
	else if (isServer == 0) {
		printf("Aplikacja dziala jako klient \n");
	}
	else {
		printf("Aplikacja dziala jako serwer \n");
	}

	printf("---------------- \n");
	printf("0 - Wyjscie \n");
	printf("\n");
}

void readOptions() {
	char opcja;
	do {
		opcja = getchar();
	} while ((opcja < '0' || opcja > '9') && (opcja < 'a' || opcja > 'b') && ((opcja != 's' || opcja != 'c') && isOnlineMod != 0 ));

	printf("\n");	
	switch (opcja)
	{
	case '1':
		chatMod();
		break;
	case '2':
		changeName();
		break;
	case '3':
		showAllMessages();
		break;
	case '4':
		changeDeleteTime();
		break;
	case '5':
		removeOlder();
		break;
	case '6':
		changeThreadCount();
		break;
	case '7':
		FindInLogs();
		break;
	case '8':
		createRandomMessagesInit();
		break;
	case 'a':
		pastMessageFromClipboard();
		break;
	case 'b':
		selectMessageToCopy();
		break;
	case 's':
		startServer();
		break;
	case 'c':
		startClient();
		break;
	case '0':
		goodEnd();
	}
}



int main() {

	//srand(time(NULL));

	addNewMessage("SYS", "Witaj w chacie !!!\n");

	while (1) {
		drawMenu();
		readOptions();
	}

	return 0;
}