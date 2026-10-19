// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

// Writes a tiny regular grid. No Qt widgets and no VTK render window.

#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkXMLImageDataWriter.h>

int main(int argc, char** argv) {
  const char* path = argc > 1 ? argv[1] : "grid.vti";

  vtkNew<vtkImageData> image;
  image->SetDimensions(8, 8, 8);
  image->SetSpacing(1.0, 1.0, 1.0);
  image->AllocateScalars(VTK_FLOAT, 1);

  vtkFloatArray* scalars =
      vtkFloatArray::SafeDownCast(image->GetPointData()->GetScalars());
  const vtkIdType n = scalars->GetNumberOfTuples();
  for (vtkIdType i = 0; i < n; ++i) {
    scalars->SetValue(i, static_cast<float>(i));
  }

  vtkNew<vtkXMLImageDataWriter> writer;
  writer->SetFileName(path);
  writer->SetInputData(image);
  return writer->Write() ? 0 : 1;
}
