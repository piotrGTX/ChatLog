#define _CRT_SECURE_NO_WARNINGS

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_USER_LEN 20U
#define MAX_MESSAGE_LEN 100U
#define DEFAULT_NAME "piotrGTX"
#define DEFAULT_DELETE_TIME 10U
#define SHOW_LIMIT 10U

struct Log {
	char user[MAX_USER_LEN];			
	char message[MAX_MESSAGE_LEN];
	time_t date;

	struct Log *prev;
} *root = NULL;

char your_name[MAX_USER_LEN] = DEFAULT_NAME;
size_t delete_time = DEFAULT_DELETE_TIME;

void copyText(const char* source, char* desc, size_t limit) {
	size_t i = 0;
	while(i < (limit-1) && source[i] != '\0') {
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
	printf("Entery aby powrot do menu\n\n");

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
	scanf("%zu", &delete_time);
}

void chatMod() {
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
			break;
		}

		addNewMessage(your_name, new_message);
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

void drawMenu() {
	system("cls");
	printf("1 - Otworz chat \n");
	printf("2 - Zmien swoja nazwe (%s) \n", your_name);
	printf("3 - Historia \n");
	printf("4 - Zmien czas usuwania wiadomosci \n");
	printf("5 - Usun wiadomosci starsze niz %zu sekund \n", delete_time);
	printf("6 - Wyjscie \n");
	printf("\n");
}

void readOptions() {
	char opcja;
	do {
		opcja = getchar();
	} while (opcja < '1' || opcja > '6');

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
		removeAll();
		exit(0);
	}
}

void createRandomMessages(unsigned int ilosc) {

	srand(time(0));


	while (ilosc--) {
		char wiadomosc[15];
		for (int i = 0; i < 15; i++) {
			char new_char = floor(rand() % 10) + 'a';
			wiadomosc[i] = new_char;
		}
		wiadomosc[14] = '\0';
		addNewMessage("Piotr", wiadomosc);
	}
}