#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned char BYTE;
struct tagBITMAPFILEHEADER
{
    WORD bfType; //specifies the file type
    DWORD bfSize; //specifies the size in bytes of the bitmap file
    WORD bfReserved1; //reserved; must be 0
    WORD bfReserved2; //reserved; must be 0
    DWORD bfOffBits; //species the offset in bytes from the bitmapfileheader to the bitmap bits
};
struct tagBITMAPINFOHEADER
{
    DWORD biSize; //specifies the number of bytes required by the struct
    LONG biWidth; //specifies width in pixels
    LONG biHeight; //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage; //size of image in bytes
    LONG biXPelsPerMeter; //number of pixels per meter in x axis
    LONG biYPelsPerMeter; //number of pixels per meter in y axis
    DWORD biClrUsed; //number of colors used by th ebitmap
    DWORD biClrImportant; //number of colors that are important
};

typedef struct frequencyNode
{
    int key; // the value (0-255) that this node represents
    int frequency; // the frequency that the value occurs
    frequencyNode *left;
    frequencyNode *right;
} frequencyNode;

// typedef struct treeNode
// {
//     frequencyNode data;
//     treeNode *left;
//     treeNode *right;
// } treeNode;

struct tagBITMAPINFOHEADER *infoHeader;

void fileRead(struct tagBITMAPFILEHEADER *fileHead, struct tagBITMAPINFOHEADER *fileInfo, FILE *fp)
{
    fread(&(fileHead -> bfType), sizeof(WORD), 1, fp);
    fread(&(fileHead -> bfSize), sizeof(DWORD), 1, fp);
    fread(&(fileHead -> bfReserved1), sizeof(WORD), 1, fp);
    fread(&(fileHead -> bfReserved2), sizeof(WORD), 1, fp);
    fread(&(fileHead -> bfOffBits), sizeof(DWORD), 1, fp);
    fread(fileInfo, sizeof(struct tagBITMAPINFOHEADER), 1, fp);
}

void fileWrite(struct tagBITMAPFILEHEADER *fileHead, struct tagBITMAPINFOHEADER *fileInfo, BYTE array[], DWORD size, FILE *fp)
{
    fwrite(&(fileHead -> bfType), sizeof(WORD), 1, fp);
    fwrite(&(fileHead -> bfSize), sizeof(DWORD), 1, fp);
    fwrite(&(fileHead -> bfReserved1), sizeof(WORD), 1, fp);
    fwrite(&(fileHead -> bfReserved2), sizeof(WORD), 1, fp);
    fwrite(&(fileHead -> bfOffBits), sizeof(DWORD), 1, fp);
    fwrite(fileInfo, sizeof(struct tagBITMAPINFOHEADER), 1, fp);
    fwrite(array, size, 1, fp);
}

void findFrequencies(BYTE *arr, DWORD size, int *frequency)
{
    for(int y = 0; y < infoHeader->biHeight; ++y)
        for(int x = 0; x < infoHeader->biWidth * 3; ++x)
            frequency[arr[y*size + x]]++;
}

// fill and sort node array
void sortNodes(frequencyNode *nodeArr, int *frequencies)
{
    for (int i = 0; i < 256; i++)
    {
        nodeArr[i].key = i;
        nodeArr[i].frequency = frequencies[i];
    }

    // insertion sort
    int j;
    for(int i = 1; i < 256; ++i)
    {
        j = i;
        while(j > 0 && nodeArr[j].frequency < nodeArr[j-1].frequency)
        {
            frequencyNode temp = nodeArr[j-1];
            nodeArr[j-1] = nodeArr[j];
            nodeArr[j] = temp;
            -j;
        }
    }
}

void insertElement(frequencyNode *arr, frequencyNode insertNode)
{
    int i = 0;
    while(insertNode.frequency > arr[i].frequency)
        ++i;
    
    frequencyNode *temp = &arr[i];
    arr[i] = insertNode;
    while(i < 254)
    {
        arr[i] = *temp;
        *temp = arr[i + 1];
        ++i;
    }
    arr[i] = *temp;
}

void buildHuffman(frequencyNode *arr)
{
    while(arr[0].key != -1)
    {
        arr[0] = ;
    }
}

int main(int argc, char *argv[])
{
    struct tagBITMAPFILEHEADER *fileHeader = (struct tagBITMAPFILEHEADER*)malloc(sizeof(struct tagBITMAPFILEHEADER));
    infoHeader = (struct tagBITMAPINFOHEADER*)malloc(sizeof(struct tagBITMAPINFOHEADER));

    char *inputName = argv[1];
    char *outputName[strlen(inputName)];
    strcpy(outputName, inputName);

    // give output file a .slo extension
    outputName[strlen(inputName) - 3] = 's';
    outputName[strlen(inputName) - 2] = 'l';
    outputName[strlen(inputName) - 1] = 'o';

    FILE *inputFP = fopen(inputName, "rb");
    FILE *outputFP = fopen(outputName, "wb");
    
    fileRead(fileHeader, infoHeader, inputFP);

    LONG byteWidth = (infoHeader -> biWidth) * 3;
    int padding = (4 - ((byteWidth) % 4)) % 4;
    LONG trueWidth = byteWidth + padding;
    DWORD trueSize = trueWidth * (infoHeader -> biHeight);
    BYTE byteArr[trueSize];
    fread(byteArr, trueSize, 1, inputFP);
    int freqArr[256];
    frequencyNode nodeArr[256];

    // // set array keys to -1
    // for(int i = 0; i < 256; ++i)
    //     nodeArr[i].key = -1;


    findFrequencies(byteArr, trueSize, freqArr);
    sortNodes(nodeArr, freqArr);

    fileWrite(fileHeader, infoHeader, byteArr, trueSize, outputFP);

    fclose(inputFP);
    fclose(outputFP);
    
    return 0;
}