#include <iostream> 
#include <Windows.h>
#include <fstream> 
#include <stdio.h>

using namespace std;

short *pData = NULL;
DWORD dwDataSize = 0;

void data_16()
{
	FILE* fp;
	fopen_s(&fp, "laser.txt", "w");
	int Size = dwDataSize / 2;

	fprintf(fp, "const uint16_t sound_seample[%d] = {\n", Size);
	for (int i = 0; i < Size / 16; i++)
	{
		for (int j = 0; j < 16; j++)
			fprintf(fp, "\t%d,", (32768 + pData[i * 16 + j]) >> 6);
		fprintf(fp, "\n");
	}
	fprintf(fp, "};\n");

	fclose(fp);

	fopen_s(&fp, "data.txt", "w");
	for (int i = 0; i < Size; i++)
	{
		fprintf(fp, "%d\n", (32768 + pData[i]) >> 8);
	}
}

void data_8()
{
	unsigned char* pdata = (unsigned char*)pData;

	FILE* fp;
	fopen_s(&fp, "laser.txt", "w");
	int Size = dwDataSize;

	fprintf(fp, "const uint16_t sound_seample[%d] = {\n", Size);
	for (int i = 0; i < Size / 16; i++)
	{
		for (int j = 0; j < 16; j++)
			fprintf(fp, "\t%d,", pdata[i * 16 + j] * 8);
		fprintf(fp, "\n");
	}
	fprintf(fp, "};\n");

	fclose(fp);

	fopen_s(&fp, "data.txt", "w");
	for (int i = 0; i < Size; i++)
	{
		fprintf(fp, "%d\n", pdata[i]);
	}
}

void main(void)
{
	setlocale(LC_ALL, "Russian"); //������
	
	ifstream file("ghost.wav", ios::beg | ios::in | ios::binary); //���� wav'��
	if (!file) cout << "�� ������� ������� ����" << endl;


	DWORD dwRIFF = 0; //� ��� ���� ������ wav-�����, �� ��������� � RIFF-������, ������� ������ 4 ����� ��������
	file.read((char*)&dwRIFF, 4);
	if (dwRIFF != MAKEFOURCC('R', 'I', 'F', 'F')) //���� RIFF �� ��������, �� ������ ����� �� ������ �������
	{
		cout << "���� �� �������� RIFF - formated" << endl;

	}

	file.ignore(4); //��������� 4 ����� ��� ����� �� ������ ��� ���������� �� �����
	DWORD dwWave = 0; //������ WAVE
	DWORD dwFormat = 0; // ������ fmt /������ �����/
	long  lSizeFmt = 0; //������ ������ fmt

	file.read((char*)&dwWave, 4); //������ 4 �����, ��� ��� Wav-����� ������ ���� �������� ������ WAVE
	if (dwWave != MAKEFOURCC('W', 'A', 'V', 'E')) //��������
	{
		cout << "���� �� ����� ������ WAVE" << endl;

	}

	file.read((char*)&dwFormat, 4); //���� ��� ��... �� ������ 
	if (dwFormat != MAKEFOURCC('f', 'm', 't', ' ')) //���� ������ fmt - �� ������� 
	{
		cout << "���� �� ����� Format(fmt)-������ " << endl;

	}
	file.read((char*)&lSizeFmt, 4); //��� ������ 4 �����, ��� � ��� ������ ������ ������, ��� ���� ��� ������ + ����� ���� ������ ������, ������ ����� � 16 ������
	WAVEFORMATEX WaveFormat = WAVEFORMATEX(); //��������� ��������� �����
	file.read((char*)&WaveFormat, 16); //��������� ��
	file.ignore(lSizeFmt - 16); //��������� ������
	if (WaveFormat.wFormatTag != 1) //���� ��� <>1 �� ������ ���� ���������� �����, �� ��������� �� �� �����, �������� �� � ������ �������
	{
		cout << "���� ����� ����������" << endl;

	}

	DWORD dwSection = 0; //��������� ������ � ��� ��� fact(�� ������������) ��� data(��� ��� ��� ��� ����)
	file.read((char*)&dwSection, 4); //������ ���������
	if (dwSection == MAKEFOURCC('f', 'a', 'c', 't')) //���� ��� �� fact - �� ���������� ��
	{
		cout << "���� ����� ������ fact" << endl;
		DWORD dwSizeFact = 0;
		file.read((char*)&dwSizeFact, 4);
		file.ignore(dwSizeFact);
		file.read((char*)&dwSection, 4);
	}


	if (dwSection != MAKEFOURCC('d', 'a', 't', 'a')) //�� � ������ ���� data - ��� ��, ��� ��� �� ����� ��������
	{
		cout << "�� ������� ������ data" << endl;

	}
	file.read((char*)&dwDataSize, 4); //��������� �� ������
	pData = new short[dwDataSize/2]; //����������� ������� ���������� ���� char[������ �������� ������]


	file.read((char*)pData, dwDataSize); //��������� � ��� ��� �������� ������
	file.close(); //������� ����, �� ��� ��� �� �����, � ��� ���� pData

	data_8();

	FILE* fp = 0;
	fopen_s(&fp, "wave.txt", "w");
	int Size = 64;
	fprintf(fp, "const uint16_t sound_seample[%d] = {\n", Size);
	for (int i = 0; i < Size / 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			float y = 256.0f + 255.0f * sin(2.0f * 3.14f * float(i * 16 + j) / float(Size));
			fprintf(fp, "\t%d,", uint16_t(y));
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "};\n");

	fclose(fp);
}