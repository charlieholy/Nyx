#include "ch_json.h"

std::string GetJson2Str(const Json::Value& jdata){
	Json::FastWriter writer;
	return writer.write(jdata);
}

bool GetJsonFromStr(const std::string& f,Json::Value& t){
	Json::Reader reader;
	if(!reader.parse(f,t))
	{
		return false;
	}
	return true;
}

bool GetJsonBoolean(const Json::Value& value,std::string key)
{
	if (value.type() == Json::objectValue)
	{
		if (value[key].type() == Json::booleanValue)
		{
			return value[key].asBool();
		}
	}
	return false;
}

int GetJsonInt(const Json::Value& value)
{
	Json::ValueType type = value.type();
	if(type == Json::intValue)
	{
		return value.asInt();
	}
	else
	{
		return -1;
	}
}

int64_t GetJsonInt64(const Json::Value & value)
{
	Json::ValueType type = value.type();
	if(type == Json::uintValue || type == Json::intValue)
	{
		return value.asInt64();
	}
	else
	{
		return -1;
	}
}

int GetJsonInt(const Json::Value& value,std::string key)
{
	if( value.type() == Json::objectValue)
	{
		if(value[key].type() == Json::intValue)
		{
			return value[key].asInt();
		}
	}
	return -1;
}

int64_t GetJsonInt64(const Json::Value & value, std::string key)
{
	if (value.type() == Json::objectValue)
	{
		if (value[key].type() == Json::uintValue || value[key].type() == Json::intValue)
		{
			return value[key].asInt64();
		}
	}
	return -1;
}

double GetJsonDouble(const Json::Value& value)
{
	Json::ValueType type = value.type();
	if (type == Json::realValue)
	{
		return value.asDouble();
	}
	else if (type == Json::intValue)
	{
		return value.asInt();
	}
	else
	{
		return 0.0;
	}
}

double GetJsonDouble(const Json::Value& value, std::string key)
{
	if (value.type() == Json::objectValue)
	{
		if (value[key].type() == Json::realValue)
		{
			return value[key].asDouble();
		}
		else if (value[key].type() == Json::intValue)
		{
			return value[key].asInt();
		}
	}
	return 0.0;
}

std::string GetJsonStr(const Json::Value& value)
{
	Json::ValueType type = value.type();
	if(type == Json::stringValue)
	{
		return value.asString();
	}
	else
	{
		return "";
	}
}

std::string GetJsonStr(const Json::Value& value,std::string key)
{
	std::string r = "";
	if( value.type() == Json::objectValue)
	{
		if(value[key].type() == Json::stringValue)
		{
			r = value[key].asString();
		}
	}
	return r;
}

const Json::Value& GetJsonArr(const Json::Value& value)
{
	if(value.type() == Json::arrayValue)
	{
		return value;
	}
	return JSON_NULL;
}

const Json::Value& GetJsonArr(const Json::Value& value,std::string key)
{
	if(value.type() == Json::objectValue)
	{
		if(value[key].type() == Json::arrayValue)
		{
			return value[key];
		}
	}
	return JSON_NULL;
}

const Json::Value& GetJsonArrPos(const Json::Value& value,int size)
{
	if(value.type() == Json::arrayValue)
	{
		return value[size];
	}
	return JSON_NULL;
}


const Json::Value& GetJsonVal(const Json::Value& value)
{
	if(value.type() == Json::objectValue)
	{
		return value;
	}
	return JSON_NULL;
}

const Json::Value& GetJsonVal(const Json::Value& value,std::string key)
{
	if(value.type() == Json::objectValue)
	{
		if(value[key].type() == Json::objectValue)
		{
			return value[key];
		}
	}
	return JSON_NULL;
}