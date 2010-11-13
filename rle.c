/*
 * RLE - kompresja i dekompresja pliku.
 * Wykonał: K.S.
 * 
 * Copyrights 2010 (c) K.S
 * 
 * wersja: 0.3a (beta)
 * 
 * Zwracane kody bledow:
 * -1 - Nieprawidlowy parametr
 *  0 - Wszystko OK
 *  1 - Blad dostepu do pliku
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


void kompresja(FILE *wej, FILE *wyj) {
	unsigned char poprzedni, obecny, nastepny, znak, minus[2];
	unsigned short int licznik;
	long int sPoz, kPoz, ilosc, i;

	licznik		= 0;
	poprzedni	= 0;
	obecny		= 0;
	nastepny	= 0;
	minus[0]	= 0x00;
	minus[1]	= 0x01;
	printf("Kompresja\n");

	while ( fread(&obecny, sizeof(char), 1, wej) == 1 ) {

		if ( licznik == 0 ) {
			poprzedni = obecny;
			licznik = 1;
			continue;
		}

		/* Jesli mamy powtarzajacy sie bajt */
		if ( poprzedni == obecny ) {
			if ( licznik == 255 ) {
				fwrite(&licznik, sizeof(char), 1, wyj);
				fwrite(&poprzedni, sizeof(char), 1, wyj);
				licznik = 1;
			}
			else ++licznik;
		}

		/* Nie ma powtarzajacego sie bajtu (sasiednie sa rozne) */
		else {
			if ( licznik > 1 ) {
				fwrite(&licznik, sizeof(char), 1, wyj);
				fwrite(&poprzedni, sizeof(char), 1, wyj);
				licznik = 1;

				/* Jesli to nie koniec pliku to nastepny przebieg petli */
				if ( fread(&nastepny, sizeof(char), 1, wej) == 1 ) {
					fseek(wej, -2, SEEK_CUR);
					licznik = 0;
					continue;					
				}
				/* Nie ma kolejnego znaku, osiagnelismy koniec pliku. */
				else {
					fwrite(&minus, sizeof(char), 2, wyj);
					fwrite(&obecny, sizeof(char), 1, wyj);
					licznik = 0;
					break;
				}
			}

			/* Szukamy ile i jakie znaki wystepuja pojedynczo. */
			sPoz = ftell(wej) - 2;

			while ( poprzedni != obecny ) {
				if ( (ftell(wej) - sPoz) == 255 ) break;
				znak = poprzedni;
				poprzedni = obecny;
				fread(&obecny, sizeof(char), 1, wej);
				if ( feof(wej) ) { poprzedni = znak; break; }
			}
			
			kPoz = ( poprzedni == obecny ) ? ftell(wej) - 2 : ftell(wej);
			ilosc = kPoz-sPoz;
			fwrite(&minus[0], sizeof(char), 1, wyj);
			fwrite(&ilosc, sizeof(char), 1, wyj);
			fseek(wej, sPoz, SEEK_SET);
			for (i=sPoz; i<kPoz; ++i) {
				fread(&znak, sizeof(char), 1, wej);
				fwrite(&znak, sizeof(char), 1, wyj);
			}

			licznik = 0;
		}

		poprzedni = obecny;
	}
	if ( feof(wej) && licznik > 0 ) {
		fwrite(&licznik, sizeof(char), 1, wyj);
		fwrite(&poprzedni, sizeof(char), 1, wyj);
		licznik = 0;
	}
}


/* Funkcja dekompresujaca plik RLE */
void dekompresja(FILE *wej, FILE *wyj) {
	unsigned char obecny, znak, znaki[255];
	unsigned short int i;

	printf("Dekompresja\n");
	while ( fread(&obecny, sizeof(char), 1, wej) == 1 ) {

		/* 
		 * Jezeli odczytany w petli bajt to 0x00, tzn.
		 * ze kolejny bajt okresla dlugosc ciagu, ktory trzeba odczytac
		 * i 'wyswietlic' bez powtarzania.
		 */
		if ( obecny == 0x00) {
			fread(&obecny, sizeof(char), 1, wej);
			fread(&znaki, sizeof(char), obecny, wej);
			for (i=0; i<obecny; ++i) fwrite(&znaki[i], sizeof(char), 1, wyj);
		}

		/* 
		 * Jezeli odczytany w glownej petli bajt NIE JEST 0x00 tzn.,
		 * ze JEST to liczba powtorzen nastepnego bajtu.
		 */
		else {
			fread(&znak, sizeof(char), 1, wej);
			for (i=0; i<obecny; ++i) fwrite(&znak, sizeof(char), 1, wyj);
		}
	}
}

int main(int argc, char *argv[]) {
	char w[2];
	char wejPlik[255], wyjPlik[255];
	FILE *wej, *wyj;

	/* Jezeli podano niewlasciwa liczbe argumentow to wyswietlamy jak sie tego uzywa. */
	if (argc < 3 || argc > 4) {
		printf("Uzyj: rle <plik_wejsciowy> <plik_wyjsciowy> [OPCJA]\n\n");
		printf("Opcje mozna pominac. Do wyboru sa nastepujace:\n\n");
		printf("\tOPCJA\t\tOPIS\n\n");
		printf("\tk|c\t\tKompresja pliku\n\td\t\tDekompresja pliku\n");
		exit(-1);
	}

	/* Pobranie nazw plikow we/wy z parametrow nr 1 i 2 */
	strncpy(wejPlik, argv[1], sizeof wejPlik - 1); wejPlik[sizeof wejPlik -1 ] = 0;
	strncpy(wyjPlik, argv[2], sizeof wyjPlik - 1); wyjPlik[sizeof wyjPlik -1 ] = 0;

	/* Otwarcie plikow wejsciowego i wyjsciowego. */
	if ( (wej = fopen(wejPlik, "rb")) == NULL ) {
		perror("Blad podczas dostepu do pliku wejsciowego");
		exit(1);
	}
	if ( (wyj = fopen(wyjPlik, "wb")) == NULL ) {
		perror("Blad podczas dostepu do pliku wyjsciowego");
		exit(1);
	}

	/* Krotkie info o autorze. */
	printf("RLE - kompresja i dekompresja pliku.\n\nProgram napisany na zaliczenie projektu z AiSD.\nWykonali: Krzysztof Smoliński & Łukasz Stanecki.\n\n");
	
	/* Jezeli podano 3 parametr to przypisujemy go do 'w'. A jak nie to wyswietlamy menu. */
	if ( argc == 4 ) strncpy(w, argv[3], 1);
	else {
		printf("Wybierz dzialanie:\nK - Kompresja\nD - Dekompresja\nW - Wyjscie\nWybieram [k/d/W]: ");
		scanf("%1c", &w[0]);
	}

	/* Wybor pomiedzy kompresja i dekompresja, domyslnie wyjscie z programu */	
	switch (w[0]) {
		case 'K':
		case 'k':
		case 'C':
		case 'c':
			kompresja(wej, wyj);
			break;
		case 'D':
		case 'd':
			dekompresja(wej, wyj);
			break;
		default:
			printf("Bye bye :(\n");
	}

	/* Zamkniecie plikow wejsciowego i wyjsciowego. */
	fclose(wej);
	fclose(wyj);

	return EXIT_SUCCESS;
}
