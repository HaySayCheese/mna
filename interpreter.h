#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <list>

#include <assert.h>


class FileLinesInputStream {
public:
    FileLinesInputStream(std::string &filename);

    bool nextLine(std::string &str);
    inline std::size_t currentLineNumber() const;
    inline bool isOk() const;

private:
    std::ifstream mInputStream;
    std::size_t   mCurrentLineNumber;
};


typedef char AlphabetSymbol;
class Alphabet {
public:
    bool addSymbol(AlphabetSymbol symbol);
    bool isSymbolPresent(AlphabetSymbol symbol) const;
    std::size_t symbolsCount() const;

private:
    std::list<AlphabetSymbol> mAlphabet;
};


class Instruction
{
public:
    Instruction();

    void setReplaceble(std::string &replaceble);
    void setReplacer(std::string &replacer);
    void setFinal(bool isFinal = true);

    std::string& replaceble();
    std::string& replacer();
    bool isFinal() const;
    bool isOk() const;

private:
    bool mIsFinal;
    std::string mReplacer, mReplaceble;
};


//-- interpreter
class Interpreter
{
public:
   bool processFile(std::string &fileName);

private:
   bool loadAlphabet(FileLinesInputStream &file);
   bool loadSourceWord(FileLinesInputStream &file);
   bool loadInstructions(FileLinesInputStream &file);

   bool executeInstructions();
   bool executeInstruction(Instruction &instr);
   inline void checkSystemFirstSymbol();
   inline void checkSystemLastSymbol();

   void printAllInstructions() const;

private:
    std::string mFileName;
    std::string mSourceWord;
    Alphabet mAlphabet;
    std::vector<Instruction> mInstructions;
};


#endif // INTERPRETER_H
