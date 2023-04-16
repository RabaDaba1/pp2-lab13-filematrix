#include <vector>
#include <string>
#include <iterator>  // std::ostream_iterator<>()
#include <algorithm> // std::copy()
#include <cstdio>    // std::remove()
#include <numeric>   // std::accumulate()

#if __cplusplus >= 201703L
    #include <filesystem>
#else
    #error "Masz za stary kompilator, to zajecia informatyki, a nie spotkanie sympatykow epoki kamienia lupanego!"
#endif

#include <gtest/gtest.h>

#include "../FileMatrix.hpp"


namespace
{
using namespace std;
using namespace ::testing;


template<typename Container>
string container2String(const Container& container)
{
    const auto containerAsString = std::accumulate(begin(container), end(container), string(), [](const string& lhs, auto rhs)
    {
        return (lhs.empty() ? lhs : lhs + ",") + to_string(rhs);
    });
    return "{" + containerAsString + "}";
}

template<typename T>
T readNextValueFromFile(ifstream &myFile)
{
    T value;
    myFile.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

std::string fileInWritablePath(string_view fileName)
{
    constexpr const char* bobotEnvironmentVariableName = "BOBOT_ENV";
    const char* bobotVariable = getenv(bobotEnvironmentVariableName);
    if (bobotVariable)
    {
        constexpr const char* writableDirectory = "/tmp/";

        return std::filesystem::path(writableDirectory) / fileName;
    }
    return string(fileName);
}

template<typename T>
vector<T> readElementsFromFile(ifstream& myFile)
{
    vector<T> readValues;
    while (true)
    {
        const auto element = readNextValueFromFile<T>(myFile);
        if (myFile.eof())
        {
            break;
        }
        readValues.push_back(element);
    }
    return readValues;
}

fstream::pos_type getBinaryFileSize(const string& fileName)
{
#if defined(__cpp_lib_filesystem)
    return std::filesystem::file_size(fileName);
#else
    ifstream myFile(fileName, ios_base::in | ios_base::binary);
    if (!myFile)
    {
        throw runtime_error("File not opened! Does it exist?");
    }

    myFile.seekg(0, ios_base::end);
    return myFile.tellg();
#endif
}

bool fileExists(const string& fileName)
{
#if defined(__cpp_lib_filesystem)
    return filesystem::exists(fileName);
#else
    ifstream openedFile(fileName);
    if (openedFile.is_open())
    {
        return true;
    }
    else
    {
        return false;
    }
#endif
}
} // namespace


struct FileMatrixTester : public ::testing::Test
{
    using IndexType = std::size_t;

    vector<string> filesToRemove_;

	vector<string> filesInWorkingDirectory_beforeTest;
	vector<string> filesInWorkingDirectory_afterTest;

    void TearDown()
    {
        for (const auto& s : filesToRemove_)
        {
            if (fileExists(s))
            {
                remove(s.c_str());
            }
        }
        filesToRemove_.clear();
    }

	template<typename T>
    auto calculateExpectedFileSize(std::size_t rows, std::size_t cols, const FileMatrix<T>&)
    {
        auto sizeOfInformationAboutSizeOfMatrix = 2 * sizeof(IndexType); // rows_ and columns_
        return sizeOfInformationAboutSizeOfMatrix + rows * cols * sizeof(T);
    }

	template<typename T>
    void checkDataInFile(const string& fileName, IndexType expectedCols, IndexType expectedRows,
                         const vector<T>& dataToCheck)
    {
        ifstream myFile(fileName, ios_base::in | ios_base::binary);
        if (!myFile)
        {
            throw runtime_error("File '" + fileName + "' not opened! Does it exist?");
        }

        const auto [rows, columns] = readRowsAndColumns<IndexType>(myFile);
        ASSERT_EQ(expectedCols, columns);
        ASSERT_EQ(expectedRows, rows);

        const auto elementsFromFile = readElementsFromFile<T>(myFile);
        ASSERT_EQ(dataToCheck.size(), elementsFromFile.size());
        ASSERT_TRUE(std::equal(begin(elementsFromFile), end(elementsFromFile), begin(dataToCheck)))
                << "expected elements: " << container2String(dataToCheck)
                << ", elements in file: " << container2String(elementsFromFile);
    }

public:
    template<typename IndexType>
    std::pair<IndexType, IndexType> readRowsAndColumns(ifstream &myFile)
    {
        IndexType rowAndColumn[2]{};
        if (!myFile.read(reinterpret_cast<char*>(rowAndColumn), sizeof(rowAndColumn)))
        {
            throw std::runtime_error("The file does not contain row and column data!");
        }
        return {rowAndColumn[0], rowAndColumn[1]};
    }
};


TEST_F(FileMatrixTester, initialisationOfSingleElementMatrix_expectedFileCreated)
{
    constexpr IndexType ROWS = 1, COLS = 1;

    using Type = int;

    const string filename = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: " << filenameInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;
}

TEST_F(FileMatrixTester, initialisationOfMatrixSingleColumnButMultipleRows_expectedFileWithProperSizeCreated)
{
    constexpr IndexType ROWS = 1000, COLS = 1;

    using Type = float;

    const string filename = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;
}

TEST_F(FileMatrixTester, initialisationOfMatrixMultipleColumnAndSingleRow_expectedFileWithProperSizeCreated)
{
    constexpr IndexType ROWS = 1, COLS = 1000;

    using Type = double;

    const string filename = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;
}

TEST_F(FileMatrixTester, initialisationOfSquareMatrixMultipleColumnAndMultipleRows_expectedFileWithProperSizeCreated)
{
    constexpr IndexType ROWS = 100, COLS = 100;

    using Type = long long;

    const string filename = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;
}

TEST_F(FileMatrixTester, accesingMatrixDataWithTooGreatIndex_expectedThrowingOutOfRangeException)
{
    constexpr IndexType ROWS = 5, COLS = 10;

    using Type = std::size_t;

    const string filename = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    const unsigned rowIndexOutOfRange = ROWS + 1;
    ASSERT_THROW(matrix[rowIndexOutOfRange], out_of_range);

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;
}

TEST_F(FileMatrixTester, accesingMatrixElementsWithIndex_expectedSucessfullWriteAndRead)
{
    constexpr IndexType ROWS = 2, COLS = 3;

    using Type = char;

    const string filename = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    vector<Type> valuesWhichIExpect;
    valuesWhichIExpect.reserve(ROWS * COLS);
    for (remove_const_t<decltype(ROWS)> r = 0; r < ROWS; ++r)
        for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
        {
            const auto valueToAdd = r*COLS + c;
            matrix[r][c] = valueToAdd;

            valuesWhichIExpect.push_back(valueToAdd);
        }
    matrix.flush(); // to save current row to file

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;

    checkDataInFile(filenameInProperPath, COLS, ROWS, valuesWhichIExpect);
}

TEST_F(FileMatrixTester, comparingOfMatrixesWithTheSameData_expectedComparationReturnsTrue)
{
    constexpr IndexType ROWS = 2, COLS = 3;

    using Type = int_fast8_t;

    const string filename1 = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath1 = fileInWritablePath(filename1);
    filesToRemove_.push_back(filenameInProperPath1);

    const string filename2 = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath2 = fileInWritablePath(filename1);
    filesToRemove_.push_back(filenameInProperPath2);


    FileMatrix<Type,IndexType> matrix1(ROWS, COLS, filenameInProperPath1);
    FileMatrix<Type,IndexType> matrix2(ROWS, COLS, filenameInProperPath2);

    vector<Type> valuesWhichIExpect;
    valuesWhichIExpect.reserve(ROWS * COLS);
    for (remove_const_t<decltype(ROWS)> r = 0; r < ROWS; ++r)
        for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
        {
            const auto valueToAdd = r*COLS + c;
            matrix1[r][c] = valueToAdd;
            matrix2[r][c] = valueToAdd;

            valuesWhichIExpect.push_back(valueToAdd);
        }
    matrix1.flush(); // to save current row to file
    matrix2.flush();

    ASSERT_EQ(matrix1, matrix2);

    ASSERT_TRUE(fileExists(filenameInProperPath1)) << "file path: "  << filenameInProperPath1;
    ASSERT_TRUE(fileExists(filenameInProperPath2)) << "file path: "  << filenameInProperPath2;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix1), getBinaryFileSize(filenameInProperPath1)) << "file path: "  << filenameInProperPath1;
    ASSERT_EQ(getBinaryFileSize(filenameInProperPath1), getBinaryFileSize(filenameInProperPath2));

    checkDataInFile(filenameInProperPath1, COLS, ROWS, valuesWhichIExpect);
    checkDataInFile(filenameInProperPath2, COLS, ROWS, valuesWhichIExpect);
}

TEST_F(FileMatrixTester, comparingDifferentMatrixesWithTheSameNumberOfRowsAndColumnsButDifferentData_expectedComparationReturnsFalse)
{
    constexpr IndexType ROWS = 2, COLS = 3;

    using Type = int_fast64_t;

    const string filename1 = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath1 = fileInWritablePath(filename1);
    filesToRemove_.push_back(filenameInProperPath1);

    const string filename2 = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath2 = fileInWritablePath(filename2);
    filesToRemove_.push_back(filenameInProperPath2);


    FileMatrix<Type,IndexType> matrix1(ROWS, COLS, filenameInProperPath1);
    FileMatrix<Type,IndexType> matrix2(ROWS, COLS, filenameInProperPath2);

    for (remove_const_t<decltype(ROWS)> r = 0; r < ROWS; ++r)
        for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
        {
            const auto valueToAdd = r*COLS + c;
            matrix1[r][c] = valueToAdd;
            matrix2[r][c] = valueToAdd;
        }

    matrix1.flush(); // to save current row to file
    matrix2.flush();

    for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
    {
        matrix2[ROWS-1][c] *= 2;
    }

    matrix2.flush(); // to save current row to file


    ASSERT_NE(matrix1, matrix2);

    ASSERT_TRUE(fileExists(filenameInProperPath1)) << "file path: "  << filenameInProperPath1;
    ASSERT_TRUE(fileExists(filenameInProperPath2)) << "file path: "  << filenameInProperPath2;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix1), getBinaryFileSize(filenameInProperPath1)) << "file path: "  << filenameInProperPath1;
    ASSERT_EQ(getBinaryFileSize(filenameInProperPath1), getBinaryFileSize(filenameInProperPath2));
}

TEST_F(FileMatrixTester, copyConstruction_expectedFileCopiedWithEqualBinaryData)
{
    constexpr IndexType ROWS = 100, COLS = 200;

    using Type = int32_t;

    const auto filenameBase = "testfile_" + to_string(__LINE__);
    const string filename = filenameBase + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    const string expectedfilenameOfCopiedMatrix = filenameBase + "_copy" + FileMatrix<Type>::extention();
    const string filenameOfCopiedMatrixInProperPath = fileInWritablePath(expectedfilenameOfCopiedMatrix);
    filesToRemove_.push_back(filenameOfCopiedMatrixInProperPath);


    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    vector<Type> valuesWhichIExpect;
    valuesWhichIExpect.reserve(ROWS * COLS);
    for (remove_const_t<decltype(ROWS)> r = 0; r < ROWS; ++r)
        for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
        {
            const auto valueToAdd = r*COLS + c;
            matrix[r][c] = valueToAdd;

            valuesWhichIExpect.push_back(valueToAdd);
        }
    matrix.flush(); // to save current row to file

    auto matrixCopy = matrix;

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_TRUE(fileExists(filenameOfCopiedMatrixInProperPath)) << "file path: "  << filenameOfCopiedMatrixInProperPath; // testfile_301_copy.matrix
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_EQ(getBinaryFileSize(filenameInProperPath), getBinaryFileSize(filenameOfCopiedMatrixInProperPath));

    checkDataInFile(filenameInProperPath, COLS, ROWS, valuesWhichIExpect);

    ASSERT_EQ(matrix, matrixCopy);
}

TEST_F(FileMatrixTester, assignmentOperatorCopying_expectedMatricesTheSame)
{
    constexpr IndexType ROWS = 300, COLS = 100;

    using Type = unsigned long long;

    const auto filenameBase = "testfile_" + to_string(__LINE__);
    const string filename = filenameBase + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    const string expectedfilenameOfCopiedMatrix = filenameBase + "_copy" + FileMatrix<Type>::extention();
    const string filenameOfCopiedMatrixInProperPath = fileInWritablePath(expectedfilenameOfCopiedMatrix);
    filesToRemove_.push_back(filenameOfCopiedMatrixInProperPath);


    FileMatrix<Type,IndexType> matrix1(ROWS, COLS, filenameInProperPath);
    FileMatrix<Type,IndexType> matrix2(COLS, ROWS, filenameOfCopiedMatrixInProperPath);

    vector<Type> valuesWhichIExpect;
    valuesWhichIExpect.reserve(ROWS * COLS);
    for (remove_const_t<decltype(ROWS)> r = 0; r < ROWS; ++r)
        for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
        {
            const auto valueToAdd = r*COLS + c;
            matrix1[r][c] = valueToAdd;

            valuesWhichIExpect.push_back(valueToAdd);
        }
    matrix1.flush(); // to save current row to file

    matrix2 = matrix1;

    matrix2.flush();

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_TRUE(fileExists(filenameOfCopiedMatrixInProperPath)) << "file path: "  << filenameOfCopiedMatrixInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix1), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_EQ(getBinaryFileSize(filenameInProperPath), getBinaryFileSize(filenameOfCopiedMatrixInProperPath));

    checkDataInFile(filenameInProperPath, COLS, ROWS, valuesWhichIExpect);
    checkDataInFile(filenameOfCopiedMatrixInProperPath, COLS, ROWS, valuesWhichIExpect);

    ASSERT_EQ(matrix1, matrix2);
}

TEST_F(FileMatrixTester, moveConstruction_expectedFileMovedButSourceObjectUsable)
{
    constexpr IndexType ROWS = 100, COLS = 200;

    using Type = int16_t;

    const auto filenameBase = "testfile_" + to_string(__LINE__);
    const string filename = filenameBase + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    const string expectedfilenameOfCopiedMatrix = filenameBase + "_move" + FileMatrix<Type>::extention();
    const string filenameOfCopiedMatrixInProperPath = fileInWritablePath(expectedfilenameOfCopiedMatrix);
    filesToRemove_.push_back(filenameOfCopiedMatrixInProperPath);


    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    vector<Type> valuesWhichIExpect;
    valuesWhichIExpect.reserve(ROWS * COLS);
    for (remove_const_t<decltype(ROWS)> r = 0; r < ROWS; ++r)
        for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
        {
            const auto valueToAdd = r*COLS + c;
            matrix[r][c] = valueToAdd;

            valuesWhichIExpect.push_back(valueToAdd);
        }
    matrix.flush(); // to save current row to file

    const auto matrixMoved = std::move(matrix);

    ASSERT_TRUE(fileExists(filenameOfCopiedMatrixInProperPath)) << "file path: " << filenameOfCopiedMatrixInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameOfCopiedMatrixInProperPath)) << "file path: "  << filenameInProperPath;

    if (fileExists(filenameInProperPath))
    {
        ASSERT_LT(getBinaryFileSize(filenameInProperPath), getBinaryFileSize(filenameOfCopiedMatrixInProperPath));
    }

    checkDataInFile(filenameOfCopiedMatrixInProperPath, COLS, ROWS, valuesWhichIExpect);

    ASSERT_NE(matrix, matrixMoved);
    ASSERT_EQ(IndexType{}, matrix.columns());
    ASSERT_EQ(IndexType{}, matrix.rows());

    matrix = matrixMoved;

    ASSERT_EQ(matrix, matrixMoved);
}

TEST_F(FileMatrixTester, assignmentOperatorMoving_expectedFileMovedButSourceObjectUsable)
{
    constexpr IndexType ROWS = 200, COLS = 100;

    using Type = unsigned long long;

    const auto filenameBase = "testfile_" + to_string(__LINE__);
    const string filename = filenameBase + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    const string expectedfilenameOfCopiedMatrix = filenameBase + "_move" + FileMatrix<Type>::extention();
    const string filenameOfCopiedMatrixInProperPath = fileInWritablePath(expectedfilenameOfCopiedMatrix);
    filesToRemove_.push_back(filenameOfCopiedMatrixInProperPath);


    FileMatrix<Type,IndexType> matrix1(ROWS, COLS, filenameInProperPath);
    FileMatrix<Type,IndexType> matrix2(filenameOfCopiedMatrixInProperPath);

    vector<Type> valuesWhichIExpect;
    valuesWhichIExpect.reserve(ROWS * COLS);
    for (remove_const_t<decltype(ROWS)> r = 0; r < ROWS; ++r)
        for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
        {
            const auto valueToAdd = r*COLS + c;
            matrix1[r][c] = valueToAdd;

            valuesWhichIExpect.push_back(valueToAdd);
        }
    matrix1.flush(); // to save current row to file

    matrix2 = std::move(matrix1);

    matrix2.flush();

    ASSERT_TRUE(fileExists(filenameOfCopiedMatrixInProperPath)) << "file path: "  << filenameOfCopiedMatrixInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix1), getBinaryFileSize(filenameOfCopiedMatrixInProperPath)) << "file path: "  << filenameInProperPath;
    if (fileExists(filenameInProperPath))
    {
        ASSERT_LT(getBinaryFileSize(filenameInProperPath), getBinaryFileSize(filenameOfCopiedMatrixInProperPath));
    }

    checkDataInFile(filenameOfCopiedMatrixInProperPath, COLS, ROWS, valuesWhichIExpect);

    ASSERT_NE(matrix1, matrix2);
    ASSERT_EQ(IndexType{}, matrix1.columns());
    ASSERT_EQ(IndexType{}, matrix1.rows());

    matrix1 = matrix2;

    ASSERT_EQ(matrix1, matrix2);
}

TEST_F(FileMatrixTester, containerHasBeginEndMethodsReturningIterator)
{
    constexpr IndexType ROWS = 2, COLS = 3;

    using Type = char;

    const string filename = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    vector<Type> valuesWhichIExpect;
    valuesWhichIExpect.reserve(ROWS * COLS);
    for (remove_const_t<decltype(ROWS)> r = 0; r < ROWS; ++r)
        for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
        {
            const auto valueToAdd = r*COLS + c;
            matrix[r][c] = valueToAdd;

            valuesWhichIExpect.push_back(valueToAdd);
        }
    matrix.flush(); // to save current row to file

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;

    checkDataInFile(filenameInProperPath, COLS, ROWS, valuesWhichIExpect);

#ifndef UNDEFINED_FILE_MATRIX_ITERATOR
    auto iteratorBegin = matrix.begin();
    auto iteratorEnd = matrix.end();
    ASSERT_NE(iteratorBegin, iteratorEnd);
#else
    ADD_FAILURE() << "Iterator not implemented!";
#endif
}

TEST_F(FileMatrixTester, containerHasIteratowWorkingWithForLoop)
{
    constexpr IndexType ROWS = 2, COLS = 3;

    using Type = int32_t;

    const string filename = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    vector<Type> valuesWhichIExpect;
    valuesWhichIExpect.reserve(ROWS * COLS);
    for (remove_const_t<decltype(ROWS)> r = 0; r < ROWS; ++r)
        for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
        {
            const auto valueToAdd = r*COLS + c;
            matrix[r][c] = valueToAdd;

            valuesWhichIExpect.push_back(valueToAdd);
        }
    matrix.flush(); // to save current row to file

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;

    checkDataInFile(filenameInProperPath, COLS, ROWS, valuesWhichIExpect);

#ifndef UNDEFINED_FILE_MATRIX_ITERATOR
    IndexType i = 0;
    for (auto element : matrix)
    {
        ASSERT_EQ(valuesWhichIExpect[i], element) << "index:" << i;
        ++i;
    }
#else
    ADD_FAILURE() << "Iterator not implemented!";
#endif
}

TEST_F(FileMatrixTester, containerHasIteratowWorkingWithStl)
{
    constexpr IndexType ROWS = 2, COLS = 3;

    using Type = int32_t;

    const string filename = "testfile_" + to_string(__LINE__) + FileMatrix<Type>::extention();
    const string filenameInProperPath = fileInWritablePath(filename);
    filesToRemove_.push_back(filenameInProperPath);

    FileMatrix<Type,IndexType> matrix(ROWS, COLS, filenameInProperPath);

    vector<Type> valuesWhichIExpect;
    valuesWhichIExpect.reserve(ROWS * COLS);
    for (remove_const_t<decltype(ROWS)> r = 0; r < ROWS; ++r)
        for (remove_const_t<decltype(COLS)> c = 0; c < COLS; ++c)
        {
            const auto valueToAdd = r*COLS + c;
            matrix[r][c] = valueToAdd;

            valuesWhichIExpect.push_back(valueToAdd);
        }
    matrix.flush(); // to save current row to file

    ASSERT_TRUE(fileExists(filenameInProperPath)) << "file path: "  << filenameInProperPath;
    ASSERT_EQ(calculateExpectedFileSize(ROWS, COLS, matrix), getBinaryFileSize(filenameInProperPath)) << "file path: "  << filenameInProperPath;

    checkDataInFile(filenameInProperPath, COLS, ROWS, valuesWhichIExpect);

#ifndef UNDEFINED_FILE_MATRIX_ITERATOR
    auto iteratorBegin = matrix.begin();
    auto iteratorEnd = matrix.end();
    ASSERT_NE(iteratorBegin, iteratorEnd);

    ASSERT_TRUE(std::equal(begin(valuesWhichIExpect), end(valuesWhichIExpect),
                           begin(matrix), end(matrix)));
#else
    ADD_FAILURE() << "Iterator not implemented!";
#endif
}
