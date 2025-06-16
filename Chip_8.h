#ifndef CHIP_8
#define CHIP_8
#include <iostream>
#include <stdint.h>
#include <vector>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <time.h> 
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>

class Chip_8 {
public:
	enum ShiftMode {
		COSMAC_VIP,
		SUPER_CHIP,
		UNKNOWN,
	};

	Chip_8(ShiftMode shiftmode, std::string ROM);

private:

	enum MemoryMap {
		PROGRAM_START = 0x200,
		FONT_START = 0x50,
		FONT_END = 0x9F,
	};

	//const std::string pathROM{};

	const ShiftMode shiftmode{};
	uint16_t opcode{};
	uint8_t memory[4096]{};
	uint16_t programCounter{};
	uint16_t indexRegister{};
	std::vector<uint16_t> stack{};

	uint8_t delayTimer{};
	uint8_t soundTimer{};

	uint8_t registers[16]{};

	uint32_t display[64 * 32]{};

	bool keypad[16]{};
	bool isKeyPressed{};
	uint8_t keyPressed{};
	bool isKeyReleased{};
	const uint8_t font[80]{
							0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
							0x20, 0x60, 0x20, 0x20, 0x70, // 1
							0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
							0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
							0x90, 0x90, 0xF0, 0x10, 0x10, // 4
							0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
							0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
							0xF0, 0x10, 0x20, 0x40, 0x40, // 7
							0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
							0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
							0xF0, 0x90, 0xF0, 0x90, 0x90, // A
							0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
							0xF0, 0x80, 0x80, 0x80, 0xF0, // C
							0xE0, 0x90, 0x90, 0x90, 0xE0, // D
							0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
							0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
	
	void LoadFont();
	bool OpenROM(const std::string pathROM);
	
	uint16_t getXfromOPC(uint16_t OPC);
	uint16_t getYfromOPC(uint16_t OPC);
	//TODO: implement getNNNfromOPC

	void fetch();

	void OPC0x00E0();
	void OPC0x00EE();
	void OPC0x1NNN(uint16_t OPC);
	void OPC0x2NNN(uint16_t OPC);
	void OPC0x3XNN(uint16_t OPC, uint16_t XIndex);
	void OPC0x4XNN(uint16_t OPC, uint16_t XIndex);
	void OPC0x5XY0(uint16_t OPC, uint16_t XIndex, uint16_t YIndex);
	void OPC0x6XNN(uint16_t OPC, uint16_t XIndex);
	void OPC0x7XNN(uint16_t OPC, uint16_t XIndex);
	void OPC0x8XY0(uint16_t OPC, uint16_t XIndex, uint16_t YIndex);
	void OPC0x8XY1(uint16_t OPC, uint16_t XIndex, uint16_t YIndex, ShiftMode shiftmode);
	void OPC0x8XY2(uint16_t OPC, uint16_t XIndex, uint16_t YIndex, ShiftMode shiftmode);
	void OPC0x8XY3(uint16_t OPC, uint16_t XIndex, uint16_t YIndex, ShiftMode shiftmode);
	void OPC0x8XY4(uint16_t OPC, uint16_t XIndex, uint16_t YIndex);
	void OPC0x8XY5(uint16_t OPC, uint16_t XIndex, uint16_t YIndex);
	void OPC0x8XY7(uint16_t OPC, uint16_t XIndex, uint16_t YIndex);
	void OPC0x8XY6(uint16_t OPC, uint16_t XIndex, uint16_t YIndex, ShiftMode shiftmode);
	void OPC0x8XYE(uint16_t OPC, uint16_t XIndex, uint16_t YIndex, ShiftMode shiftmode);
	void OPC0x9XY0(uint16_t OPC, uint16_t XIndex, uint16_t YIndex);
	void OPC0xANNN(uint16_t OPC);
	void OPC0xBNNN(uint16_t OPC, uint16_t XIndex, ShiftMode shiftmode);
	void OPC0xCXNN(uint16_t OPC, uint16_t XIndex);
	void OPC0xDXYN(uint16_t OPC, uint16_t XIndex, uint16_t YIndex);
};

#endif
