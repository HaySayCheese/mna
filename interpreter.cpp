#include "interpreter.h"


FileLinesInputStream::FileLinesInputStream(std::string &filename):
    mInputStream(filename.c_str()), mCurrentLineNumber(0) {
}


bool FileLinesInputStream::nextLine(std::string &str) {

    /* Writes next line from file to @str.
     * Returns true if no errors occured, otherwise - returns false. */


#ifndef NDEBUG
    assert(! mInputStream.fail());
#endif

    if (std::getline(mInputStream, str)) {
        ++mCurrentLineNumber;
        return true;
    }

    return false;
}


bool FileLinesInputStream::isOk() const {
    return !mInputStream.fail();
}


std::size_t FileLinesInputStream::currentLineNumber() const {
    return mCurrentLineNumber;
}


/* Alphabet */
bool Alphabet::addSymbol(AlphabetSymbol symbol) {

    /* Tries to add new symbol to the alphabet.
     * If the symbol is already exists - returns False,
     * otherwise - returns True.

     * TODO: Improve correct Unicode support. */

    if (! isSymbolPresent(symbol)){
        mAlphabet.push_back(symbol);
        return true;
    }

    return false;
}


bool Alphabet::isSymbolPresent(AlphabetSymbol symbol) const {

    /* Returns True if symbol "symbol" is in the alphabet.
     * Otherwise -returns False. */

    std::list<AlphabetSymbol>::const_iterator it = mAlphabet.begin();
    for (; it != mAlphabet.end(); ++it){
        if ((*it) == symbol)
            return true;
    }

    return false;
}


std::size_t Alphabet::symbolsCount() const {
    return mAlphabet.size();
}



/* Instruction */
Instruction::Instruction() :
    mIsFinal(false) {}

void Instruction::setReplaceble(std::string &replaceble) {

    /* Sets replaceble part of instruction.
     * Not checks if @replaceble is correct. */


#ifndef NDEBUG
    assert(! replaceble.empty());
#endif

    mReplaceble = replaceble;
}


void Instruction::setReplacer(std::string &replacer) {

    /* Sets replacer part of instruction.
     * Not checks if @replacer is correct. */


#ifndef NDEBUG
    assert(! replacer.empty());
#endif

    mReplacer = replacer;
}


void Instruction::setFinal(bool isFinal) {

    /* If @isFinal == true - instruction will be final.
     * (No one instruction will be executed after current instuirction).
     * Otherwise - instruction is not final.
     *
     * Not checks if @replacer is correct. */

    mIsFinal = isFinal;
}


std::string& Instruction::replaceble() {

    /* Returns replaceble part of instruction. */

#ifndef NDEBUG
    assert (! mReplaceble.empty());
#endif

    return mReplaceble;
}


std::string& Instruction::replacer() {

    /* Returns replacer part of instruction. */

#ifndef NDEBUG
    assert (! mReplacer.empty());
#endif

    return mReplacer;
}


bool Instruction::isFinal() const {

    return mIsFinal;
}


bool Instruction::isOk() const {

    /* Returns true if instruction is correct and can be executed,
     * otherwise returns fasle. */

    if (mReplaceble.empty())
        return false;

    return true;
}



/* Interpeter */
bool Interpreter::processFile(std::string &fileName) {

    /* Opens if possible file "filename", analise it's content,
     * loads alphabet and instructions, and try to execute them. */


#ifndef NDEBUG
    assert(! fileName.empty());
#endif

    /* Check for non-empty filename and try to open file. */
    if (fileName.empty()) {
        std::cout << "ERROR: No file specified. Process stopped.";
        return false;
    }

    FileLinesInputStream inputFile(fileName);
    if (! inputFile.isOk()) {
        std::cout << "Can't open file \"" << fileName << "\". Process stopped." << std::endl;
        return false;
    }


    if (! loadAlphabet(inputFile))
        return false;

    if (! loadSourceWord(inputFile))
        return false;

    if (! loadInstructions(inputFile))
        return false;

    /* Print all loaded instructions */
    std::cout << std::endl << "Loaded instructions: " << std::endl;
    printAllInstructions();

    return executeInstructions();
}


bool Interpreter::loadAlphabet(FileLinesInputStream &file) {

    /* Reads file line-by-line.
     * Checks every non-empty and non-commented line for alphabet definition.
     * Forms the list of all finded alphabetical symbols.
     *
     * TODO: Currently alphabet definition must be located in one line.
     *       Improve posibility to write the definition in several lines. */


#ifndef NDEBUG
    assert(file.isOk());
#endif

    bool fileContainsErrors = false;
    std::string line;

    while (file.nextLine(line)) {
        if (line.empty())
            continue;

        /* Ignore commented string. */
        if (line.size() >= 2 && line.at(0) == '/' && line.at(1) == '/')
            continue;

        std::size_t pos = 0;

        /* Ignore space and tab symbols from beginning of line. */
        for (; pos<line.size(); ++pos) {
            if (line.at(pos) == ' ' || line.at(pos) == '\t')
                continue;
            else
                break;
        }

        /* Check if end of the line is not reached.
         * If so - ignore this line (it contains only uninformative symbols). */
        if (pos >= line.size())
            continue;


        /* Infromative symbol detected.
         * Check if it is "T" or "t", that defines start of alphabet's enum. */
        if (line.at(pos) != 't' && line.at(pos) != 'T') {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: invalid symbol detected. "
                      << "\"T\" or \"t\" is expected as an symbol, that defines alphabet's enum."
                      << std::endl;
            fileContainsErrors = true;
            break;
        }
        else
            ++pos;

        /* Ignore space and tab symbols between "T" and "=". */
        for (; pos<line.size(); ++pos) {
            if (line.at(pos) == ' ' || line.at(pos) == '\t')
                continue;
            else
                break;
        }

        /* Check if end of the line is not reached. */
        if (pos >= line.size()) {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: unexpected end of line. \"=\" is expected."
                      << std::endl;
            fileContainsErrors = true;
            break;
        }


        /* Check for "=". */
        if (line.at(pos) != '=') {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: invalid symbol detected. \"=\" is expected."
                      << std::endl;
            fileContainsErrors = true;
            break;
        }
        else
            ++pos;

        /* Ignore space and tab symbols between "=" and "{". */
        for (; pos<line.size(); ++pos) {
            if (line.at(pos) == ' ' || line.at(pos) == '\t')
                continue;
            else
                break;
        }

        /* Check if end of the line is not reached. */
        if (pos >= line.size()) {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: unexpected end of line, that defines alphabet. "
                      << "\"{\" is expected."
                      << std::endl;
            fileContainsErrors = true;
            break;
        }


        /* Check for "{". */
        if (line.at(pos) != '{') {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: invalid symbol detected. \"{\" is expected. "
                      << std::endl;
            fileContainsErrors = true;
            return false;
        }
        else
            ++pos;

        /* Check if end of the line is not reached. */
        if (pos+1 >= line.size()) {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: unexpected end of line. Symbol of the alphabet is expected. "
                      << std::endl;
            fileContainsErrors = true;
            break;
        }


        /* Load all defined letters into alphabet. */
        for (; pos<line.size(); ++pos) {
            if (line.at(pos) == '}')
                break;


            /* If comma is overrided by "\" - add it to the alphabet.
             * Othrevise - ignore it as a separator. */
            else if (line.at(pos) == '\\') {
                if (line.size() >= pos+1 && line.at(pos+1) == ',') {
                    if (! mAlphabet.addSymbol(',')) {
                        std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                                  << "Warning: symbol \",\" is duplicated. "
                                  << std::endl;

                        /* At this instruction was checked 2 symbols of a line,
                         * so we need to increment "pos" for correct analising of all next symbols. */
                        ++pos;
                        continue;
                    }
                }
                else if (line.size() >= pos+1 && line.at(pos+1) == '\\') {
                    if (! mAlphabet.addSymbol('\\')) {
                        std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                                  << "Warning: symbol \"\\\" is duplicated. "
                                  << std::endl;

                        /* At this instruction was checked 2 symbols of a line,
                         * so we need to increment "pos" for correct analising of all next symbols. */
                        ++pos;
                        continue;
                    }
                }
                else
                    continue;
            }
            else if (line.at(pos) == ','){
                /* Ignore comma as a separator. */
                continue;
            }
            else if (line.at(pos) == '\t'){
                /* Ignore tab symbol. */
                continue;
            }
            else if (! mAlphabet.addSymbol(line.at(pos))) {

                /* Add all other symbols to alphabet */
                std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                          << "Warning: symbol \"" << line.at(pos) << "\" is duplicated. "
                          << std::endl;
                continue;
            }
        }

        /* After loading all symbols from the alphabet
         * we do not need to read next line from file. */
        break;
    }


    /* If errors occurred while file parsing - return negative result. */
    if (fileContainsErrors) {
        std::cout << "Alphabet was not loaded since errors occured. "
                  << std::endl;
        return false;
    }

    /* Check if alphabet contains symbols.
     * If not - return negative result, since algorithm can't work without alphabet. */
    if (mAlphabet.symbolsCount() == 0) {
        std::cout << "Alphabet was not loaded. "
                  << "It is possible that current file does not contains alphabet definition at all. "
                  << std::endl;
        return false;
    }

    return true;
}


bool Interpreter::loadSourceWord(FileLinesInputStream &file) {

    /* Reads file line-by-line.
     * Checks every non-empty and not-commented line for source word definition.
     *
     * TODO: Currently source word must be situated in one line.
     *       Source word that situated in several lines will be parsed incorrect. */


#ifndef NDEBUG
    assert(file.isOk());
#endif

    /* Add system symbols to the alphabet. */
    mAlphabet.addSymbol('!');
    mAlphabet.addSymbol('@');


    bool fileContainsErrors = false;
    std::string line;

    while (file.nextLine(line)) {
        if (line.empty())
            continue;

        /* Ignore commented string. */
        if (line.size() >= 2 && line.at(0) == '/' && line.at(1) == '/')
            continue;

        std::size_t pos = 0;

        /* Ignore space and tab symbols from beginning of line. */
        for (; pos<line.size(); ++pos) {
            if (line.at(pos) == ' ' || line.at(pos) == '\t')
                continue;
            else
                break;
        }

        /* Check if end of the line is not reached.
         * If so - ignore this line (it contains only uninformative symbols). */
        if (pos >= line.size())
            continue;


        /* Infromative symbol detected.
         * Check if it is "V" or "v", that defines start of source word. */
        if (line.at(pos) != 'v' && line.at(pos) != 'V') {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: invalid symbol detected. "
                      << "\"V\" or \"v\" is expected as an symbol, that defines beginnning of the source word."
                      << std::endl;
            fileContainsErrors = true;
            break;
        }
        else
            ++pos;

        /* Ignore space and tab symbols between "V" and "=". */
        for (; pos<line.size(); ++pos) {
            if (line.at(pos) == ' ' || line.at(pos) == '\t')
                continue;
            else
                break;
        }

        /* Check if end of the line is not reached. */
        if (pos >= line.size()) {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: unexpected end of line. \"=\" is expected."
                      << std::endl;
            fileContainsErrors = true;
            break;
        }



        /* Check for "=". */
        if (line.at(pos) != '=') {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: invalid symbol detected. \"=\" is expected."
                      << std::endl;
            fileContainsErrors = true;
            break;
        }
        else
            ++pos;

        /* Ignore space and tab symbols between "=" and first symbol of source word. */
        for (; pos<line.size(); ++pos) {
            if (line.at(pos) == ' ' || line.at(pos) == '\t')
                continue;
            else
                break;
        }

        /* Check if end of the line is not reached. */
        if (pos >= line.size()) {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: unexpected end of line, that defines source word. "
                      << std::endl;
            fileContainsErrors = true;
            break;
        }


        /* Read symbol per symbol.
         * If detected symbol is in the loaded alphabet - add it to the source word,
         * otherwise - show warning. */
        for (; pos<line.size(); ++pos) {

            /* Ignore commented end of line */
            if (line.at(pos) == '/' && line.size() >= pos+1 && line.at(pos+1) == '/')
                break;

            /* Ignore tab symbol. */
            if (line.at(pos) == '\t')
                continue;

            if (! mAlphabet.isSymbolPresent(line.at(pos))) {
                std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                          << "WARNING: detected symbol \"" << line.at(pos) << "\" is absent in the loaded alphabet. "
                          << std::endl;
            }

            mSourceWord.push_back(line.at(pos));
        }

        break;
    }


    /* If errors occurred while file parsing - return negative result. */
    if (fileContainsErrors) {
        std::cout << "Source word was not loaded since errors occured. "
                  << std::endl;
        return false;
    }

    /* Check if source word contains symbols.
     * If not - show warning. */
    if (mSourceWord.size() == 0) {
        std::cout << "Warning: source word is empty. "
                  << std::endl;
    }

    return true;
}


bool Interpreter::loadInstructions(FileLinesInputStream &file) {

    /* Reads file line-by-line.
     * Checks every non-empty and not-commented line for instruction definition. */


#ifndef NDEBUG
    assert(file.isOk());
#endif

    bool fileContainsErrors = false;
    std::string line;

    Instruction instruction;

    while (file.nextLine(line)) {
        if (line.empty())
            continue;

        /* Ignore commented string. */
        if (line.size() >= 2 && line.at(0) == '/' && line.at(1) == '/')
            continue;

        std::size_t pos = 0;

        /* Ignore space and tab symbols from beginning of line. */
        for (; pos<line.size(); ++pos) {
            if (line.at(pos) == ' ' || line.at(pos) == '\t')
                continue;
            else
                break;
        }

        /* Check if end of the line is not reached.
         * If so - ignore this line (it contains only uninformative symbols). */
        if (pos >= line.size())
            continue;


        /* Infromative symbol detected.
         * Load symbols to the instruction replaceble part. */
        std::string replaceble;
        for (; pos<line.size(); ++pos) {

            /* Stop loading replaceble part of instruction and mark it as final on "->.". */
            if (line.at(pos) == '-' && line.size() >= pos+2 && line.at(pos+1) == '>' && line.at(pos+2) == '.') {
                pos += 3;
                instruction.setFinal();
                break;
            }

            /* Stop loading replaceble part of instruction on "->". */
            if (line.at(pos) == '-' && line.size() >= pos+1 && line.at(pos+1) == '>') {
                pos += 2;
                break;
            }

            /* Ignore tab symbol. */
            if (line.at(pos) == '\t')
                continue;

            /* If current symbol not exists in loaded alphabet - show warning */
            if (! mAlphabet.isSymbolPresent(line.at(pos))) {
                std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                          << "WARNING: detected symbol \""
                          << line.at(pos) << "\" is absent in the loaded alphabet. "
                          << std::endl;
            }

            replaceble.push_back(line.at(pos));
        }
        instruction.setReplaceble(replaceble);

        /* Check if end of the line is not reached. */
        if (pos >= line.size()) {
            std::cout << "[" << file.currentLineNumber() << "; " << pos << "] "
                      << "Syntax error: unexpected end of line, that defines instruction. "
                      << std::endl;
            fileContainsErrors = true;
            break;
        }

        /* Load symbols to the instruction replacer part. */
        std::string replacer;
        for (; pos<line.size(); ++pos) {

            /* Stop loading on ";" */
            if (line.at(pos) == ';')
                break;

            /* Stop loading on "//" */
            if (line.at(pos) == '/' && line.size() >= pos+1 && line.at(pos+1) == '/')
                break;

            /* Ignore tab symbol. */
            if (line.at(pos) == '\t')
                continue;

            replacer.push_back(line.at(pos));
        }
        instruction.setReplacer(replacer);

        /* If instruction is not correct - display the error message. */
        if (! instruction.isOk()) {
            fileContainsErrors = true;
            std::cout << "ERROR: Instruction at line " << file.currentLineNumber() << " is invalid. "
                      << std::endl;
            continue;
        }

        mInstructions.push_back(instruction);
    }


    /* If errors occurred while file parsing - return negative result. */
    if (fileContainsErrors) {
        std::cout << "Errors occured while instructions loading. "
                  << std::endl;
        return false;
    }

    /* If no one instruction was loaded - show message and return negative result. */
    if (mInstructions.size() == 0) {
        std::cout << "No one instruction was loaded. Nothing to execute."
                  << std::endl;
        return false;
    }

    return true;
}


void Interpreter::printAllInstructions() const {

    /* Prints all loaded instructions to standard output. */


#ifndef NDEBUG
    assert(mInstructions.size() > 0);
#endif

#define NUMBER_COLUMN_WIDTH      4
#define REPLACEBLE_COLUMN_WIDTH  17
#define REPLACER_COLUMN_WIDTH    17
#define FINAL_COLUMN_WIDTH       5

    /* Caption */
    std::cout << std::setw(NUMBER_COLUMN_WIDTH)     << std::left << "N "
              << std::setw(REPLACEBLE_COLUMN_WIDTH) << std::left << "Replaceble "
              << std::setw(FINAL_COLUMN_WIDTH)      << std::left << "Type "
              << std::setw(REPLACER_COLUMN_WIDTH)   << std::left << "Replacer "
              << std::endl;

    /* Table */
    for (std::size_t i=0; i < mInstructions.size(); ++i) {
        Instruction instr = mInstructions.at(i);

        std::cout << std::setw(NUMBER_COLUMN_WIDTH)     << std::left << i+1
                  << std::setw(REPLACEBLE_COLUMN_WIDTH) << std::left << instr.replaceble();

        if (instr.isFinal())
            std::cout << std::setw(FINAL_COLUMN_WIDTH) << std::left << " ->.";
        else
            std::cout << std::setw(FINAL_COLUMN_WIDTH) << std::left << " -> ";

        std::cout << std::setw(REPLACER_COLUMN_WIDTH) << std::left << instr.replacer()
                  << std::endl;

    }
}


bool Interpreter::executeInstructions() {

    /* Executs all loaded instructions and display every step in std::cout */


#define NUMBER_COLUMN_WIDTH        4
#define INSTR_NUMBER_COLUMN_WIDTH  8

    /* Caption */
    std::cout << std::endl << "Executing process: "   << std::endl;
    std::cout << std::setw(NUMBER_COLUMN_WIDTH)       << std::left << "N "
              << std::setw(INSTR_NUMBER_COLUMN_WIDTH) << std::left << "Instr. "
              << std::left << "Source word "
              << std::endl;

    /* Executing instructions and print results. */
    for (std::size_t i=0, number=0; i<mInstructions.size(); ++i) {
        Instruction instr = mInstructions.at(i);

        if (! executeInstruction(instr)) {
            if (i+1 == mInstructions.size()) {
                /* The last instruction in a list does not execute. Exit. */
                return true;
            }
        } else {
            ++number;
            std::cout << std::setw(NUMBER_COLUMN_WIDTH) << number
                      << std::setw(INSTR_NUMBER_COLUMN_WIDTH) << i
                      << mSourceWord
                      << std::endl;

            if (instr.isFinal())
                return true;
            else {

                /* Start from first instruction. */
                i= -1;
                continue;
            }
        }
    }

    /* No one instruction was executed. */
    return false;
}


bool Interpreter::executeInstruction(Instruction &instr) {

    /* Tries to execute instruction @instr.
     * If @instr is incorrect or it can't be executed - returns false,
     * otherwise - returns true. */


#ifndef NDEBUG
    assert(instr.isOk());
#endif

    if (instr.isOk()) {
        /* Find position of first replaceble part in source word. */
        std::size_t pos = mSourceWord.find(instr.replaceble());
        if (pos == std::string::npos)
            return false;

        if (instr.replacer() == "!")
            mSourceWord = mSourceWord.erase(pos, instr.replaceble().size());
        else
            mSourceWord = mSourceWord.replace(pos, instr.replaceble().size(), instr.replacer());

        checkSystemFirstSymbol();
        checkSystemLastSymbol();
        return true;
    }

    return false;
}


void Interpreter::checkSystemFirstSymbol() {

    /* Checks if first symbol of source word is "!".
     * If not - inserts first symbol "!".  */

    if (mSourceWord.at(0) != '!')
        mSourceWord.insert(0, "!");
}


void Interpreter::checkSystemLastSymbol() {

    /* Checks if last symbol of source word is "@".
     * If not - inserts last symbol "@".  */

    if (mSourceWord.at(mSourceWord.size() - 1) != '@')
        mSourceWord.push_back('@');
}
