#include "Chip_8.h"

Chip_8::Chip_8(ShiftMode shiftmode, std::string ROM) : shiftmode(shiftmode) {

	if (!OpenROM(ROM)) {
		//throw error?
		std::cout << "problems opening the ROM! Retry";
	}

	srand(static_cast<unsigned int>(time(NULL)));
	LoadFont();
}

void Chip_8::LoadFont() {

	for (int i = 0; i < 80; i++)
	{
		memory[FONT_START + i] = font[i];
	}
}

bool Chip_8::OpenROM(const std::string pathROM) {

	std::ifstream ROMfile{ pathROM, std::ios::binary | std::ios::ate };

	if (ROMfile.is_open())
	{
		int size{ static_cast<int>(ROMfile.tellg()) };
		char* buffer = new char[size];

		ROMfile.seekg(0, std::ios::beg);
		ROMfile.read(buffer, size);

		if (size > 4096 - 512)
		{
			return false;
		}

		for (int i = 0; i < size; i++)
		{
			memory[PROGRAM_START + i] = buffer[i];
		}
		programCounter = PROGRAM_START;
		return true;
	}

	return false;
}

void Chip_8::fetch() {
	opcode = memory[programCounter];
	opcode = opcode << 8;
	opcode = opcode | memory[programCounter + 1];
	programCounter += 2;
}

uint16_t Chip_8::getXfromOPC(uint16_t OPC) {
	return (OPC & 0x0F00) >> 8;
}
uint16_t Chip_8::getYfromOPC(uint16_t OPC) {
	return (OPC & 0x00F0) >> 4;
}

void Chip_8::OPC0x00E0() {
	//clear display
	clearScreen();
	for (int row = 0; row < 32; row++)
	{
		for (int column = 0; column < 64; column++)
		{
			display[column + 64 * (row)] = 0;
		}
	}
}

void Chip_8::OPC0x00EE() {
	programCounter = stack.back();
	stack.pop_back();
}

void Chip_8::OPC0x1NNN(uint16_t OPC) {
	programCounter = OPC & 0x0FFF;
}

void Chip_8::OPC0x2NNN(uint16_t OPC) {
	stack.push_back(programCounter);
	programCounter = OPC & 0x0FFF;

}

void Chip_8::OPC0x3XNN(uint16_t OPC, uint16_t XIndex) {

	bool condition = (registers[XIndex] == (OPC & 0x00FF));
	if (condition)
	{
		programCounter += 2;
	}
}

void Chip_8::OPC0x4XNN(uint16_t OPC, uint16_t XIndex) {
	bool condition = (registers[XIndex] != (OPC & 0x00FF));
	if (condition)
	{
		programCounter += 2;
	}
}

void Chip_8::OPC0x5XY0(uint16_t OPC, uint16_t XIndex, uint16_t YIndex) {

	if (registers[XIndex] == registers[YIndex])
	{
		programCounter += 2;
	}
}

void Chip_8::OPC0x6XNN(uint16_t OPC, uint16_t XIndex) {
	registers[XIndex] = (OPC & 0x00FF);
}

void Chip_8::OPC0x7XNN(uint16_t OPC, uint16_t XIndex) {
	registers[XIndex] += (OPC & 0x00FF);
}

void Chip_8::OPC0x8XY0(uint16_t OPC, uint16_t XIndex, uint16_t YIndex) {

	registers[XIndex] = registers[YIndex];
}

void Chip_8::OPC0x8XY1(uint16_t OPC, uint16_t XIndex, uint16_t YIndex, ShiftMode shiftmode) {

	registers[XIndex] = registers[XIndex] | registers[YIndex];
	if (shiftmode == COSMAC_VIP)
	{
		registers[0xF] = 0;
	}
}

void Chip_8::OPC0x8XY2(uint16_t OPC, uint16_t XIndex, uint16_t YIndex, ShiftMode shiftmode) {

	registers[XIndex] = registers[XIndex] & registers[YIndex];

	if (shiftmode == COSMAC_VIP)
	{
		registers[0xF] = 0;
	}
}

void Chip_8::OPC0x8XY3(uint16_t OPC, uint16_t XIndex, uint16_t YIndex, ShiftMode shiftmode) {

	registers[XIndex] = registers[XIndex] ^ registers[YIndex];

	if (shiftmode == COSMAC_VIP)
	{
		registers[0xF] = 0;
	}
}

void Chip_8::OPC0x8XY4(uint16_t OPC, uint16_t XIndex, uint16_t YIndex) {

	bool overflow = registers[XIndex] + registers[YIndex] > 255;

	registers[XIndex] = registers[XIndex] + registers[YIndex];

	registers[0xF] = 0x0;

	if (overflow)
	{
		registers[0xF] = 0x1;
	}
}

void Chip_8::OPC0x8XY5(uint16_t OPC, uint16_t XIndex, uint16_t YIndex) {

	bool underflow = registers[XIndex] < registers[YIndex];
	registers[XIndex] = registers[XIndex] - registers[YIndex];

	if (underflow) {
		registers[0xF] = 0x0;
	} else {
		registers[0xF] = 0x1;
	}


}

void Chip_8::OPC0x8XY7(uint16_t OPC, uint16_t XIndex, uint16_t YIndex) {

	bool underflow = registers[YIndex] < registers[XIndex];
	registers[XIndex] = registers[YIndex] - registers[XIndex];

	if (underflow) {
		registers[0xF] = 0x0;
	} else {
		registers[0xF] = 0x1;
	}

}

void Chip_8::OPC0x8XY6(uint16_t OPC, uint16_t XIndex, uint16_t YIndex, ShiftMode shiftmode) {

	if (shiftmode == COSMAC_VIP) {
		registers[XIndex] = registers[YIndex];
	}

	uint16_t shiftvalue = registers[XIndex] & 0x01;

	registers[XIndex] = registers[XIndex] >> 1;

	if (shiftvalue == 1) {
		registers[0xF] = 0x1;
	} else {
		registers[0xF] = 0x0;
	}

}

void Chip_8::OPC0x8XYE(uint16_t OPC, uint16_t XIndex, uint16_t YIndex, ShiftMode shiftmode) {

	if (shiftmode == COSMAC_VIP) {
		registers[XIndex] = registers[YIndex];
	}

	uint16_t shiftvalue = ((registers[XIndex] >> 7) & 0x1);

	registers[XIndex] = registers[XIndex] << 1;

	if (shiftvalue == 1) {
		registers[0xF] = 0x1;
	} else {
		registers[0xF] = 0x0;
	}
}

void Chip_8::OPC0x9XY0(uint16_t OPC, uint16_t XIndex, uint16_t YIndex) {

	if (registers[XIndex] != registers[YIndex])
	{
		programCounter += 2;
	}

}

void Chip_8::OPC0xANNN(uint16_t OPC) {
	indexRegister = OPC & 0x0FFF;
}

void Chip_8::OPC0xBNNN(uint16_t OPC, uint16_t XIndex, ShiftMode shiftmode) {

	uint16_t NNN = OPC & 0x0FFF;

	if (shiftmode == COSMAC_VIP) {
		programCounter = NNN + registers[0];
	} else {
		programCounter = NNN + registers[XIndex];
	}
}

void Chip_8::OPC0xCXNN(uint16_t OPC, uint16_t XIndex) {

	uint16_t NN = OPC & 0x00FF;
	registers[XIndex] = (rand() % 256) & NN;
}

void Chip_8::OPC0xDXYN(uint16_t OPC, uint16_t XIndex, uint16_t YIndex) {
	//coordinate
	uint8_t x = XIndex & 63;
	uint8_t y = YIndex & 31;
	uint8_t n = OPC & 0x000F; //sprite height
	registers[0xF] = 0;

	for (int row = 0; row < n; row++) {
		uint8_t spriteData = memory[indexRegister + row];

		if (y + row > 31) {
			break;
		}

		for (int column = 0; column < 8; column++) {
			if (x + column > 63) {
				break;
			}

			bool isPixelSpriteOn = (spriteData & (0x80 >> column)) != 0;

			if (!isPixelSpriteOn) {
				continue;
			}

			int coordinates = (y + row) * 64 + (x + column);


			if (display[coordinates] && isPixelSpriteOn) {
				registers[0xF] = 1;
				//drawScaledPixel((x + column) * 10, (y + row) * 10, false);
			} else {
				//drawScaledPixel((x + column) * 10, (y + row) * 10, true);
			}

			display[coordinates] ^= 1;
		}
	}


}

