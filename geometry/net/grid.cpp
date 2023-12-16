#include "geometry/net/grid.h"

namespace core {
Grid::Grid() {
  nodes_ = NULL;
  row_size_ = 0;
  col_size_ = 0;
}

Grid::Grid(int row, int col) {
  nodes_ = NULL;
  row_size_ = 0;
  col_size_ = 0;

  SetSize(row, col);

  coord_dimension_ = 2;
}

Grid::~Grid() { Empty(); }

// Geometry
int Grid::GetDimension() const { return 2; }

const char *Grid::GetGeometryName() const { return "GRID"; }

GeometryType Grid::GetGeometryType() const { return GTGrid; }

Geometry *Grid::Clone() const {
  Grid *poNewGrid = new Grid(row_size_, col_size_);

  Matrix2D<RawPoint> *pNodeBuf = poNewGrid->GetGridNodes();
  for (int i = 0; i < row_size_; i++)
    for (int j = 0; j < col_size_; j++) {
      RawPoint rawPt = nodes_->GetElement(i, j);
      pNodeBuf->SetElement(rawPt, i, j);
    }

  poNewGrid->SetCoordinateDimension(coord_dimension_);

  return poNewGrid;
}

void Grid::Empty() {
  SMT_SAFE_DELETE(nodes_);
  row_size_ = col_size_ = 0.0;

  coord_dimension_ = 0;
}

void Grid::GetEnvelope(Envelope *envelope) const {
  for (int i = 0; i < row_size_; i++) {
    for (int j = 0; j < col_size_; j++) {
      envelope->Merge(nodes_->GetElement(i, j).x,
                      nodes_->GetElement(i, j).y);
    }
  }
}

bool Grid::IsEmpty() const { return coord_dimension_ == 0; }

// Non standard
void Grid::SetCoordinateDimension(int nDimension) {
  coord_dimension_ = nDimension;
}

// SpatialRelation
bool Grid::Equals(const Geometry *geometry) const {
  Grid *poOGrid = (Grid *)geometry;

  if (poOGrid == this) return true;

  if (geometry->GetGeometryType() != GetGeometryType()) return false;

  // we should eventually test the SRS.

  int row, col;
  poOGrid->GetSize(row, col);

  if (row != row_size_ || col != col_size_) return false;

  Matrix2D<RawPoint> *pNodeBuf = poOGrid->GetGridNodes();
  for (int i = 0; i < row_size_; i++) {
    for (int j = 0; j < col_size_; j++) {
      RawPoint rawPt = nodes_->GetElement(i, j);
      RawPoint rawPt1 = pNodeBuf->GetElement(i, j);

      if (rawPt1.x != rawPt.x || rawPt1.y != rawPt.y) return false;
    }
  }

  return true;
}

void Grid::SetSize(int row, int col) {
  //先释放原有空间
  Empty();

  //重置参数
  row_size_ = row;
  col_size_ = col;

  //申请空间
  nodes_ = new Matrix2D<RawPoint>(row_size_, col_size_);
}
//////////////////////////////////////////////////////////////////////////
//重置网格大小
void Grid::ReSize(int row, int col) {
  if (row == row_size_ && col == col_size_) return;

  SetSize(row, col);
}

void Grid::GetSize(int &row, int &col) const {
  row = row_size_;
  col = col_size_;
}
}  // namespace core