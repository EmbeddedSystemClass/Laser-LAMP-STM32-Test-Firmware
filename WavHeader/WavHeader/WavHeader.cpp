#include <iostream> 
#include <Windows.h>
#include <fstream> 
#include <stdio.h>


using namespace std;


void main(void)
{
	setlocale(LC_ALL, "Russian"); //Локаль
	
	ifstream file("laser.wav", ios::beg | ios::in | ios::binary); //Наша wav'ка
	if (!file) cout << "Не удалось открыть файл" << endl;


	DWORD dwRIFF = 0; //И так идет формат wav-файла, он относится к RIFF-файлам, поэтому читаем 4 байта структуы
	file.read((char*)&dwRIFF, 4);
	if (dwRIFF != MAKEFOURCC('R', 'I', 'F', 'F')) //Если RIFF не получили, то значит какую то залепу открыли
	{
		cout << "Файл не является RIFF - formated" << endl;

	}

	file.ignore(4); //Пропустим 4 байта там какая то охинея нам совершенно не нужна
	DWORD dwWave = 0; //Секция WAVE
	DWORD dwFormat = 0; // Секция fmt /формат файла/
	long  lSizeFmt = 0; //Размер секции fmt

	file.read((char*)&dwWave, 4); //Читаем 4 байта, там для Wav-файла должно быть название секции WAVE
	if (dwWave != MAKEFOURCC('W', 'A', 'V', 'E')) //Проверим
	{
		cout << "Файл не имеет секции WAVE" << endl;

	}

	file.read((char*)&dwFormat, 4); //Если все ОК... То читаем 
	if (dwFormat != MAKEFOURCC('f', 'm', 't', ' ')) //Если секция fmt - не найдена 
	{
		cout << "Файл не имеет Format(fmt)-Секции " << endl;

	}
	file.read((char*)&lSizeFmt, 4); //Еще читаем 4 байта, там у нас размер секции формат, где идет сам формат + может идти всякая охинея, формат лежит в 16 байтах
	WAVEFORMATEX WaveFormat = WAVEFORMATEX(); //Определим структуру винды
	file.read((char*)&WaveFormat, 16); //Прочитаем ее
	file.ignore(lSizeFmt - 16); //Пропустим охинею
	if (WaveFormat.wFormatTag != 1) //Если таг <>1 то значит есть компрессия файла, ее разбирать мы не будем, работаем не с жатыми данными
	{
		cout << "Файл имеет компрессию" << endl;

	}

	DWORD dwSection = 0; //Следующая секция у нас или fact(Не обязательная) или data(это как раз наш звук)
	file.read((char*)&dwSection, 4); //Читаем заголовок
	if (dwSection == MAKEFOURCC('f', 'a', 'c', 't')) //Если все же fact - то пропускаем ее
	{
		cout << "Файл имеет секцию fact" << endl;
		DWORD dwSizeFact = 0;
		file.read((char*)&dwSizeFact, 4);
		file.ignore(dwSizeFact);
		file.read((char*)&dwSection, 4);
	}


	if (dwSection != MAKEFOURCC('d', 'a', 't', 'a')) //Ну а теперь сама data - это то, над чем мы будем работать
	{
		cout << "Не найдена секция data" << endl;

	}
	DWORD dwDataSize = 0;
	file.read((char*)&dwDataSize, 4); //Прочитаем ее размер
	short *pData = new short[dwDataSize/2]; //Динамически выделим переменную типа char[размер звуковых данных]


	file.read((char*)pData, dwDataSize); //Прочитаем в нее все звуковые данные
	file.close(); //Закроем файл, он нам уже не нужен, у нас есть pData

	FILE* fp;
	fopen_s(&fp, "laser.txt", "w");
	int Size = dwDataSize / 2;

	fprintf(fp, "uint16_t laser_wav[%d] = {\n", Size);
	for (int i = 0; i < Size / 16; i++)
	{
		for (int j = 0; j < 16; j++)
			fprintf(fp, "\t%d,", (32768 + pData[i * 16 + j]) >> 8);
		fprintf(fp, "\n");
	}
	fprintf(fp, "};\n");

	fclose(fp);

	fopen_s(&fp, "data.txt", "w");
	for (int i = 0; i < Size; i++)
	{
		fprintf(fp, "%d\n", (32768 + pData[i]) >> 8);
	}

	fclose(fp);
}