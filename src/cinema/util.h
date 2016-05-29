#pragma once
#include <vector>
using namespace std;

void AddIntRangeArg(Document& doc, string name, vector<int> values);
void AddBoolRangeArg(Document& doc, string name, vector<bool> values);
void SetIntAttr(Document& doc, string name, int val);
void SetDoubleAttr(Document& doc, string name, double val);

