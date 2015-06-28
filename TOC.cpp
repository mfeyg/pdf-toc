#include <string>
#include <vector>
#include <iostream>
using namespace std;

struct Entry {
  string name;
  int page;
  vector<Entry> children;
  Entry(string name, int page, vector<Entry> children)
    : name(name), page(page), children(children) {}
};

struct Line {
  int depth;
  string name;
  int page;
  Line(int depth, string name, int page)
    : depth(depth), name(name), page(page) {}
};

Line parseLine(const string& line) {
  int tabs = 0;
  while (line[tabs] == '\t') ++tabs;
  int last_tab = line.rfind('\t');
  int page_no = stoi(line.substr(last_tab + 1));
  string name(line, tabs, last_tab - tabs);
  return Line(tabs, string(name, 0, name.find_last_not_of('\t')+1), page_no);
}

int readLines(istream& in, vector<Line>& out) {
  string line;
  int count = 0;
  while (getline(in, line)) {
    out.push_back(parseLine(line));
    ++count;
  }
  return count;
}

vector<Line>::iterator readSection(vector<Line>::iterator in,
                                   vector<Line>::iterator end,
                                   vector<Entry>& out) {
  int depth = in->depth;
  while (in != end && in->depth == depth) {
    string name = in->name;
    int page = in->page;
    ++in;
    vector<Entry> children;
    if (in->depth > depth)
      in = readSection(in, end, children);
    out.push_back(Entry(name, page, children));
  }
  return in;
}

void printTOC(ostream& out, const Entry& toc) {
  out << "Name: '" << toc.name << "' Page: " << toc.page << " Children: [";
  for (auto it = toc.children.begin(); it != toc.children.end(); ++it) {
    printTOC(out, *it);
  }
  out << "], ";
}

int main() {
  vector<Line> lines;
  readLines(cin, lines);
  vector<Entry> toc;
  readSection(lines.begin(), lines.end(), toc);
  cout << toc.size() << endl;
  for (auto it = toc.begin(); it != toc.end(); ++it) {
    printTOC(cout, *it);
    cout << endl;
  }
}
