#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
using namespace std;

#pragma warning(disable: 4996)


SOCKET Sockets[10];
int SockCount = 0;
int MaxClientsQuantity = 10; //максимальное колличество клиентов, которые могут быть подключены

//глобальные переменные для формирования сообщения


int CharToInt(char* str)
{
	int result = 0;
	while (*str != '\0')
	{
		result *= 10;
		result += *str - '0';
		++str;
	}

	return result;
}


void SentParameters(int index) {

	char Descriprion1[20] = "Имя компьютера: ";
	char Descriprion2[45] = " , Имя пользователя данного компьютера: ";

	char UserName[MAX_COMPUTERNAME_LENGTH + 1];
	char CompName[60];

	//получение сообщений от клиента  с координатами перемещения
	char xbuf[256];
	recv(Sockets[index], xbuf, sizeof(xbuf), NULL); // сокет, буфер для прием, размер буфера, флаги
	char ybuf[256];
	recv(Sockets[index], ybuf, sizeof(ybuf), NULL);
	int x = CharToInt(xbuf);
	int y = CharToInt(ybuf);
	
	//перемещение окна возвращает булевое значение
	string resultMove = "Результат перемещения окна: " + to_string(MoveWindow(GetConsoleWindow(), x, y, 500, 400, TRUE));
	char mess1[256];
	//отправка результата перемещения
	strcpy(mess1, resultMove.c_str());
	send(Sockets[index], mess1, sizeof(mess1), NULL);

	DWORD size = sizeof(CompName);
	DWORD size2 = sizeof(UserName);

	//получение параметров
	GetComputerNameA(CompName, &size);
	GetUserNameA(UserName, &size2);

	//перобразование к массиву символов для отправки сообщения
	char mess2[256];
	strcpy(mess2, Descriprion1);
	strcat(mess2, CompName);
	strcat(mess2, Descriprion2);
	strcat(mess2, UserName);

	//отправка
	send(Sockets[index], mess2, sizeof(mess2), NULL);

	//вывод времени отправки
	auto timesent = std::chrono::system_clock::now();
	std::time_t timeSent = std::chrono::system_clock::to_time_t(timesent);
	std::cout << "Отправка сообщения клиенту №"<< index+1 <<" выполнена в " << std::ctime(&timeSent) << "\n";

}



int main(int argc, char* argv[]) {

	//устаовка русского языка на консоли
	setlocale(LC_ALL, "Russian");
	//Загрузка библиотеки 
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	HANDLE hMutex = CreateMutex(NULL, TRUE, L"MutexForServer1");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		cout << "Уже есть один экземпляр сервера!";
		exit(0);
	}

	//создание адреса
	SOCKADDR_IN addr;// структура для хранения адреса
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");//ip - local host
	addr.sin_port = htons(1111);//незарезервированный хост
	addr.sin_family = AF_INET;//семейство адресов в данном случае Ipv4 

	//инициализация сокета посредством функции socket()
	SOCKET sListen = socket(
		AF_INET,// семейство используемых адресов
		SOCK_STREAM,//протокл соединения TCP
		NULL); // тип протокола, если UDP или TCP, то оставляем NULL

	//привязка сокетов к адресу
	bind(sListen,(SOCKADDR*)&addr,sizeof(addr));

    
	//ожидание соединения
	listen(sListen, MaxClientsQuantity); // первый аргумент - "слушающий" сокет, второй аргумент - макс. кол-во процессов для подкл.

	SOCKET newConnection;
	for (int i = 0; i < MaxClientsQuantity; i++) {

		//для каждого нового соединения открывается новый поток для отправки сообщений
		//в него передается функция отправки (LPTHREAD_START_ROUTINE)SentParameters и номер соединения (LPVOID)(i) по которому отправлять
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newConnection == 0) {
			std::cout << "Error connection with client " <<i+1<<"\n";
		}
		else 
		{
			std::cout << "Client "<<i+1<< " connected!"<<"\n";
			Sockets[i] = newConnection;
			SockCount++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SentParameters, (LPVOID)(i), NULL, NULL);
		}
	}

	system("pause");
	return 0;
}