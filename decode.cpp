#include <fstream>
#include <iostream>
using namespace std;

int main()
{
    fstream file;
    file.open("compressed.huff");
    
    int places [32];
    for (int i = 0, x = 1; i < 32; i++, x *= 2)
        places[i] = x;
    
    int codes [256][2];
    int currBinary (file.get()), currLength (4), dictionarySize (1);
    int codeBitlength = currBinary / 16;
    currBinary %= 16;
    for (int i = 0; i < 256; i++)
    {
        if (currLength < codeBitlength)
        {
            currBinary = 256*currBinary + file.get();
            dictionarySize++;
            currLength += 8;
        }
        codes[i][1] = currBinary / places[currLength - codeBitlength];
        currBinary %= places[currLength - codeBitlength];
        currLength -= codeBitlength;
    }
    for (int i = 0; i < 256; i++)
    {
        if (codes[i][1] != 0)
        {
            while (currLength < codes[i][1])
            {
                currBinary = 256*currBinary + file.get();
                dictionarySize++;
                currLength += 8;
            }
            codes[i][0] = currBinary / places[currLength - codes[i][1]];
            currBinary %= places[currLength - codes[i][1]];
            currLength -= codes[i][1];
        }
    }
    cout << "Dictionary information occupies " << dictionarySize << " bytes.";
    
    int numCodes = 0;
    for (int i = 0; i < 256; i++)
        if (codes[i][1] != 0)
            numCodes++;
    
    int actCodes [numCodes][2];
    for (int i1 = 0, i2 = 0; i1 < 256; i1++)
    {
        if (codes[i1][1] != 0)
        {
            actCodes[i2][0] = i1;
            actCodes[i2++][1] = codes[i1][0] * places[31 - codes[i1][1]];
        }
    }
    
    int gap = (int) (numCodes / 1.3);
    while (gap >= 1)
    {
        bool swapped (false);
        for (int i = 0; i < numCodes - gap; i++)
        {
            if (actCodes[i][1] > actCodes[i + gap][1])
            {
                swapped = true;
                int x0 (actCodes[i][0]), x1 (actCodes[i][1]);
                actCodes[i][0] = actCodes[i + gap][0];
                actCodes[i][1] = actCodes[i + gap][1];
                actCodes[i + gap][0] = x0;
                actCodes[i + gap][1] = x1;
            }
        }
        if (!swapped)
            gap = (int) (gap / 1.3);
    }
    
    int tree [2*numCodes - 1][3];
    int treeIndex (numCodes);
    for (int i = 0; i < numCodes; i++)
        tree[i][0] = actCodes[i][0];
    int stack [256][2];
    int stackIndex (0), codeIndex (0);
    stack[0][1] = -1;
    while (stack[0][1] != 0)
    {
        stack[stackIndex][0] = codeIndex;
        stack[stackIndex][1] = codes[actCodes[codeIndex++][0]][1];
        while (stackIndex > 0 && stack[stackIndex][1] ==
                stack[stackIndex-1][1])
        {
            tree[treeIndex][0] = -1;
            tree[treeIndex][1] = stack[stackIndex-1][0];
            tree[treeIndex][2] = stack[stackIndex--][0];
            stack[stackIndex][0] = treeIndex++;
            stack[stackIndex][1]--;
        }
        stackIndex++;
    }
    
    ofstream decoded;
    decoded.open("output.txt");
    int root = 2*numCodes - 2;
    int treeNode = root;
    if (currLength == 0)
    {
            currBinary = file.get();
            currLength = 8;
    }
    while (!(file.eof() && currLength == 0))
    {
        currLength--;
        int nextBit = currBinary / places[currLength];
        currBinary %= places[currLength];
        treeNode = tree[treeNode][(nextBit == 1) + 1];
        if (tree[treeNode][0] != -1)
        {
            decoded << (char) tree[treeNode][0];
            treeNode = root;
        }
        if (currLength == 0 && !file.eof())
        {
            currBinary = file.get();
            currLength = 8;
        }
    }
    
    return 0;
}