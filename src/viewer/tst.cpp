#include <iostream>
#include <vtkXMLImageDataReader.h>
#include <vtkImageData.h>

int
main(int a, char **b)
{
	vtkXMLImageDataReader *r = vtkXMLImageDataReader::New();
	r->SetFileName("/Users/gda/Desktop/fake.vti");
	r->Update();
	vtkImageData *d = r->GetOutput();
	int *xyz = d->GetDimensions();
	std::cerr << xyz[0] << " " << xyz[1] << " " << xyz[2] << "\n";
}
  
