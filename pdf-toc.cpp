#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <utility>

using namespace std;

bool has_extension(const char *filename, const char *extension)
{
    int offset = strlen(filename) - strlen(extension);
    return offset >= 0 && strcmp(filename + offset, extension) == 0;
}

void check_arguments(int argc, char *argv[])
{
    if (argc == 5
        && has_extension(argv[1], ".pdf")
        && has_extension(argv[2], ".txt")
        && strcmp(argv[3], "-o") == 0
        && has_extension(argv[4], ".pdf"))
    {
        return;
    }
    else
    {
        cerr << "Usage: " << argv[0] << " in.pdf toc.txt -o out.pdf" << endl;
        exit(EXIT_FAILURE);
    }
}

int find_startxref(ifstream *in_pdf)
{
    char str[31];
    in_pdf->seekg(-30, in_pdf->end);
    in_pdf->read(str, 30);
    str[in_pdf->gcount()] = '\0';
    char *ptr = strstr(str, "startxref");
    if (!ptr)
    {
        cerr << "Failed to find startxref" << endl;
        exit(EXIT_FAILURE);
    }
    int byte_offset;
    int found = sscanf(ptr, "startxref %d", &byte_offset);
    if (found != 1)
    {
        cerr << "Failed to read startxref offset" << endl;
        exit(EXIT_FAILURE);
    }
    return byte_offset;
}

string add_xref(map<int, int> *xref_map, ifstream *in_pdf, int byte_offset)
{
    in_pdf->seekg(byte_offset);
    string xref;
    *in_pdf >> xref;
    if (xref != "xref")
    {
        cerr << "xref is not where it is supposed to be" << endl;
        exit(EXIT_FAILURE);
    }

    string token;
    while (*in_pdf >> token, token != "trailer")
    {
        int first_object;
        istringstream(token) >> first_object;
        int count;
        *in_pdf >> count;
        for (int i = 0; i < count; i++)
        {
            int offset;
            int generation;
            char in_use;
            *in_pdf >> offset >> generation >> in_use;
            if (in_use == 'n')
            {
                // std::map.insert() only inserts if key is not yet present
                xref_map->insert(pair<int, int>(first_object + i, offset));
            }
        }
    }
    int start_of_trailer = (int) in_pdf->tellg() - 7;
    int prev = -1;
    while (*in_pdf >> token, token != "%%EOF")
    {
        if (has_extension(token.c_str(), "/Prev"))
        {
            *in_pdf >> prev;
        }
    }
    int length_of_trailer = (int) in_pdf->tellg() + 2 - start_of_trailer;
    cout << start_of_trailer << '\t' << length_of_trailer << endl;
    if (prev != -1)
    {
        add_xref(xref_map, in_pdf, prev);
    }
    char trailer[length_of_trailer];
    in_pdf->seekg(start_of_trailer);
    in_pdf->read(trailer, length_of_trailer);
    trailer[length_of_trailer] = '\0';
    return string(trailer);
}

int main(int argc, char *argv[])
{
    check_arguments(argc, argv);
    ifstream in_pdf(argv[1]);
    ifstream toc_txt(argv[2]);
    ofstream out_pdf(argv[4]);

    int byte_offset = find_startxref(&in_pdf);
    map<int, int> xref_map;
    string trailer = add_xref(&xref_map, &in_pdf, byte_offset);

    cout << "xref_map has " << xref_map.size() << " elements." << endl;
    //in_pdf.seekg(0);
    //out_pdf << in_pdf.rdbuf();
    //out_pdf << toc_txt.rdbuf();
}
