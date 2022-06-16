#include <iostream>
#include <string>
#include <fstream>
#include <math.h>

/*
simple and compact
1) Program Description
    The program simulates a memory system that uses a write back style cache. 


2) I/O design
        The cache uses two memory blocks that hold 8 items each. I have a text file
        that is read line by line and runs the decode and searchCache function on that specific line.
        The output is set up with the following format:
        Instruction     Hit/Miss
        Registers
        Cache Block 0 Information
        Cache Block 1 Information
        Memory Address Data

3)Compile and Results
        I believe my results are correct. Based on the compiled results
        and stepping through the code I did not see any logic errors that would
        produce any adverse results.
*/

using namespace std;

//Block used in the cache
struct block
{
    int valid;
    int tag;
    int data;
    int history;
};

//
int* decode(string);

block* searchCache(block b[][2], int*, int[8], int[128]);
void bitPrint32(int v);
void bitPrint4(int v);

int main()
{
    ifstream myFile;
    myFile.open("Input-object-code.txt");
    string line;
    int* decodedValue;
    int op = 0;
    int rs = 0;
    int rt = 0;
    int offset = 0;
    int cacheSetNum = 0;
    int cacheTag = 0;
    int regNum = 0;
    int iCount = 0;
    int jCount = 0;



    block myBlock[8][2];
    int reg_file[8];
    int mem[128];

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            myBlock[i][j].valid = 0;
            myBlock[i][j].tag = 0;
            myBlock[i][j].data = 0;
            myBlock[i][j].history = 0;
        }
    }

    for (int i = 0; i < 8; i++)
        reg_file[i] = 0;

    for (int i = 0; i < 128; i++)
        mem[i] = 5 + i;

    if (myFile.is_open())
    {
        while (getline(myFile, line, '\r'))
        {
            cout << line << "\t";
            decodedValue = decode(line);
            searchCache(myBlock, decodedValue, reg_file, mem);
        }
        myFile.close();
    }

    cout << "Registers: " << endl;
    for (int i = 0; i < 8; i++)
    {
        cout << "Register " << i << ": \t";
        bitPrint32(reg_file[i]);
        cout << endl;
    }

    cout << "Cache Block 0" << endl;
    cout << "Set# \t" << "Valid \t" << "Tag \t" << "Data \t" << endl;
    for (int i = 0; i < 8; i++)
    {
           cout << i << "\t";
           cout << myBlock[i][0].valid << "\t";
           bitPrint4(myBlock[i][0].tag);
           cout << "\t";
           bitPrint32(myBlock[i][0].data);
           cout << endl;
    }

    cout << "Cache Block 1" << endl;
    cout << "Set# \t" << "Valid \t" << "Tag \t" << "Data \t" << endl;
    for (int i = 0; i < 8; i++)
    {
        cout << i << "\t";
        cout << myBlock[i][1].valid << "\t";
        bitPrint4(myBlock[i][1].tag);
        cout << "\t";
        bitPrint32(myBlock[i][1].data);
        cout << endl;
    }


    cout << "Address \t" << "Data \t" << endl;
    for (int i = 0; i < 128; i++)
    {
        cout << i << "\t";
        bitPrint32(mem[i]);
        cout << endl;
    }

    system("pause");
    return 0;
}



int* decode(string line)
{
    int op = 0;
    int rs = 0;
    int rt = 0;
    int indexCounter = 0;
    int byteAddress;
    int wordAddress;
    int offset = 0;
    int cacheSetNum = 0;
    int cacheTag = 0;
    int regNum = 0;
    //100011 = 35
    //101011 = 43
    //Check for OpCode
    if (line[0] == '1' && line[1] == '0' && line[2] == '0' && line[3] == '0' && line[5] == '1' && line[5] == '1')
        op = 35;
    else
        op = 43;

    //Converts the rt portion of the line (index 11-15) to be an int value
    for (int i = 15; i >= 11; i--)
    {
        if (line[i] == '1')
            rt += pow(2, indexCounter);
        indexCounter++;
    }
    indexCounter = 0;
    //Converts the offset portion of the line (index 16-31) to be an int value
    for (int i = line.length() - 1; i >= 16; i--)
    {
        if (line[i] == '1')
            offset += pow(2, indexCounter);
        indexCounter++;
    }
    //Makes the offset a word address instead of byte.
    byteAddress = rs + offset;
    wordAddress = byteAddress / 4;

    //Computes cacheSetNum and cacheTag
    cacheSetNum = wordAddress % 8;
    cacheTag = wordAddress / 8;

    //Computes correct register
    regNum = rt - 16;

    //Decoded Value as follows
    int* val = new int[7]{ rs, rt, offset, cacheSetNum, cacheTag, regNum, op};
    //decodedValue = val;
    return val;
}

block* searchCache(block myBlock[][2], int* decodedValue, int reg_file[8], int mem[128])
{
    int rs;
    int rt;
    int offset;
    int cacheSetNum;
    int cacheTag;
    int regNum;
    int op;
    int memCopy;
    rs = *(decodedValue + 0);
    rt = *(decodedValue + 1);
    offset = *(decodedValue + 2);
    cacheSetNum = *(decodedValue + 3);
    cacheTag = *(decodedValue + 4);
    regNum = *(decodedValue + 5);
    op = *(decodedValue + 6);

    int byteAddress = rs + offset;
    int wordAddress = byteAddress / 4;
    int memAddress = (cacheTag * 8) + cacheSetNum;

    if (myBlock[cacheSetNum][0].valid == 1 && myBlock[cacheSetNum][0].tag == cacheTag)
    {
        if (op == 35)
        {
            reg_file[regNum] = myBlock[cacheSetNum][0].data;
            myBlock[cacheSetNum][0].history = 1;
            myBlock[cacheSetNum][1].history = 0;
            cout << "Read hit" << endl;
        }
        else if (op == 43)
        {
            myBlock[cacheSetNum][0].data = reg_file[regNum];
            myBlock[cacheSetNum][0].history = 1;
            myBlock[cacheSetNum][1].history = 0;
            cout << "Write hit" << endl;
        }
    }
    else if (myBlock[cacheSetNum][1].valid == 1 && myBlock[cacheSetNum][1].tag == cacheTag)
    {
        if (op == 35)
        {
            reg_file[regNum] = myBlock[cacheSetNum][1].data;
            myBlock[cacheSetNum][0].history = 0;
            myBlock[cacheSetNum][1].history = 1;
            cout << "Read hit" << endl;
        }
        else if (op == 43)
        {
            myBlock[cacheSetNum][1].data = reg_file[regNum];
            myBlock[cacheSetNum][0].history = 0;
            myBlock[cacheSetNum][1].history = 1;
            cout << "Write hit" << endl;
        }
    }
    else
    { 
        if (op == 35)
        {
            cout << "Read miss" << endl;
            if (myBlock[cacheSetNum][0].history == 1)
            {
                //ALL THIS FOR BLOCK 1
                if (myBlock[cacheSetNum][1].valid == 1)
                    {
                    mem[memAddress] = myBlock[cacheSetNum][1].data;
                    }

                memCopy = mem[memAddress];
                myBlock[cacheSetNum][1].data = memCopy;
                myBlock[cacheSetNum][1].valid = 1;
                myBlock[cacheSetNum][1].tag = cacheTag;
                myBlock[cacheSetNum][0].history = 0;
                myBlock[cacheSetNum][1].history = 1;
                reg_file[regNum] = myBlock[cacheSetNum][1].data;
            }
            else if(myBlock[cacheSetNum][1].history == 1)
            {
                //ALL THIS FOR BLOCK 0
                if (myBlock[cacheSetNum][0].valid == 1)
                {
                    mem[memAddress] = myBlock[cacheSetNum][0].data;
                }

                memCopy = mem[memAddress];
                myBlock[cacheSetNum][0].data = memCopy;
                myBlock[cacheSetNum][0].valid = 1;
                myBlock[cacheSetNum][0].tag = cacheTag;
                myBlock[cacheSetNum][0].history = 1;
                myBlock[cacheSetNum][1].history = 0;
                reg_file[regNum] = myBlock[cacheSetNum][0].data;
            }
            else
            {
                //Should be same case as block 0
                if (myBlock[cacheSetNum][0].valid == 1)
                {
                    mem[memAddress] = myBlock[cacheSetNum][0].data;
                }
                memCopy = mem[memAddress];
                myBlock[cacheSetNum][0].data = memCopy;
                myBlock[cacheSetNum][0].valid = 1;
                myBlock[cacheSetNum][0].tag = cacheTag;
                myBlock[cacheSetNum][0].history = 1;
                myBlock[cacheSetNum][1].history = 0;
                reg_file[regNum] = myBlock[cacheSetNum][0].data;
            }
        }
        if (op == 43)
        {
            cout << "Write miss" << endl;
            mem[wordAddress] = reg_file[regNum];
        }
    }
    return *myBlock;
}

void bitPrint32(int v)
{
    int mask = 1;
    mask <<= 31;

    for (int i = 1; i <= 32; i++)
    {
        if ((v & mask) == 0)
            cout << "0";
        else
            cout << "1";
        v <<= 1;
    }
    cout << endl;
}

void bitPrint4(int v)
{
    int mask = 1;
    mask <<= 3;

    for (int i = 1; i <= 4; i++)
    {
        if ((v & mask) == 0)
            cout << "0";
        else
            cout << "1";
        v <<= 1;
    }
}