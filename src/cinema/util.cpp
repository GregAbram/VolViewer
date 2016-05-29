#include "../common/common.h"
#include "util.h"

using namespace std;
using namespace rapidjson;

void
AddIntRangeArg(Document& doc, string name, vector<int> values)
{
  Value::MemberIterator m = doc["arguments"].FindMember(name.c_str());
  if (m != doc["arguments"].MemberEnd())
    doc["arguments"].RemoveMember(name.c_str());

	Value arg(kObjectType);
	Value ov(kObjectType), av(kArrayType);

	ov.SetInt(values[0]);
	arg.AddMember("default", ov, doc.GetAllocator());

	av.Clear();
	for (int i = 0; i < values.size(); i++)
	{
		Value v(values[i]);
		av.PushBack(v, doc.GetAllocator());
	}
	arg.AddMember("values", av, doc.GetAllocator());

	ov.SetString("range", doc.GetAllocator());
	arg.AddMember("type", ov, doc.GetAllocator());
	
	ov.SetString(name.c_str(), doc.GetAllocator());
	arg.AddMember("label", ov, doc.GetAllocator());

	ov.SetString(name.c_str(), doc.GetAllocator());
	doc["arguments"].AddMember(ov, arg, doc.GetAllocator());
}

void
AddBoolRangeArg(Document& doc, string name, vector<bool> values)
{
  Value::MemberIterator m = doc["arguments"].FindMember(name.c_str());
  if (m != doc["arguments"].MemberEnd())
    doc["arguments"].RemoveMember(name.c_str());

	Value arg(kObjectType);
	Value ov(kObjectType), av(kArrayType);

	ov.SetInt(values[0]);
	arg.AddMember("default", ov, doc.GetAllocator());

	av.Clear();
	for (int i = 0; i < values.size(); i++)
	{
		Value v(values[i] ? 1 : 0);
		av.PushBack(v, doc.GetAllocator());
	}
	arg.AddMember("values", av, doc.GetAllocator());

	ov.SetString("range", doc.GetAllocator());
	arg.AddMember("type", ov, doc.GetAllocator());
	
	ov.SetString(name.c_str(), doc.GetAllocator());
	arg.AddMember("label", ov, doc.GetAllocator());

	ov.SetString(name.c_str(), doc.GetAllocator());
	doc["arguments"].AddMember(ov, arg, doc.GetAllocator());
}

void
SetIntAttr(Document& doc, string name, int val)
{
  Value::MemberIterator m = doc["descriptor"].FindMember(name.c_str());
  if (m != doc["descriptor"].MemberEnd())
    doc["descriptor"].RemoveMember(name.c_str());

	Value v(kObjectType);
	v.SetInt(val);
	Value n; n.SetString(name.c_str(), doc.GetAllocator());
	doc["descriptor"].AddMember(n, v, doc.GetAllocator());
}

void
SetDoubleAttr(Document& doc, string name, double val)
{
  Value::MemberIterator m = doc["descriptor"].FindMember(name.c_str());
  if (m != doc["descriptor"].MemberEnd())
    doc["descriptor"].RemoveMember(name.c_str());

	Value v(kObjectType);
	v.SetDouble(val);
	Value n; n.SetString(name.c_str(), doc.GetAllocator());
	doc["descriptor"].AddMember(n, v, doc.GetAllocator());
}

