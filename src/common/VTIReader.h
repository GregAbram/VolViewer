#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <vtkObjectFactory.h>
#include <vtkXMLGenericDataObjectReader.h>
#include <vtkXMLDataParser.h>
#include <vtkAbstractArray.h>
#include <vtkDataArray.h>

using namespace std;

class VTIReader : public vtkXMLGenericDataObjectReader
{
protected:
	~VTIReader() { this->CloseStream(); }
public:
	static VTIReader* New();

	void GetInfo();
	void ShowInfo();
	char *GetData(const char *name, const char*& type);

	size_t *GetCounts() { return counts; }
	float  *GetOrigin() { return origin; }
	float  *GetDeltas() { return deltas; }

	vector<const char *> *GetNames() { return &arrayNames; }


private:
	vector<const char *> arrayNames;
	vector<vtkXMLDataElement *> arrayElements;
	size_t counts[3];
	float  origin[3];
	float  deltas[3];
};

