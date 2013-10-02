#include <fstream>
using namespace std;

void setCodes(int codes[][2], int tree[][3], int root, int s, int length)
{
    if (tree[root][0] != -1)
    {
        codes[tree[root][0]][0] = s;
        codes[tree[root][0]][1] = length;
    }
    else
    {
        setCodes(codes, tree, tree[root][1], 2*s, length + 1);
        setCodes(codes, tree, tree[root][2], 2*s + 1, length + 1);
    }
}

int main()
{
    ifstream file;
    file.open("input.txt");
    
    int freq [256];
    for (int i = 0; i < 256; i++)
        freq[i] = i;
    while (!file.eof())
        freq[file.get()] += 1000;
    file.close();
    
    int gap = 224;
    while (gap >= 1)
    {
        bool swapped (false);
        for (int i = 0; i < 256 - gap; i++)
        {
            if (freq[i] > freq[i + gap])
            {
                swapped = true;
                int x (freq[i]);
                freq[i] = freq[i + gap];
                freq[i + gap] = x;
            }
        }
        if (!swapped)
            gap = (int) (gap / 1.3);
    }
    
    int firstCode = 0;
    for (; freq[firstCode] < 1000; firstCode++);
    int tree [511 - 2*firstCode][3];
    int treeIndex (256 - firstCode);
    int queue [treeIndex][2];
    int queueEnd (treeIndex - 1);
    for (int i = 0; i <= queueEnd; i++)
    {
        queue[i][0] = freq[firstCode+i] % 1000;
        queue[i][1] = freq[firstCode+i] / 1000;
        tree[i][0] = queue[i][0];
        queue[i][0] = i;
    }
    for (int i = 0; i < queueEnd; treeIndex++, i++)
    {
        tree[treeIndex][0] = -1;
        tree[treeIndex][1] = queue[i][0];
        tree[treeIndex][2] = queue[i+1][0];
        queue[i+1][0] = treeIndex;
        queue[i+1][1] = queue[i][1] + queue[i+1][1];
        
        for (int x = i+1; x < queueEnd && queue[x][1] > queue[x+1][1];
  		 x++) 
        {
			int x0 = queue[x][0];
			int x1 = queue[x][1];
			queue[x][0] = queue[x+1][0];
			queue[x][1] = queue[x+1][1];
			queue[x+1][0] = x0;
			queue[x+1][1] = x1;
        }
    }
    
    int codes [256][2];
    for (int i = 0; i < 256; i++)
        codes[i][1] = 0;
    setCodes(codes, tree, queue[queueEnd][0], 0, 0);
    
    int places [32];
    for (int i = 0, x = 1; i < 32; i++, x *= 2)
        places[i] = x;
    
    ofstream encoded;
    encoded.open("compressed.huff");
    int maxLength (0), codeBitlength (0);
    for (int i = 0; i < 256; i++)      
        if (codes[i][1] > maxLength)
            maxLength = codes[i][1];
    for (; maxLength > 0; codeBitlength++)
        maxLength /= 2;
    int currLength (4), currBinary (codeBitlength);
    for (int i = 0; i < 256; i++)
    {
        currBinary = (currBinary << codeBitlength) + codes[i][1];
        currLength += codeBitlength;
        if (currLength >= 8)
        {
            currLength -= 8;
            encoded << (char) (currBinary / places[currLength]);
            currBinary %= places[currLength];
        }
    }
    for (int i = 0; i < 256; i++)
    {
        if (codes[i][1] != 0)
        {
            currBinary = (currBinary << codes[i][1]) + codes[i][0];
            currLength += codes[i][1];
            while (currLength >= 8)
            {
                currLength -= 8;
                encoded << (char) (currBinary / places[currLength]);
                currBinary %= places[currLength];
            }
        }
    }
    
    file.open("input.txt");
    while (!file.eof())
    {
        int next = file.get();
        if (next != -1)
        {
            currBinary = (currBinary << codes[next][1]) + codes[next][0];
            currLength += codes[next][1];
            while (currLength >= 8)
            {
                currLength -= 8;
                encoded << (char) (currBinary / places[currLength]);
                currBinary %= places[currLength];
            }
        }
    }

    for (int i = 0; i < 256 && currLength != 0; i++)
    {
            if (codes[i][1] > 8 - currLength)
            {
                    currBinary = (currBinary << codes[i][1]) + codes[i][0];
                    currLength += codes[i][1];
                    encoded << (char) (currBinary / places[currLength - 8]);
                    break;
            }
    }
    file.close();
    encoded.close();

    return 0;
}