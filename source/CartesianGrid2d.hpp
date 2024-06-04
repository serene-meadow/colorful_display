#ifndef CartesianGrid2d_hpp
#define CartesianGrid2d_hpp true

#include <vector>
#include "Utility.hpp"

namespace Project { template<typename, typename=std::int_fast32_t> class CartesianGrid2d; }

template<typename T, typename SizeT> class Project::CartesianGrid2d {
  static_assert(not std::is_same_v<T, bool>);
  static_assert(std::is_signed_v<SizeT>);
  static_assert(std::is_integral_v<SizeT>);

  using Table = std::vector<T>;

  private:
    Table table;
    SizeT rowCount;
    SizeT colCount;

  public:
    explicit CartesianGrid2d(SizeT const setRowCount, SizeT const setColumnCount):
      table(setRowCount * setColumnCount),
      rowCount{setRowCount},
      colCount{setColumnCount}
    {
      if (setRowCount < 0) throw setRowCount;
      if (setColumnCount < 0) throw setColumnCount;
    }

    explicit CartesianGrid2d(): CartesianGrid2d(SizeT{0u}, SizeT{0u}) {};

    SizeT getFlatIndex(SizeT const row, SizeT const column) const {
      return Utility::wrapValue(row, rowCount) * colCount + Utility::wrapValue(column, colCount);
    }

    T /***/ &at(SizeT const row, SizeT const column) /***/ { return table[getFlatIndex(row, column)]; }
    T const &at(SizeT const row, SizeT const column) const { return table[getFlatIndex(row, column)]; }

    Table const &getTable() const { return table; }
};

#endif
