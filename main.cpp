#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <time.h>
#include <SDL.h>

// Screen Parameters
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320

// I/O Status Indentifiers
#define DISP_DO_NOT_UPDATE 0
#define DISP_NEEDS_UPDATE 1
#define WAITING_FOR_KEY 2
#define HALTED 3
#define NO_KEY 16


// SOUND Status Identifiers
#define YES_SOUND 1
#define NO_SOUND 0

char parse_key(char key_in) {
	char key;
	switch (key_in) {
	case SDLK_4:
		key = 0x7;
		break;
	case SDLK_5:
		key = 0x8;
		break;
	case SDLK_6:
		key = 0x9;
		break;
	case SDLK_7:
		key = 0xF;
		break;
	case SDLK_r:
		key = 0x4;
		break;
	case SDLK_t:
		key = 0x5;
		break;
	case SDLK_y:
		key = 0x6;
		break;
	case SDLK_u:
		key = 0xE;
		break;
	case SDLK_f:
		key = 0x1;
		break;
	case SDLK_g:
		key = 0x2;
		break;
	case SDLK_h:
		key = 0x3;
		break;
	case SDLK_j:
		key = 0xD;
		break;
	case SDLK_v:
		key = 0xA;
		break;
	case SDLK_b:
		key = 0x0;
		break;
	case SDLK_n:
		key = 0xB;
		break;
	case SDLK_m:
		key = 0xC;
		break;
	default:
		key = NO_KEY;
		break;
	}
	return key;
}

class chip8 {
	unsigned char mem[4096] = {			//Main Memory
										//With Preloaded Font Data
		0xF0, 0x90, 0x90, 0x90, 0xF0,	// 0
		0x20, 0x60, 0x20, 0x20, 0x70,	// 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0,	// 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0,	// 3
		0x90, 0x90, 0xF0, 0x10, 0x10,	// 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0,	// 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0,	// 6
		0xF0, 0x10, 0x20, 0x40, 0x40,	// 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0,	// 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0,	// 9
		0xF0, 0x90, 0xF0, 0x90, 0x90,	// A
		0xE0, 0x90, 0xE0, 0x90, 0xE0,	// B
		0xF0, 0x80, 0x80, 0x80, 0xF0,	// C
		0xE0, 0x90, 0x90, 0x90, 0xE0,	// D
		0xF0, 0x80, 0xF0, 0x80, 0xF0,	// E
		0xF0, 0x80, 0xF0, 0x80, 0x80	// F
	};
	unsigned char V[16] = { 0 };		//Registers
	unsigned char ST = 0, DT = 0;		//Sound Timer, Delay Timer
	unsigned short PC = 0x0200, I;		//Program Counter, I-register
	unsigned short stack[16] = { 0 };	//Stack
	short SP = -1;						//Stack Pointer

public:
	unsigned char disp[64][32] = {0};
	bool opened;
	chip8(char[]);
	int timer_tick();
	int exec_ins(unsigned char);
};


chip8::chip8(char fname[]) {
	disp[64][32] = { 0 };
	ST = 0, DT = 0;		
	PC = 0x0200;	
	SP = -1;	
	FILE * rom = fopen(fname, "rb");
	unsigned char tempc;
	if (!rom) {
		printf("Failed to open rom!\n");
		opened = 0;
	}
	if (rom) {
		fseek(rom, 0, SEEK_END);
		unsigned short index = 0x200;
		int pos = ftell(rom);
		rewind(rom);
		for (int i = 0; i < pos; i++) mem[index++] = fgetc(rom);
	}
}


int chip8::timer_tick() {
	int status = NO_SOUND;
	if (ST) {
		status = YES_SOUND;
		--ST;
	}
	if (DT) {
		--DT;
	}
	return status;
}

int chip8::exec_ins(unsigned char key) {
	int status = DISP_DO_NOT_UPDATE;

	unsigned short curr_ins = (short)mem[PC + 1] | (((short)mem[PC]) << 8);
	unsigned short nnn, n, x, y, kk;
	nnn = curr_ins & 0xFFF;
	n = curr_ins & 0x000F;
	x = (curr_ins & 0x0F00) >> 8;
	y = (curr_ins & 0x00F0) >> 4;
	kk = curr_ins & 0x00FF;

	unsigned char rnd;
	int vx, vy;

	switch (curr_ins) {
	case 0x00E0:
		//00E0 - clear screen
		for (int x = 0; x < 64; x++) {
			for (int y = 0; y < 32; y++) {
				disp[x][y] = 0;
			}
		}
		status = DISP_NEEDS_UPDATE;
		break;
	case 0x00EE:
		//00EE - pop return address from stack and return
		PC = stack[SP--];
		return status;
	}

	switch (curr_ins & 0xF000) {
	case 0x1000:
		//1nnn - jump to nnn
		PC = nnn;
		return status;
	case 0x2000:
		//2nnn - call nnn
		stack[++SP] = PC;
		PC = nnn;
		return status;
	case 0x3000:
		//3xkk - if Vx == kk skip next inst
		if (V[x] == kk) PC += 2;
		break;
	case 0x4000:
		//4xkk - if Vx != kk skip next inst
		if (V[x] != kk) PC += 2;
		break;
	case 0x5000:
		//5xy0 - if Vx==Vy skip next instr
		if (V[x] == V[y]) PC += 2;
		break;
	case 0x6000:
		//6xkk - Vx = kk
		V[x] = kk;
		break;
	case 0x7000:
		//7xkk - Vx += kk
		V[x] += kk;
		break;
	case 0x8000:
		switch (curr_ins & 0x000F) {
		case 0x0000:
			//8xy0 - Vx = Vy
			V[x] = V[y];
			break;
		case 0x0001:
			//8xy1 - Vx |= Vy
			V[x] |= V[y];
			break;
		case 0x0002:
			//8xy2 - Vx &= Vy
			V[x] &= V[y];
			break;
		case 0x0003:
			//8xy3 - Vx ^= Vy
			V[x] ^= V[y];
			break;
		case 0x0004:
			//8xy4 - Vx += Vy (VF = carry)
			vx = V[x], vy = V[y];
			V[x] += V[y];
			V[0xF] = !((vx + vy) > 0xFF);
			break;
		case 0x0005:
			//8xy5 - Vx -= Vy (VF = NOT borrow)
			vx = V[x], vy = V[y];
			V[x] -= V[y];
			V[0xF] = ((vx - vy) < 0);
			break;
		case 0x0006:
			//8xy6 - Vx = Vx >> 1 (VF = LSB Vx)
			V[0xF] = V[x] & 0x1;
			V[x] = V[x] >> 1;
			break;
		case 0x0007:
			//8xy7 - Vx = Vy - Vx (VF = NOT borrow)
			vx = V[x], vy = V[y];
			V[x] = V[y] - V[x];
			V[0xF] = ((vy - vx) < 0);
			break;
		case 0x000E:
			//8xyE - Vx = Vx << 1 (VF = MSB Vx)
			V[0xF] = V[x] & 0x80;
			V[x] = V[x] << 1;
			break;
		}
		break;
	case 0x9000:
		//9xy0 - if Vx != Vy skip nex instr
		if (V[x] != V[y]) PC += 2;
		break;
	case 0xA000:
		//Annn - set I-register to nnn
		I = nnn;
		break;
	case 0xB000:
		//Bnnn - PC = nnn + V0
		PC = nnn + V[0x0];
		return status;
	case 0xC000:
		//Cxkk - Vx = (random byte) & kk
		rnd = clock();
		V[x] = rnd & kk;
		break;
	case 0xD000:
		//Dxyn - draw sprite starting at location I,
		//n rows high at (Vx, Vy) and VF = collision
		for (int dy = 0; dy < n; dy++) {
			for (int dx = 0; dx < 8; dx++) {
				if (mem[I + dy] & (0x80 >> dx)) {
					disp[(V[x] + dx)%64][(V[y] + dy)%32] = 1 - disp[(V[x] + dx) % 64][(V[y] + dy) % 32];
					if (disp[V[x] + dx][V[y] + dy] != 1) V[0xF] = 1;
				}
			}
		}
		status = DISP_NEEDS_UPDATE;
		break;
	case 0xE000:
		switch (curr_ins & 0x00FF) {
		case 0x009E:
			//Ex9E - if key pressed is Vx then skip next instruction
			if (key == V[x]) PC += 2;
			break;
		case 0x00A1:
			//Ex9E - if key not pressed is Vx then skip next instruction
			if (key != V[x]) PC += 2;
			break;
		}
		break;
	case 0xF000:
		switch (curr_ins & 0x00FF) {
		case 0x07:
			//Fx07 - Vx = DT
			V[x] = DT;
			break;
		case 0x0A:
			//Fx0A - wait for key press, store key in Vx
			if (key == NO_KEY) return WAITING_FOR_KEY;
			V[x] = key;
			break;
		case 0x15:
			//Fx15 - DT = Vx
			DT = V[x];
			break;
		case 0x18:
			//Fx18 - ST = Vx
			ST = V[x];
			break;
		case 0x1E:
			//Fx1E - I-register += Vx
			vx = V[x];
			vy = I;
			if (vx + vy > 0xFFF) V[0xF] = 1;
			else V[0xF] = 0;
			I += V[x];
			break;
		case 0x29:
			//Fx29 - I = font location of digit at Vx
			I = V[x] * 5;
			break;
		case 0x33:
			//Fx33 - Store BCD representation of Vx 
			//in I (hundreds), I+1 (tens), I+2 (units)
			mem[I] = V[x] / 100;
			mem[I + 1] = (V[x] / 10) - 10 * (V[x] / 100);
			mem[I + 2] = V[x] - 10 * (V[x] / 10);
			break;
		case 0x55:
			//Fx55 - Store V0---Vx in memory starting at I
			for (unsigned char i = 0; i <= x; i++) {
				mem[I + i] = V[x];
			}
			break;
		case 0x65:
			//Fx07 - Retrieve V0---Vx from memory starting at I
			for (unsigned char i = 0; i <= x; i++) {
				V[x] = mem[I + i];
			}
			break;
		}
		break;
	}
	PC += 2;
	return status;
}

int main(int argc, char* argv[]) {
	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	chip8 eunit("CONNECT4");
	clock_t prev_clock=0,curr_clock;

	if (eunit.opened) {
		int status = DISP_DO_NOT_UPDATE, key = NO_KEY;

		while (status != HALTED) {
			curr_clock = clock();
			if ((curr_clock - prev_clock) > CLOCKS_PER_SEC / 60) {
				eunit.timer_tick();
				prev_clock = curr_clock;
			}
			status = eunit.exec_ins(key);
			key = NO_KEY;
			switch (status){
			case DISP_NEEDS_UPDATE:
				SDL_SetRenderDrawColor(renderer, 0,0,0,0);
				SDL_RenderClear(renderer);
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				for (int x = 0; x < WINDOW_WIDTH; x++) {
					for (int y = 0; y < WINDOW_HEIGHT; y++) {
						if (eunit.disp[x *64/WINDOW_WIDTH][y *32/WINDOW_HEIGHT]) {
							SDL_RenderDrawPoint(renderer, x, y);
						}
					}
				}
				SDL_RenderPresent(renderer);
				break;

			case WAITING_FOR_KEY:
				SDL_WaitEvent(&event);
				if (event.type == SDL_QUIT) status = HALTED;
				else if (event.type == SDL_KEYDOWN) key = parse_key(event.key.keysym.sym);
				break;
			}
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
					status = HALTED;
					break;
				case SDL_KEYDOWN:
					key = parse_key(event.key.keysym.sym);
					break;
				case SDL_KEYUP:
					key = NO_KEY;
					break;
				}
			}
		}
	}
	else {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "FILE NOT LOADED", "Could not load file. Please check if it exists.", window);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
