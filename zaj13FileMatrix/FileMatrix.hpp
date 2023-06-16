#pragma once

#include <iosfwd>
#include <string>
#include <cstddef> // std::size_t
#include <memory>  // std::unique_ptr
#include <fstream>
#include <sstream>

/** @file FileMatrix.hpp
    @date 16 kwietnia 2023
    @brief W ramach zadania trzeba zaimplementowac wszystkie opisane metody szablonu klasy FileMatrix.
    Do ponizszych metod **sa testy** w pliku @ref FileMatrixTests.cpp.
    FileMatrix jest szablonem klasy, dlatego też musi mieć implementacje w pliku nagłówkowym,
    oczywiście polecam dłuższych metod nie definiować w klasie, tylko pod nią.
    Jeśli by ktoś chciał mimo wszystko dokonać definicji w pliku źródłowym,
    może zastosować tzw. [Explicit instantiation definition](https://en.cppreference.com/w/cpp/language/class_template)
    **dla każdego z typów!**
**/

#define UNDEFINED_FILE_MATRIX_ITERATOR


/** class FileMatrix
 * @brief Szablon klasy FileMatrix jest macierzą elementów o określonym typie,
 * ale trudność polega na tym, że z założenia jest ona w stanie trzymać macierz nie mieszczącą się w pamięci,
 * poprzez trzymanie wszystkich danych na dysku, a w aktualnej pamięci podręcznej jest tylko jeden rząd danych.
 * Dane trzymane na dysku są w formacie **binarnym**, w następującej formie:
 * 1. `IndexType rows_`
 * 2. `IndexType columns_`
 * 3. `T[]` - dane zależnie od ich ilości, będzie to `rows_*columns_`
 *
 * @note Można (a nawet należy) zaimplementować dodatkowe metody.
 * @note Można (a nawet wygodniej) dodać dodatkowe składowe klasy
 *       (niewykluczone użycie kwantyfikatora `mutable`)
 *
 * @tparam T typ przetrzymywany w macierzy
 * @tparam IndexType typ do indeksowania elementow, ten typ powinien byc uzyty do skladowych `rows_`, `columns_`
 *
 * @param filename_ pełna nazwa pliku (czyli wraz ze ścieżką i rozszerzeniem)
 * @param rows_ ilość rzędów w macierzy prostokątnej
 * @param columns_ ilość kolumn w macierzy prostokątnej
 * @param currentRow_ jest to ostatnio odczytywany rząd macierzy,
 *        dzięki niemu możemy ograniczyć lekko operacje dyskowe
 * @param currentRowNumber_ informacja którym rzędem jest powyższy
 * @param fileDescriptor_ deskryptor pliku o nazwie `fname_`,
 *        nie musimy za każdym razem na nowo otwierać i zamykać pliku (to trwa) **/
template<typename T, typename IndexType = std::size_t>
class FileMatrix
{
public:
    static constexpr auto extention()
    {
        return ".matrix";
    }

    /// @brief konstruktor przyjmujący nazwę pliku, wystarczy, że zapamięta on ją w zmiennej `fname_` i wyzeruje zmienne klasy
    FileMatrix(const std::string& filename): filename_(filename)
    {}

    /** @brief konstruktor przyjmujący poza nazwą pliku również informacje
     *  ile jest wierszy i kolumn, tym samym powinien on utworzyć plik
     *  i wypełnić go zawartością **/
    FileMatrix(IndexType rows, IndexType columns, const std::string& newFileNam);

    /** @brief konstruktor kopiujący, który powinien utworzyć nowy plik
     *  w tym samym katalogu i o nazwie niemalże identycznej jak `sourceMatrix.fname_`
     *  nazwą pliku ale z suffixem: `_copy` np.:
     *  `path/to/file/matrix100x100.matrix` -> `path/to/file/matrix100x100_copy.matrix`
     *  Zawartość dwóch plików powinna być dokładnie taka sama (zgodność binarna) **/
    FileMatrix(const FileMatrix& sourceMatrix);

    /** @brief konstruktor przenoszący, który zmienić nazwę pliku `sourceMatrix.fname_`
     *  na taki zawierający nazwę z suffixem `_move` w tym samym katalogu
     *  i o nazwie niemalże identycznej np.:
     *  `path/to/file/matrix100x100.matrix` -> `path/to/file/matrix100x100_move.matrix`
     *  `sourceMatrix` powinien być wyczyszczony jak w stanie po zawołaniu
     *  konstruktora przyjmującego jedynie nazwę pliku.
     *  Plik `fname_` nie musi istnieć, ale obiekt
     *  powinien się nadawać do dalszego użytkowania. **/
    FileMatrix(FileMatrix&& sourceMatrix);

    /** @brief destruktor - powinien przede wszystkim posprzątać zasoby, które się same nie posprzątają **/
    ~FileMatrix();

    /** @brief operator przypisania kopiujący, po jego zawołaniu plik
     *  `fname_` powinien mieć zawartość binarnie identyczną do `sourceMatrix.fname_` **/
    FileMatrix& operator=(const FileMatrix& sourceMatrix);

    /** @brief operator przypisania przenoszący, po jego zawołaniu plik
     *  `fname_` powinien mieć zawartość dokładnie taką jak `sourceMatrix.fname_`
     *  (najprościej zrobić `rename()` na plikach).
     *  Z kolei plik `fname_` nie musi istnieć, ale obiekt
     *  powinien się nadawać do dalszego użytkowania. **/
    FileMatrix& operator=(FileMatrix&& sourceMatrix);

    auto rows() const
	{
        return rows_;
	}

    auto columns() const
	{
        return columns_;
	}

    const auto& filename() const
	{
        return filename_;
	}

    /** @brief operator indeksowania, który zwraca wskaźnik do wskazanego wiersza macierzy.
     *  W razie jeśli w pamięci jest inny należy zrzucić jego dane na dysk i wczytać właściwy wiersz, następnie go zwrócić.
     *  @note Tym sposobem zadziała zawołanie `matrix[row][column]`,
     *        jednakże nie jesteśmy w stanie sprawdzić, czy `column` nie wychodzi poza zakres
     *  **/
    T* operator[](IndexType indexOfRow);

    /** @brief operator indeksowania stały, analogiczny jak operator indeksowania niestały.
     *  @note Aby on działał w analogiczny sposób pewne składowe zapewne będą musiały mieć przedrostem `mutable`. **/
    const T* operator[](IndexType indexOfRow) const;

    bool operator==(const FileMatrix& matrix) const;

	bool operator!=(const FileMatrix& matrix) const
	{
		return !((*this) == matrix);
	}

    /** @brief operator mnożenia całej macierzy przez wartość **/
    FileMatrix& operator*=(const T& value);

    /** @brief metoda zrzucająca aktualnie trzymany wiersz w pamięci na dysk.
     *  @note zalecam też aby wołała `fstream::flush()`, aby buforowalne dane zostały mimo wszystko od razu wrzucone na dysk **/
    void flush() const {
        fileDescriptor_.seekp(2*sizeof(IndexType) + currentRowNumber_ * columns_ * sizeof(T), std::ios::beg);
        fileDescriptor_.write(reinterpret_cast<char *>(currentRow_.get()), sizeof(T)*columns_);

        fileDescriptor_.flush();
    };

    /** @brief iterator umożliwiający przeglądanie danych idąc wierszami, a następnie w każdym wierszu do każdej kolumny.
     *  Wystarczy, żeby to był iterator jednokierunkowy.
     *  @note Jeśli go zdefiniujesz zdefiniuj makro: `#define FILE_MATRIX_ITERATOR_DEFINED 1` **/
    struct iterator {
//        std::string fname_ {};
//        mutable std::fstream descriptor_ {};
//        IndexType dim_rows_, dim_cols_;
//        IndexType position_ {};
//
//        iterator(const FileMatrix& matrix, IndexType pos = {}): fname_(matrix.filename_), dim_rows_(matrix.rows_), dim_cols_(matrix.columns_), position_(pos) {
//            descriptor_ = std::fstream(fname_, std::ios::in | std::ios::out | std::ios::binary);
//        };
//
//        iterator(const iterator& sourceIterator): fname_(sourceIterator.fname_), dim_rows_(sourceIterator.dim_rows_), dim_cols_(sourceIterator.dim_cols_), position_(sourceIterator.position_) {
//            descriptor_ = std::fstream(fname_, std::ios::in | std::ios::out | std::ios::binary);
//        }
//
//        iterator& operator++() {
//            position_++;
//            return *this;
//        };
//        iterator& operator--() {
//            position_--;
//            return *this;
//        };
//
//        bool operator==(const iterator &anotherIt) const { return fname_ == anotherIt.fname_ && position_ == anotherIt.position_; }
//        bool operator!=(const iterator& anotherIt) const { return fname_ != anotherIt.fname_ || position_ != anotherIt.position_; }
//
//        bool operator<(const iterator& rhs) const {return position_ < rhs.position_; }
//        bool operator>(const iterator& rhs) const {return position_ > rhs.position_; }
//
//        int operator*() const {
//            descriptor_.seekg(2*sizeof(IndexType) + position_ * sizeof(T), std::ios::beg);
//
//            char *result {};
//            descriptor_.read(result, sizeof(T));
//
//            std::stringstream ss(result);
//            int num;
//            ss >> num;
//
//            return num;
//        }
    };

    iterator begin() {
//        return iterator(*this, 0);
    }
    iterator end() {
//        return iterator(*this, rows_*columns_);
    }

private: // methods:
    // TODO: zaimplementuj jesli cos potrzeba
    void setFilename(const std::string& filename, const std::string& suffix = "") {
        filename_ = filename;
        std::size_t pos = filename_.rfind('.');

        if (pos != std::string::npos)
            filename_.insert(pos, suffix);
    }

    void copy(const FileMatrix& sourceMatrix) {
        // Copy attributes
        rows_ = sourceMatrix.rows_;
        columns_ = sourceMatrix.columns_;
        currentRowNumber_ = sourceMatrix.currentRowNumber_;

        // Create new binary file
        std::fstream output(filename_, std::ios::out | std::ios::binary);

        // Copy source file contents
        sourceMatrix.fileDescriptor_.seekg(0, std::ios::beg);
        sourceMatrix.fileDescriptor_.seekp(0, std::ios::beg);
        output << sourceMatrix.fileDescriptor_.rdbuf();
        output.close();

        // Copy the currentRow_
        currentRow_ = std::make_unique<T[]>(rows_);
        std::copy(sourceMatrix.currentRow_.get(), sourceMatrix.currentRow_.get() + rows_, currentRow_.get());

        fileDescriptor_ = std::fstream(filename_, std::ios::out | std::ios::in | std::ios::binary);
    }

    void move(FileMatrix &&sourceMatrix) {
        // Move attributes
        rows_ = std::exchange(sourceMatrix.rows_, 0);
        columns_ = std::exchange(sourceMatrix.columns_, 0);
        currentRow_ = std::move(sourceMatrix.currentRow_);
        currentRowNumber_ = std::exchange(sourceMatrix.currentRowNumber_, 0);
        fileDescriptor_ = std::fstream(filename_, std::ios::out | std::ios::in | std::ios::binary);

        // Rename binary file
        std::rename(sourceMatrix.filename_.data(), filename_.data());

        // Clear
    }

    void loadRow(IndexType indexOfRow) const {
        if (indexOfRow >= rows_)
            throw std::out_of_range("");

        // Write current row to file if there is one loaded
        if(currentRow_)
            flush();

        currentRowNumber_ = indexOfRow;

        // Move descriptor to the position of the i-th row
        fileDescriptor_.seekg(2*sizeof(IndexType) + currentRowNumber_ * columns_ * sizeof(T), std::ios::beg);
        fileDescriptor_.seekp(2*sizeof(IndexType) + currentRowNumber_ * columns_ * sizeof(T), std::ios::beg);

        // Read the row into a dynamically allocated array
        currentRow_ = std::make_unique<T[]>(columns_);
        fileDescriptor_.read(reinterpret_cast<char*>(currentRow_.get()), columns_ * sizeof(T));
    }

private: // fields:
    std::string filename_;

    IndexType rows_{}, columns_{};

    mutable std::fstream fileDescriptor_;

    mutable std::unique_ptr<T[]> currentRow_;
    mutable IndexType currentRowNumber_ {};
};

template<typename T, typename IndexType>
FileMatrix<T, IndexType>::~FileMatrix() {
    fileDescriptor_.close();
    currentRow_.reset();
}

/** CONSTRUCTORS **/
template<typename T, typename IndexType>
FileMatrix<T, IndexType>::FileMatrix(IndexType rows, IndexType columns, const std::string &newFileName): rows_(rows), columns_(columns), filename_(newFileName) {
    // Create a file
    std::fstream file(filename_, std::ios::out | std::ios::trunc | std::ios::binary);

    // Fill file with rows and columns and rest with 0
    file.write(reinterpret_cast<char*>(&rows_), sizeof(IndexType));
    file.write(reinterpret_cast<char*>(&columns_), sizeof(IndexType));

    T buff {};
    for (int i = 0; i < rows_*columns_; i++)
        file.write(reinterpret_cast<char*>(&buff), sizeof(T));

    file.close();

    fileDescriptor_ = std::fstream(filename_, std::ios::out | std::ios::in | std::ios::binary);
    loadRow(IndexType{});
}

// Copy constructor
template<typename T, typename IndexType>
FileMatrix<T, IndexType>::FileMatrix(const FileMatrix &sourceMatrix) {
    setFilename(sourceMatrix.filename_, "_copy");

    this->copy(sourceMatrix);
}

// Move constructor
template<typename T, typename IndexType>
FileMatrix<T, IndexType>::FileMatrix(FileMatrix &&sourceMatrix) {
    setFilename(sourceMatrix.filename_, "_move");

    this->move(std::move(sourceMatrix));
}


/** ASSIGNMENT OPERATORS **/
template<typename T, typename IndexType>
FileMatrix<T, IndexType> &FileMatrix<T, IndexType>::operator=(const FileMatrix &sourceMatrix) {
    if (this != &sourceMatrix)
        this->copy(sourceMatrix);

    return *this;
}

template<typename T, typename IndexType>
FileMatrix<T, IndexType> &FileMatrix<T, IndexType>::operator=(FileMatrix &&sourceMatrix) {
    if (this != &sourceMatrix)
        this->move(std::move(sourceMatrix));

    return *this;
}

/** OVERLOADED OPERATORS **/
template<typename T, typename IndexType>
T *FileMatrix<T, IndexType>::operator[](IndexType indexOfRow) {
    loadRow(indexOfRow);

    return currentRow_.get();
}

template<typename T, typename IndexType>
const T *FileMatrix<T, IndexType>::operator[](IndexType indexOfRow) const {
    loadRow(indexOfRow);

    return currentRow_.get();
}

template<typename T, typename IndexType>
bool FileMatrix<T, IndexType>::operator==(const FileMatrix &matrix) const {
    // Check if the dimensions of the two matrices are equal
    if (rows_ != matrix.rows_ || columns_ != matrix.columns_)
        return false;

    // Compare the contents of the two matrices
    fileDescriptor_.seekg(0, std::ifstream::beg);
    matrix.fileDescriptor_.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(fileDescriptor_.rdbuf()),
                      std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(matrix.fileDescriptor_.rdbuf()));

    // The two matrices are equal
    return true;
}