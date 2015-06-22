# pdf-toc
The goal of this project is to add an "Outline" or Table of Contents to a PDF
file from a representation in a text file.

The text file format uses indentation with tabs to give structure to the Table
of Contents, with a page number at the end of each line separated from the rest
of the line by at least one tab. An example follows.
```
Title             1
  Chapter 1       3
    Section 1.1   3
    Section 1.2   5
  Chapter 2       11
    Section 2.1   11
    Section 2.2   11
    Section 2.3   17
  Chapter 3       21
    Section 3.1   21
    Section 3.2   23
    Section 3.3   25
  Appendix        27
  Index           33
    A-M           33
    N-Z           37
```
