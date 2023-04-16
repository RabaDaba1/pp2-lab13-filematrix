#include <iostream>
#include "FileMatrix.hpp"


/// @todo Uzupelnij swoje dane:
//constexpr const char* const FIRSTNAME = "";
//constexpr const char* const SURNAME   = "";
//constexpr const char* const MAIL    = "";
//constexpr const char* const BOOK_ID = "";
constexpr const char* const FIRSTNAME = "Grzegorz";
constexpr const char* const SURNAME   = "Bazior";
constexpr const char* const MAIL    = "bazior@agh.edu.pl";
constexpr const char* const BOOK_ID = "123456";
constexpr const char* const TEACHER_MAIL = "bazior[at]agh.edu.pl"; // change if not correct

void validateStudentsInfo();


int main()
{
    validateStudentsInfo();

    std::cout << "Hello " << FIRSTNAME << " " << SURNAME << "!" << '\n'
              << "make Your teacher: " << TEACHER_MAIL << " proud of you!" << std::endl;
}


constexpr inline size_t compileTimeStrlen(const char* text) noexcept
{
    return (*text ? 1+compileTimeStrlen(text+1) : 0);
}

constexpr inline size_t compileTimeCountFirstDigits(const char* text) noexcept
{
    return (*text && ('0'<=*text && *text<='9') ? 1+compileTimeCountFirstDigits(text+1) : 0);
}

constexpr inline bool compileTimeIsDigit(const char* text) noexcept
{
    return compileTimeStrlen(text) == compileTimeCountFirstDigits(text);
}

constexpr inline bool compileTimeContains(const char* text, char letter) noexcept
{
    if ('\n' == *text)
        return false;

    return *text == letter ? 1 : compileTimeContains(text+1, letter);
}

void validateStudentsInfo()
{
    static_assert(compileTimeStrlen(FIRSTNAME) > 2, "You must input your first name!");
    static_assert(compileTimeStrlen(SURNAME) > 2, "You must input your surname!");

    static_assert(compileTimeStrlen(BOOK_ID) > 1, "You must input your student book id!");
    static_assert(compileTimeIsDigit(BOOK_ID), "You student book id should consist only digits, shouldn't it?!");

    static_assert(compileTimeStrlen(MAIL) > 3, "You must input your mail (preferred AGH mail)!");
    static_assert(compileTimeContains(MAIL, '@'), "Invalid mail! It should contain '@'!");
    static_assert(compileTimeContains(MAIL, '.'), "Invalid mail! It should contain '.'!");
}
