#ifndef __SERIALIZABLE_EXCEPTION_H__
#define __SERIALIZABLE_EXCEPTION_H__
#include<string>
#include<sstream>
struct NoSuchClassException:public std::exception
{ 
public:
    explicit NoSuchClassException(const std::string&type_name);
    virtual ~NoSuchClassException()throw();
    virtual const char*what()const throw();
protected: 
    std::string message;
};

struct NoSuchFieldException:public std::exception
{ 
public:
    explicit NoSuchFieldException(const std::string&type_name,const std::string&field_name);
    virtual ~NoSuchFieldException()throw();
    virtual const char*what()const throw();
protected: 
    std::string message;
};
struct NoSuchMethodException:public std::exception
{ 
public:
    explicit NoSuchMethodException(const std::string&type_name,const std::string&field_name);
    virtual ~NoSuchMethodException()throw();
    virtual const char*what()const throw();
protected: 
    std::string message;
};

class NotSerializableException:public std::exception 
{ 
public:
    explicit NotSerializableException(const std::string&type_name);
    virtual ~NotSerializableException()throw();
    virtual const char*what()const throw();
protected: 
    std::string message;
}; 
class JsonDecodeException:public std::exception
{
public:
	explicit JsonDecodeException();
	virtual ~JsonDecodeException() throw();
	virtual const char*what()const throw()=0;
protected:
	std::string message;
};
class JsonDecodeDelimiterException:public JsonDecodeException
{
public:
	explicit JsonDecodeDelimiterException(const char&ch);
	virtual ~JsonDecodeDelimiterException();
	virtual const char*what()const throw();
};
class JsonDecodeUnknowException:public JsonDecodeException
{
public:
	explicit JsonDecodeUnknowException(const int line,const char*file);
	virtual ~JsonDecodeUnknowException();
	virtual const char*what()const throw();
};
/********************************************************************************************/
NoSuchClassException::NoSuchClassException(const std::string&type_name)
{
	std::ostringstream oss;
	oss<<"Class '"<<type_name<<"' dose not exist or not reflectable.";
	this->message=oss.str();
}
NoSuchClassException::~NoSuchClassException()
{
	
}
const char*NoSuchClassException::what()const throw()
{
	return this->message.c_str();
}
/********************************************************************************************/
NoSuchFieldException::NoSuchFieldException(const std::string&type_name,const std::string&field_name)
{
	std::ostringstream oss;
	oss<<"Object of type <"<<type_name<<"> dose not have field named '"<<field_name<<"' .";
	this->message=oss.str();
}
NoSuchFieldException::~NoSuchFieldException()
{
	
}
const char*NoSuchFieldException::what()const throw()
{
	return this->message.c_str();
}
/********************************************************************************************/
NoSuchMethodException::NoSuchMethodException(const std::string&type_name,const std::string&field_name)
{
	std::ostringstream oss;
	oss<<"Object of type <"<<type_name<<"> dose not have method named '"<<field_name<<"' .";
	this->message=oss.str();
}
NoSuchMethodException::~NoSuchMethodException()
{
	
}
const char*NoSuchMethodException::what()const throw()
{
	return this->message.c_str();
}
/********************************************************************************************/
NotSerializableException::NotSerializableException(const std::string&type_name)
{
    std::ostringstream oss;
    oss<<"NotSerializableException:";
    oss<<"Object of type < "<<type_name<<" > is not JSON serializable.";
    message=oss.str();
}
NotSerializableException::~NotSerializableException()throw()
{
	
}
const char*NotSerializableException::what()const throw()
{ 
	return message.c_str();
}
/********************************************************************************************/
JsonDecodeException::JsonDecodeException():
	message("JsonDecodeException:")
	{}
JsonDecodeException::~JsonDecodeException()throw(){}
/********************************************************************************************/
JsonDecodeDelimiterException::~JsonDecodeDelimiterException()
{
	
}
const char*JsonDecodeDelimiterException::what()const throw()
{
	return this->message.c_str();
}

JsonDecodeDelimiterException::JsonDecodeDelimiterException(const char&ch):
	JsonDecodeException()
	{
		std::ostringstream oss;
		oss<<"Expecting '"<<ch<<"' delimiter in decoding json data.";
		message+=oss.str();
	}
/********************************************************************************************/
JsonDecodeUnknowException::~JsonDecodeUnknowException()
{
	
}
const char*JsonDecodeUnknowException::what()const throw()
{
	return this->message.c_str();
}

JsonDecodeUnknowException::JsonDecodeUnknowException(const int line,const char*file):
	JsonDecodeException()
	{
		std::ostringstream oss;
		oss<<"An unknow error occurred in decoding json data.(file:"<<file<<",line:"<<line;
		message+=oss.str();
	}
#endif
	
