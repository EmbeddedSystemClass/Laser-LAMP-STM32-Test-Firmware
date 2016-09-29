#include <iostream> 
#include <Windows.h>
#include <fstream> 
#include <stdio.h>


using namespace std;


void main(void)
{
	setlocale(LC_ALL, "Russian"); //������
	
	ifstream file("laser.wav", ios::beg | ios::in | ios::binary); //���� wav'��
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
	DWORD dwDataSize = 0;
	file.read((char*)&dwDataSize, 4); //��������� �� ������
	short *pData = new short[dwDataSize/2]; //����������� ������� ���������� ���� char[������ �������� ������]


	file.read((char*)pData, dwDataSize); //��������� � ��� ��� �������� ������
	file.close(); //������� ����, �� ��� ��� �� �����, � ��� ���� pData

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