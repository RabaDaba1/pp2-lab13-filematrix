#pragma once

#include <iosfwd>
#include <string>
#include <cstddef> // std::size_t
#include <memory>  // std::unique_ptr
#include <fstream>


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
 * @param fileDescriptor_ deskryptor pliku o nazwie `filename_`,
 *        nie musimy za każdym razem na nowo otwierać i zamykać pliku (to trwa) **/
template<typename T, typename IndexType = std::size_t>
class FileMatrix
{
public:
    static constexpr auto extention()
    {
        return ".matrix";
    }

    /// @brief konstruktor przyjmujący nazwę pliku, wystarczy, że zapamięta on ją w zmiennej `filename_` i wyzeruje zmienne klasy
    FileMatrix(const std::string& filename): filename_(filename)
    {}

    /** @brief konstruktor przyjmujący poza nazwą pliku również informacje
     *  ile jest wierszy i kolumn, tym samym powinien on utworzyć plik
     *  i wypełnić go zawartością **/
    FileMatrix(IndexType rows, IndexType columns, const std::string& newFileNam);

    /** @brief konstruktor kopiujący, który powinien utworzyć nowy plik
     *  w tym samym katalogu i o nazwie niemalże identycznej jak `sourceMatrix.filename_`
     *  nazwą pliku ale z suffixem: `_copy` np.:
     *  `path/to/file/matrix100x100.matrix` -> `path/to/file/matrix100x100_copy.matrix`
     *  Zawartość dwóch plików powinna być dokładnie taka sama (zgodność binarna) **/
    FileMatrix(const FileMatrix& sourceMatrix);

    /** @brief konstruktor przenoszący, który zmienić nazwę pliku `sourceMatrix.filename_`
     *  na taki zawierający nazwę z suffixem `_move` w tym samym katalogu
     *  i o nazwie niemalże identycznej np.:
     *  `path/to/file/matrix100x100.matrix` -> `path/to/file/matrix100x100_move.matrix`
     *  `sourceMatrix` powinien być wyczyszczony jak w stanie po zawołaniu
     *  konstruktora przyjmującego jedynie nazwę pliku.
     *  Plik `filename_` nie musi istnieć, ale obiekt
     *  powinien się nadawać do dalszego użytkowania. **/
    FileMatrix(FileMatrix&& sourceMatrix);

    /** @brief destruktor - powinien przede wszystkim posprzątać zasoby, które się same nie posprzątają **/
    ~FileMatrix();

    /** @brief operator przypisania kopiujący, po jego zawołaniu plik
     *  `filename_` powinien mieć zawartość binarnie identyczną do `sourceMatrix.filename_` **/
    FileMatrix& operator=(const FileMatrix& sourceMatrix);

    /** @brief operator przypisania przenoszący, po jego zawołaniu plik
     *  `filename_` powinien mieć zawartość dokładnie taką jak `sourceMatrix.filename_`
     *  (najprościej zrobić `rename()` na plikach).
     *  Z kolei plik `filename_` nie musi istnieć, ale obiekt
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
    void flush();

    /** @brief iterator umożliwiający przeglądanie danych idąc wierszami, a następnie w każdym wierszu do każdej kolumny.
     *  Wystarczy, żeby to był iterator jednokierunkowy.
     *  @note Jeśli go zdefiniujesz zdefiniuj makro: `#define FILE_MATRIX_ITERATOR_DEFINED 1` **/
    struct iterator {};

    iterator begin();
    iterator end();

private: // methods:
    // TODO: zaimplementuj jesli cos potrzeba

private: // fields:
    std::string filename_;

    IndexType rows_{}, columns_{};

    mutable std::fstream fileDescriptor_;

    mutable std::unique_ptr<T[]> currentRow_;
    mutable IndexType currentRowNumber_{};
};
