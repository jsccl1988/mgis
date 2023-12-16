#ifndef GEOMETRY_BASE_MATRIX2D_H
#define GEOMETRY_BASE_MATRIX2D_H

#include "core/core.h"
#include "geometry/export.h"

#define NULLPTR (T*)(0)

namespace core {
template <class T>
class Matrix2D {
 public:
  Matrix2D(void);
  Matrix2D(int row, int col);
  virtual ~Matrix2D(void);

 public:
  void Resize(int row, int col);

  inline int GetRowCount() const { return row_size_; }
  inline int GetColCount() const { return col_size_; }

  inline void CalcMat2DIndex(int& iRow, int& jCol, int index) const;
  inline int CalcMat2DIndex(int iRow, int jCol) const;

  //////////////////////////////////////////////////////////////////////////
  inline void SetElements(T* pData, int nSize);
  inline void GetElements(T* pData, int nSize) const;
  inline void GetElementBuf(T*& pData, int& nSize) const;

  inline void SetElement(T& data, int iRow, int jCol);
  inline const T& GetElement(int iRow, int jCol) const;
  inline T& GetElement(int iRow, int jCol);

  inline const T* operator[](int iRow) const;
  inline T* operator[](int iRow);

  inline T& operator()(int iRow, int jCol);
  inline const T& operator()(int iRow, int jCol) const;

 protected:
  void Release(void);

 protected:
  int col_size_, row_size_;
  T* m_pData;
};

//////////////////////////////////////////////////////////////////////////
template <class T>
Matrix2D<T>::Matrix2D(void) : m_pData(NULLPTR), row_size_(0), col_size_(0) {}

template <class T>
Matrix2D<T>::Matrix2D(int row, int col)
    : m_pData(NULLPTR), row_size_(0), col_size_(0) {
  Resize(row, col);
}

template <class T>
Matrix2D<T>::~Matrix2D() {
  Release();
}

//////////////////////////////////////////////////////////////////////////
template <class T>
void Matrix2D<T>::Resize(int row, int col) {
  assert((col > 0) && (row > 0));

  if (col == col_size_ && row == row_size_) return;

  Release();

  col_size_ = col;
  row_size_ = row;
  m_pData = new T[row_size_ * col_size_];
  memset(m_pData, 0, row_size_ * col_size_ * sizeof(T));
}

template <class T>
void Matrix2D<T>::Release(void) {
  if (m_pData) {
    delete[] m_pData;
    m_pData = NULLPTR;
    col_size_ = 0;
    row_size_ = 0;
  }
}

template <class T>
T* Matrix2D<T>::operator[](int iRow) {
  assert(NULLPTR != m_pData && iRow < row_size_);

  return m_pData + col_size_ * iRow;
}

template <class T>
const T* Matrix2D<T>::operator[](int iRow) const {
  assert(NULLPTR != m_pData && iRow < row_size_);

  return m_pData + col_size_ * iRow;
}

template <class T>
inline T& Matrix2D<T>::operator()(int iRow, int jCol) {
  assert(NULLPTR != m_pData && iRow < row_size_ && jCol < col_size_);

  return m_pData[iRow][jCol];
}

template <class T>
inline const T& Matrix2D<T>::operator()(int iRow, int jCol) const {
  assert(NULLPTR != m_pData && iRow < row_size_ && jCol < col_size_);

  return m_pData[iRow][jCol];
}

template <class T>
inline void Matrix2D<T>::CalcMat2DIndex(int& iRow, int& jCol, int index) const {
  assert(index < (row_size_ - 1) * (col_size_ - 1));

  iRow = index / col_size_;
  jCol = index % col_size_;
}

template <class T>
inline int Matrix2D<T>::CalcMat2DIndex(int iRow, int jCol) const {
  return (col_size_ * iRow + jCol);
}

//////////////////////////////////////////////////////////////////////////
template <class T>
inline void Matrix2D<T>::SetElements(T* pData, int nSize) {
  assert(NULLPTR != m_pData && nSize == (row_size_ * col_size_));

  memcpy(m_pData, pData, sizeof(T) * nSize);
}

template <class T>
inline void Matrix2D<T>::GetElements(T* pData, int nSize) const {
  assert(NULLPTR != m_pData && nSize == (row_size_ * col_size_));

  memcpy(pData, m_pData, sizeof(T) * nSize);
}

template <class T>
inline void Matrix2D<T>::GetElementBuf(T*& pData, int& nSize) const {
  nSize = row_size_ * col_size_;
  pData = m_pData;
}

template <class T>
inline void Matrix2D<T>::SetElement(T& data, int iRow, int jCol) {
  assert(NULLPTR != m_pData && iRow < row_size_ && jCol < col_size_);

  m_pData[col_size_ * iRow + jCol] = data;
}

template <class T>
inline const T& Matrix2D<T>::GetElement(int iRow, int jCol) const {
  assert(NULLPTR != m_pData && iRow < row_size_ && jCol < col_size_);

  return m_pData[col_size_ * iRow + jCol];
}

template <class T>
inline T& Matrix2D<T>::GetElement(int iRow, int jCol) {
  assert(NULLPTR != m_pData && iRow < row_size_ && jCol < col_size_);

  return m_pData[col_size_ * iRow + jCol];
}
}  // namespace core

#endif  // GEOMETRY_BASE_MATRIX2D_H