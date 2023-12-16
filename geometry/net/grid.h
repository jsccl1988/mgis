#ifndef GEOMETRY_NET_GRID_H
#define GEOMETRY_NET_GRID_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT Grid : public Geometry {
 public:
  Grid();
  Grid(int row, int col);
  virtual ~Grid();

  // Geometry
  virtual void SetCoordinateDimension(int nDimension);
  virtual int GetDimension() const;
  virtual const char *GetGeometryName() const;
  virtual GeometryType GetGeometryType() const;

  virtual Geometry *Clone() const;
  virtual void Empty();
  virtual void GetEnvelope(Envelope *envelope) const;
  virtual bool IsEmpty() const;

  // grid
  void SetSize(int row, int col);
  void ReSize(int row, int col);

  void GetSize(int &row, int &col) const;

  Matrix2D<RawPoint> *GetGridNodes() { return nodes_; }
  const Matrix2D<RawPoint> *GetGridNodes() const { return nodes_; }

  // SpatialRelation
  virtual bool Equals(const Geometry *) const;

 protected:
  Matrix2D<RawPoint> *nodes_;
  int row_size_;
  int col_size_;
};
}  // namespace core
#endif  // GEOMETRY_NET_GRID_H