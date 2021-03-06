#include <iostream>
#include <iomanip>
using namespace std;

#define Nr 10

unsigned char sbox[] = {
  0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76,
  0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0,
  0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15,
  0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75,
  0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84,
  0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF,
  0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8,
  0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2,
  0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73,
  0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x08,0xDB,
  0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79,
  0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08,
  0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A,
  0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E,
  0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF,
  0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16
};
unsigned int w[44];

void print_state(string msg, unsigned int* state)
{
	cout << msg << ": " << endl;
	for (int i=0; i<4; i++)
	{
		cout << hex << setw(8) << setfill('0') << state[i]; 
	}
	cout << endl;

}

//Perform substitution on a single byte by looking it up on the SBox
unsigned char SubBytes(unsigned char c)
{
	return sbox[c];
}

//Loop through the bytes of w and replace each one with its substitution using SubBytes
unsigned int SubWord(unsigned int w)
{
	unsigned int val = 0;
	val = SubBytes(w&255);
	w >>=8;
	val |= SubBytes(w&255) << 8;
	w >>=8;
	val |= SubBytes(w&255) << 16;
	w >>=8;
	val |= SubBytes(w&255) << 24;

	return val;
}

//Perform a bitwise rotation on a word -- used in the key schedule
unsigned int RotWord(unsigned int val)
{
	return (val << 8) | (val >> 24);
}

//Expand a key from 16 bytes (4 words) to 176 bytes (44 words)
void KeyExpansion(unsigned int key[4])
{
	unsigned int RCon[11];
	RCon[1] = 0x01000000;
	RCon[2] = 0x02000000;
	RCon[3] = 0x04000000;
	RCon[4] = 0x08000000;
	RCon[5] = 0x10000000;
	RCon[6] = 0x20000000;
	RCon[7] = 0x40000000;
	RCon[8] = 0x80000000;
	RCon[9] = 0x1B000000;
	RCon[10] = 0x36000000;
	for(int i=0; i<4; i++)
	{
		w[i] = key[i];
	}
	for(int i=4; i<44; i++)
	{
		unsigned int temp = w[i-1];
		if ((i%4) == 0)
		    {
			temp = SubWord(RotWord(temp)) ^ RCon[i/4];
		    }
		
		w[i] = w[i-4] ^ temp;    
	}
}

//Add (Xor) the key for the next round into "state"
void AddRoundKey(unsigned int* state, int round)
{
  	//we apply the round key to a column, not a row. So transpose "state" and call it "t".
	unsigned int t[4] = {0,0,0,0};
	for (int i=0; i<4; i++)
	{
		t[0] <<= 8;
		t[0] |= (state[i]>>24)&255;
	}

	for (int i=0; i<4; i++)
	{
		t[1] <<= 8;
		t[1] |= (state[i]>>16)&255;
	}
	
	for (int i=0; i<4; i++)
	{
		t[2] <<= 8;
		t[2] |= (state[i]>>8)&255;
	}

	for (int i=0; i<4; i++)
	{
		t[3] <<= 8;
		t[3] |= (state[i])&255;
	}

	//Apply the round key to t(XOR step)
//*************************************************************	
	t[0] = t[0] ^ w[4*round];
	t[1] = t[1] ^ w[4*round + 1];
	t[2] = t[2] ^ w[4*round + 2];
	t[3] = t[3] ^ w[4*round + 3];
		

	//Now transpose "t" back into state.
	state[0] = state[1] = state[2] = state[3] = 0;
	for (int i=0; i<4; i++)
	{
		state[0] <<=8;
		state[0] |= (t[i]>>24)&255;	
	}

	for (int i=0; i<4; i++)
	{
		state[1] <<=8;
		state[1] |= (t[i]>>16)&255;	
	}

	for (int i=0; i<4; i++)
	{
		state[2] <<=8;
		state[2] |= (t[i]>>8)&255;	
	}

	for (int i=0; i<4; i++)
	{
		state[3] <<=8;
		state[3] |= (t[i])&255;	
	}
}

//perform the AES shift rows operation
void ShiftRows(unsigned int* state)
{
	state[1] = (state[1] << 8) | (state[1] >> 24); 
	state[2] = (state[2] << 16) | (state[2] >> 16);
	state[3] = (state[3] << 24) | (state[3] >> 8);
}

//AES field multiplication
unsigned char FieldMult(unsigned char a, unsigned char b)
{
	unsigned char p = 0; 
	unsigned char carry = 0;
	//Loop through digits of b.
	for (int i=0; i<8; i++)
	{
		if (b&1) //rightmost bit is one
		{
 			p^=a; //"add" a to our sum
		}

		//Check if shifting will produce x^8
		carry = ((a & 128) > 0);

		a <<= 1; //Shift a

		if(carry)
		{
			a^= 0x1b;
		}
		b >>=1; //Shift over so we can repeat
	}

	return p;
}

//Mix column c with column u
void MixColumns(unsigned char* c, unsigned char* u)
{	
	u[0] = FieldMult(2,c[0]) ^ FieldMult(3,c[1]) ^ c[2] ^ c[3];
	u[1] = FieldMult(2,c[1]) ^ FieldMult(3,c[2]) ^ c[3] ^ c[0];
	u[2] = FieldMult(2,c[2]) ^ FieldMult(3,c[3]) ^ c[0] ^ c[1];
	u[3] = FieldMult(2,c[3]) ^ FieldMult(3,c[0]) ^ c[1] ^ c[2];
}


//Mix all columns
void MixAllColumns(unsigned int* state)
{

	unsigned char column1[4];
	unsigned char column2[4];
	unsigned char column3[4];
	unsigned char column4[4];
	
	unsigned char ncolumn1[4];
	unsigned char ncolumn2[4];
	unsigned char ncolumn3[4];
	unsigned char ncolumn4[4];

	for (int i=0; i<4; i++)
	{
		column1[i] = (state[i]>>24) & 255;
		column2[i] = (state[i]>>16) & 255;
		column3[i] = (state[i]>>8) & 255;
		column4[i] = state[i] & 255;
	}
	MixColumns(column1, ncolumn1);
	MixColumns(column2, ncolumn2);
	MixColumns(column3, ncolumn3);
	MixColumns(column4, ncolumn4);

	for (int i=0; i<4; i++)
	{
		state[i] = (ncolumn1[i]<<24) | (ncolumn2[i]<<16) | (ncolumn3[i]<<8) | (ncolumn4[i]);
	}

}

//Perform the main AES algorithm using the key from problem 3.5 on the message from problem 3.6

int main()
{
	unsigned int state[4] = {0x328831e0,
	 			 0x435a3137,
				 0xf6309807,
				 0xa88da234};

	unsigned int key[] = {0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c};
	KeyExpansion(key);

// 	Prints the result for question 3.5.
/*	for (int i=0; i<44; i+=4)
	{
		cout << hex << w[i] << " " << w[i+1] << " " << w[i+2] << " " << w[i+3] << endl;
	}
*/

 
	print_state("Initial", state);

	AddRoundKey(state,0);
	print_state("AddRoundKey: ", state);
	for (int i=1; i<Nr; i++)
	{
		for (int j=0; j<4; j++)
		{
			state[j] = SubWord(state[j]);
		}
		print_state("subword: ",state);
		ShiftRows(state);
		print_state("shift rows: ", state);
		MixAllColumns(state);
		print_state("MixAllColumns: ", state);
		AddRoundKey(state,i);
		print_state("AddRoundKey2: ", state);
	}
	for (int j=0; j<4; j++)
	{
		state[j] = SubWord(state[j]);
	}
	print_state("subword: ",state);
	ShiftRows(state);
	print_state("shiftRows2: ", state);
	AddRoundKey(state,Nr);
	print_state("AddRoundKey3: ", state);

	for (int i=0; i<4; i++)
	{
		cout << hex << "0x" << setw(8) << setfill('0') << state[i] << " ";
	}

	cout << endl;
}









