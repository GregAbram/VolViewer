#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <vtkObjectFactory.h>
#include <vtkXMLGenericDataObjectReader.h>
#include <vtkXMLDataParser.h>
#include <vtkAbstractArray.h>
#include <vtkDataArray.h>

#include "VTIReader.h"

using namespace std;

void
VTIReader::GetInfo()
{
	int extents[6];

	this->OpenStream();
	this->CreateXMLParser();
	this->XMLParser->SetStream(this->Stream);
	this->XMLParser->Parse();

	vtkXMLDataElement *root = XMLParser->GetRootElement();
	if (! root)
	{
		std::cerr << "no root elt\n";
		exit(1);
	}

	vtkXMLDataElement *imageData = root->LookupElementWithName("ImageData");
	if (! imageData)
	{
		cerr << "Not ImageData!\n";
		exit(1);
	}

	const char *we = imageData->GetAttribute("WholeExtent");
	if (we)
	{
		stringstream ss(we);
		ss >> extents[0] >> extents[1] >> extents[2] >> extents[3] >> extents[4] >> extents[5];
	}
	else
	{
		cerr << "No WholeExtents!\n";
		exit(1);
	}

	const char *org = imageData->GetAttribute("Origin");
	if (org)
	{
		stringstream ss(org);
		ss >> origin[0] >> origin[1] >> origin[2];
	}
	else
	{
		cerr << "No Origin\n";
		exit(1);
	}

	const char *sp = imageData->GetAttribute("Spacing");
	if (sp)
	{
		stringstream ss(sp);
		ss >> deltas[0] >> deltas[1] >> deltas[2];
	}
	else
	{
		cerr << "No Spacing!\n";
		exit(1);
	}

	origin[0] += extents[0] * deltas[0];
	origin[1] += extents[1] * deltas[1];
	origin[2] += extents[2] * deltas[2];

	counts[0] = (extents[1] - extents[0]) + 1;
	counts[1] = (extents[3] - extents[2]) + 1;
	counts[2] = (extents[5] - extents[4]) + 1;

	vtkXMLDataElement *pdata = root->LookupElementWithName("PointData");
	if (! pdata)
	{
		std::cerr << "no PointData elt\n";
		exit(1);
	}

	for (int i = 0; i < pdata->GetNumberOfNestedElements(); i++)
	{
		vtkXMLDataElement *e = pdata->GetNestedElement(i);
		const char *name = e->GetAttribute("Name");
		if (! name)
		{
			std::cerr << "child of PointData does not have Name attribute\n";
			exit(1);
		}
		const char *a = e->GetAttribute("NumberOfComponents");
		if (!a || atoi(a) == 1)
		{
			arrayElements.push_back(e);
			arrayNames.push_back(name);
		}
	}
}

void
VTIReader::ShowInfo()
{
	cout << "origin: " << origin[0] << " " << origin[1] << " " << origin[2] << "\n";
	cout << "counts: " << counts[0] << " " << counts[1] << " " << counts[2] << "\n";
	cout << "deltas: " << deltas[0] << " " << deltas[1] << " " << deltas[2] << "\n";
	for (vector<vtkXMLDataElement *>::iterator i = arrayElements.begin(); i != arrayElements.end(); i++)
		cout << (*i)->GetAttribute("Name") << "\n";

}

char *
VTIReader::GetData(const char *name, const char*& type)
{
	vtkXMLDataElement *elt = NULL;
	for (vector<vtkXMLDataElement *>::iterator i = arrayElements.begin(); !elt && i != arrayElements.end(); i++)
		if (!strcmp((*i)->GetAttribute("Name"), name))
			elt = *i;

	if (!elt)
	{
		std::cerr << "No array named " << name << "\n";
		return NULL;
	}

	size_t numTuples = counts[0]*counts[1]*counts[2];

	vtkAbstractArray *ar = this->CreateArray(elt);
	ar->SetNumberOfTuples(numTuples);

	void *buffer = NULL;

	if (ar->IsA("vtkFloatArray") || ar->IsA("vtkUnsignedCharArray"))
	{
		cerr << "trying to allocate " << (ar->GetElementComponentSize() * numTuples) << " bytes\n";

		buffer = (void *)malloc(ar->GetElementComponentSize() * numTuples);
		

		// Read directly into buffer

		type = ar->IsA("vtkFloatArray") ? "float" : "uchar";

		vtkDataArray::SafeDownCast(ar)->SetVoidArray(buffer, ar->GetElementComponentSize() * numTuples, 1);
	
		if (! this->ReadArrayValues(elt, 0, ar, 0, numTuples))
		{
			std::cerr << "ReadArrayForValues error\n";
			free(buffer);
			buffer = NULL;
		}
	}
	else if (ar->IsA("vtkDoubleArray"))
	{
		type = "float";

		if (! this->ReadArrayValues(elt, 0, ar, 0, numTuples))
		{
			std::cerr << "ReadArrayForValues error\n";
			return NULL;
		}

		buffer = (void *)malloc(sizeof(float) * numTuples);

		double *src = (double *)ar->GetVoidPointer(0);
		float *dst = (float *)buffer;
		for (int i = 0; i < numTuples; i++)
			*dst++ = *src++;
	}
	else
		std::cerr << "unsupported data class: " << ar->GetClassName() << "\n";

	ar->Delete();
	return (char *)buffer;
}

vtkStandardNewMacro(VTIReader);

#if 0
int
main(int argc, char **argv)
{
VTIReader *reader = VTIReader::New();
reader->SetFileName(argv[1]);
reader->GetInfo();
// reader->ShowInfo();

const char *type;

for (vector<const char *>::iterator it = reader->GetNames()->begin(); it != reader->GetNames()->end(); it++)
	cerr << (*it) << "\n";

string varname;

cout << "? ";
cin >> varname;

char *buffer = reader->GetData(varname.c_str(), type);
if (buffer)
{
	cerr << "type = " << type << "\n";

	size_t *counts = reader->GetCounts();
	size_t sz = counts[0]*counts[1]*counts[2]*4;
	cerr << sz << " being written\n";
	
	ofstream out("foo.raw", ios::out | ios::binary);
	out.write(buffer, sz);
	out.close();

	free(buffer);
}

reader->Delete();
}
#endif
