#include <SDL.h>
#include <iostream>
#include <stdint.h>

constexpr int windowWidth{ 640 };
constexpr int windowHeight{ 320 };

extern uint32_t display[64 * 32];
extern uint8_t memory[4096];

SDL_Window* gWindow{ nullptr };

SDL_Surface* gSurface{ nullptr };

void updateScreen();

bool init() {

	bool success{ true };
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_Log("Couldn't init the device");
		success = false;
	}
	else {
		gWindow = SDL_CreateWindow("CHIP-8", windowWidth, windowHeight, 0);

		if (gWindow == nullptr)
		{
			SDL_Log("Couldn't create the Window");
			success = false;
		}
		else {
			gSurface = SDL_GetWindowSurface(gWindow);
		}
	}

	SDL_FillSurfaceRect(gSurface, nullptr, SDL_MapSurfaceRGB(gSurface, 0x00, 0x00, 0x00));
	return true;
}


void update() {
	updateScreen();
	SDL_UpdateWindowSurface(gWindow);
}

void clearScreen() {
	SDL_FillSurfaceRect(gSurface, nullptr, SDL_MapSurfaceRGB(gSurface, 0x00, 0x00, 0x00));
}

void close() {

	SDL_DestroyWindow(gWindow);
	gWindow = nullptr;
	gSurface = nullptr;
	SDL_Quit();
}

/*
voglio vedere i cambiamenti frame per frame uno alla volta quindi aspetto cosi
se volessi vedere tutta l'immagine cambiare mi converrebbe prendere le informazioni del display e modificarle sotto
(penso?)
*/
//void drawScaledPixel(int x, int y, bool white) {
//
//	uint32_t* Pixels = (uint32_t*)gSurface->pixels;
//	uint32_t color{};
//
//	int pitch = gSurface->pitch / 4;
//
//	if (white == false) //bianco su nero quindi quando bianco è falso.  Se bianco è vero coloro i 10 pixel da colorare in bianco e quindi li riempio
//	{
//		color = SDL_MapRGB(SDL_GetPixelFormatDetails(gSurface->format), nullptr, 0xFF, 0xFF, 0xFF);
//	}
//	else {
//		color = SDL_MapRGB(SDL_GetPixelFormatDetails(gSurface->format), nullptr, 0x00, 0x00, 0x00);
//	}
//	
//
//	for (int i = 0; i < 10; i++)
//	{
//		for (int j = 0; j < 10; j++)
//		{
//			Pixels[((y) + i) * pitch + ((x) + j)] = color;
//			
//		}
//	}
//}

//uint8_t draw(uint8_t x8, uint8_t y8, uint8_t N8, uint16_t index16) {
//
//	uint8_t VF{0}; //valore da ritornare in caso di collisione: 0 se ho semplicemento acceso il pixel, 1 se il pixel l'ho spento
//	//lo metto 0 ora e poi lo cambio in caso serve cambiarlo dopo sennò rimane 0 e tutti felici :)
//	int x = x8;
//	int y = y8;
//	int N = N8;
//	int index = index16;
//
//	for (int row = 0; row < N; row++) // le righe sono definite dal comandy DXYN e decide l'altezza di quello che va disegnato
//	{   // ogni porcoddio di disegno è definito in una successione di memoria lunga quanto è alto il disegno quindi se N = 3 la prima riga sara un numero esadecimale da 8 bit Z il secondo V e così via e sti numeri si trovano nella memoria della "cassetta" ( non so che cazzo usavano ai tempi quindi la chiamo cassetta)
//		
//		if (row + y > 31) // se lo sprite va fuori dallo schermo in verticale smetto di disegnare
//		{ // row + x sarebbe per controllare se posso disegnare la riga corrente (se inizio a disegnare a 30 per 4 righe e arrivo al ciclio in cui voglio disegnare alla riga 30 + 3 non posso perche ne ho solo 32
//			break; // qua break perchè se sono fuori riga non posso andare a destra della colonna
//		}
//
//		uint8_t spriteByte = memory[index + row]; //qua prendo la prima riga dello sprite che è semplicemente un numero in esadecimale da 8 bit (esempio riga sotto)
//												  //lo sprite di e0 è: 1110 0000 dove gli 1 sono i pixel che cambiano stato e gli 0 fregacazzi
//		for (int col = 0; col < 8; col++) { //8 colonne 8 bit (se non capisci sei ritardato)
//
//			if (col + x > 63) // se lo sprite va fuori dallo schermo in laterale smetto di disegnare
//			{ // col + y sarebbe per controllare se posso disegnare la colonna corrente (se inizio a disegnare a 60 per 7 righe e arrivo al ciclio in cui voglio disegnare alla colonna 60 + 5 non posso perche ne ho solo 64
//				break; //break e non return perchè posso comunque andare in basso
//			}
//
//			std::cout << "Sprite: " << std::hex << static_cast<int>(spriteByte) << std::endl;
//			//0x80 == 1000 0000
//			uint8_t spritePixel = spriteByte & (0x80 >> col); // controllo ogni cazzo di bit da sinistra verso destra per vedere se sono accessi e prendo il bit 1 di 0x80 e lo sposto per controllare gli altri in successione nel for
//
//			std::cout << "Sprite Pixel: " << std::hex << static_cast<int>(spritePixel) << std::endl;
//			//display salva lo schermo in 64x32 non scalato
//			uint32_t *displayPixel = &display[(y + row) * 64 + (col+x)]; //prendo il pixel di display per vedere più avanti il suo stato
//
//
//			int pixelPosition = (y + row) * 64 + (col + x);
//
//
//			std::cout << "Stampo al Pixel: " << std::dec << pixelPosition << std::endl;
//			std::cout << "Display Pixel: " << std::hex << static_cast<int>(*displayPixel) << std::endl;
//
//			//POI LO FACCIO
//			//sta cosa è confusionaria un sacco e sono abbastanza ritardato ma non c'ho assolutamente voglia di cambiarla cazzi tua 
//			bool white{ false }; //da salvare per vedere di che cazzo di colore devo mettere lo schermo 
//			//è al contrario quindi white = vuoto = false e nero = riempito = true per evitare confusioni (è comunque confusionario)
//			
//			if (spritePixel != 0) //se il pixel dello sprite è 1 
//			{
//				if (*displayPixel != 0) //se il pixel del display è pieno cambio il valore di "white" in riempito che sarebbe true
//				{
//					white = true; //bianco è vero perchè immagino nero su bianco qua (anche se lo schermo è bianco su nero) (un pò complicato ma sticazzi)
//					VF = 1;
//				}
//				//accendo in entrambi i casi quindi basta una sola volta
//				//commento inutile sopra comunque 
//				//è uno XOR bit a bit e lo faccio solo con il bit selezionato
//				*displayPixel ^= 0x1;
//
//				drawScaledPixel(x*10 + col*10 , y*10 + row*10, white);
//			}
//		}
//	}
//
//	//stampo su console per vedere se funziona, se non funziona piango
//	for (int i = 0; i < 32; i++)
//	{
//		for (int j = 0; j < 64; j++)
//		{
//			if (display[i * 64 + j] == 1)
//			{
//				std::cout << "1";
//			}
//			else {
//				std::cout << "0";
//			}
//		}
//		std::cout << std::endl;
//	}
//	return VF;
//}


void drawScaledPixel(int column, int row, bool pixelON) {
	uint32_t* Pixels = (uint32_t*) gSurface->pixels;
	uint32_t color{};
	int pitch = gSurface->pitch / 4;

	if (pixelON)
	{
		color = SDL_MapRGB(SDL_GetPixelFormatDetails(gSurface->format), nullptr, 0xFF, 0xFF, 0xFF);
	}
	else {
		color = SDL_MapRGB(SDL_GetPixelFormatDetails(gSurface->format), nullptr, 0x00, 0x00, 0x00);
	}

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			int coordinates = (row + i) * pitch + (column + j);
			Pixels[coordinates] = color;
		}
	}
}

void updateScreen() {
	for (int row = 0; row < 32; row++)
	{
		for (int column = 0; column < 64; column++)
		{
			if (display[column + row * 64] == 1) {
				drawScaledPixel(column * 10, row * 10, true);
				std::cout << "0";
			}
			else {
				drawScaledPixel(column * 10, row * 10, false);
				std::cout << "1";
			}
		}
		std::cout << "\n";
	}
	std::cout << "\n\n\n\n";
}
