#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace std;

bool has_extension(char *filename, char *extension) {
  int offset = strlen(filename) - strlen(extension);
  return offset >= 0 && strcmp(filename + offset, extension) == 0;
}

void check_arguments(int argc, char *argv[]) {
  if (argc == 5
      && has_extension(argv[1], ".pdf")
      && has_extension(argv[2], ".txt")
      && strcmp(argv[3], "-o") == 0
      && has_extension(argv[4], ".pdf")) {
    return;
  }
  else {
      cout << "Usage: " << argv[0] << " in.pdf toc.txt -o out.pdf" << endl;
      exit(EXIT_FAILURE);
  }

}

int main(int argc, char *argv[]) {
    check_arguments(argc, argv);
}
