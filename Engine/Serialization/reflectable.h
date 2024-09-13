#ifndef __REFLECTABLE_H__
#define __REFLECTABLE_H__
#include"config.h"
#include<vector>
#include<functional>

#include <iostream>
using namespace std;

struct Reflectable
{
public:
	virtual ~Reflectable(){};
	template<typename T,typename ...Args>
	struct Regist;
	
	template<typename T>
	struct Regist<T>;
	
	template<typename T>
	struct Inherit;
	
	template<typename T>
	inline static Config get_config(const T*object);
	inline static std::vector<std::string_view>get_serializable_types();
	template<typename FieldType=void*>
	inline static auto get_field(void*object,std::string class_name,std::string field_name); 

	template<typename FieldType=void*,typename ClassType>
	inline static auto get_field(ClassType&object,std::string field_name); 

	inline static std::string get_field_type(std::string class_name,std::string field_name); 
	inline static std::size_t get_field_offset(std::string class_name,std::string field_name); 
		
	template<typename ClassType>
	inline static std::vector<std::string>get_field_names();
	template<typename ClassType>
	inline static std::vector<std::string>get_method_names();
	template<typename ReturnType,typename ObjectType,typename...Args>
	inline static auto get_method(ObjectType&object,const std::string&field_name,Args&&...args);

	template<typename FieldType,typename ClassType>
	inline static void set_field(ClassType&object,std::string field_name,const FieldType&data);
	
	template<typename T>
	inline static void set_field(void*object,std::string class_name,std::string field_name,const T&value);

	inline static void delete_instance(std::string class_name,void*object); 
	inline static void*get_instance(std::string class_name);
	
	using ClassName=std::string;
	using MethodName=std::unordered_map<std::string,void(EmptyClass::*)(void*)>;
	using FieldName=std::unordered_map<std::string,std::pair<std::string,std::size_t>>;
private:	
	static std::unordered_map<ClassName,FieldName>field;
	static std::unordered_map<ClassName,std::function<void*(void)>>default_constructors;
	static std::unordered_map<ClassName,std::function<void(void*)>>default_deconstructors;
	static std::unordered_map<std::string,MethodName>&method;
};
std::unordered_map<std::string,std::function<void*(void)>>Reflectable::default_constructors;
std::unordered_map<std::string,std::function<void(void*)>>Reflectable::default_deconstructors;
std::unordered_map<std::string,std::unordered_map<std::string,std::pair<std::string,std::size_t>>>Reflectable::field;
std::unordered_map<std::string,std::unordered_map<std::string,void(EmptyClass::*)(void*)>>&Reflectable::method=ConfigPair::from_classmethod_string;

std::vector<std::string_view>Reflectable::get_serializable_types()
{
	std::vector<std::string_view>types;
	for(auto&it:ConfigPair::from_config_string)
		types.push_back(it.first);
	return types;
}
template<typename ClassType>
std::vector<std::string>Reflectable::get_field_names()
{
	static std::vector<std::string>names=[&]()->std::vector<std::string> 
	{                                                                    
		std::vector<std::string>names;
		for(auto&it:field[GET_TYPE_NAME(ClassType)]) 
		{
			names.push_back(it.first);
		}
		return names;	
	}();		
	return names;
}
template<typename ClassType>
std::vector<std::string>Reflectable::get_method_names()
{
	static std::vector<std::string>names=[&]()->std::vector<std::string>
	{
		std::vector<std::string>names;
		for(auto&it:method[GET_TYPE_NAME(ClassType)])
		{
			names.push_back(it.first);
		}
		return names;	
	}();		
	return names;
}
template<typename T>
Config Reflectable::get_config(const T*object)
{
	std::string class_name=GET_TYPE_NAME(T);
	Config config(&field[class_name],object);
	config.update({{"class_name",class_name}});
	return config;
}
template<typename ReturnType,typename ObjectType,typename...Args>
auto Reflectable::get_method(ObjectType&object,const std::string&field_name,Args&&...args)
{
	try
	{
		auto func=Reflectable::method.at(GET_TYPE_NAME(ObjectType)).at(field_name);
		auto method=reinterpret_cast<ReturnType(ObjectType::*)(Args...)>(func);
		return (object.*method)(std::forward<Args>(args)...);
	}
	catch(std::exception&e)
	{
		throw NoSuchMethodException(GET_TYPE_NAME(ObjectType),field_name);
	}
}
template<typename FieldType,typename ClassType>
auto Reflectable::get_field(ClassType&object,std::string field_name)
{
	try
	{
		std::string class_name=GET_TYPE_NAME(ClassType);
		std::size_t offset=Reflectable::field.at(class_name).at(field_name).second;
		if constexpr(std::is_same<FieldType,void*>::value)
			return (void*)((std::size_t)(&object)+offset);
		else
			return (*(FieldType*)((std::size_t)(&object)+offset));
	}
	catch(std::exception&e)
	{
		throw NoSuchFieldException(GET_TYPE_NAME(ClassType),field_name);
	}
}
template<typename FieldType>
auto Reflectable::get_field(void*object,std::string class_name,std::string field_name)
{
	
	try
	{
		std::size_t offset=Reflectable::field.at(class_name).at(field_name).second;
		if constexpr(std::is_same<FieldType,void*>::value)
			return (void*)((std::size_t)(object)+offset);
		else
			return (*(FieldType*)((std::size_t)(object)+offset));
	}
	catch(std::exception)
	{
		throw NoSuchFieldException(class_name,field_name);
	}
}
std::string Reflectable::get_field_type(std::string class_name,std::string field_name)
{
	return Reflectable::field.at(class_name).at(field_name).first;
}
std::size_t Reflectable::get_field_offset(std::string class_name,std::string field_name)
{
	return Reflectable::field.at(class_name).at(field_name).second;
}
void*Reflectable::get_instance(std::string class_name)
{
	try
	{
		return Reflectable::default_constructors[class_name]();
	}
	catch(std::exception)
	{
		throw NoSuchClassException(class_name);
	}
}
void Reflectable::delete_instance(std::string class_name,void*object)
{
	default_deconstructors[class_name](object);
}
template<typename FieldType,typename ClassType>
void Reflectable::set_field(ClassType&object,std::string field_name,const FieldType&data)
{
	try
	{
		std::string class_name=GET_TYPE_NAME(ClassType);
		std::size_t offset=Reflectable::field.at(class_name).at(field_name).second;
		*(FieldType*)((std::size_t)(&object)+offset)=data;
	}
	catch(std::exception&e)
	{
		throw NoSuchFieldException(GET_TYPE_NAME(ClassType));
	}
}

template<typename T>
void Reflectable::set_field(void*object,std::string class_name,std::string field_name,const T&value)
{
	try
	{
		std::size_t offset=Reflectable::field.at(class_name).at(field_name).second;
		using field_type=typename std::remove_const<typename std::remove_reference<decltype(value)>::type>::type;
		*(field_type*)((std::size_t)(object)+offset)=value;
	}
	catch(std::exception)
	{
		throw NoSuchFieldException(class_name, field_name);
	}
}
template<typename T,typename ...Args>
struct Reflectable::Regist
{
	Regist()
	{
		T object;
		static Config config=object.get_config();                           
		Reflectable::default_constructors[GET_TYPE_NAME(T)]=[](void)->void* 
		{
			return (void*)(new T());
		};
		Reflectable::default_deconstructors[GET_TYPE_NAME(T)]=[](void*object)->void  
		{
			delete ((T*)object);
		};
		Regist<Args...>();
	}
};
template<typename T>
struct Reflectable::Regist<T>
{
	Regist()
	{
		static_assert(IsSerializableType<T>::value,"There are some objects that use reflection but haven't implement public method Config get_config()const");
		T object;
		static Config config=object.get_config();
		Reflectable::default_constructors[GET_TYPE_NAME(T)]=[](void)->void*          
		{
			return (void*)(new T());
		};
		Reflectable::default_deconstructors[GET_TYPE_NAME(T)]=[](void*object)->void  
		{
			delete ((T*)object);
		};
	}
};
template<typename Parent>
struct Reflectable::Inherit
{
	template<typename Object>
	static Config get_config(const Object*object)
	{
		Config parent_config=object->Parent::get_config();                              
		auto sub_config=Reflectable::field.find(GET_TYPE_NAME(Object));                 
		if(sub_config==Reflectable::field.end())                                        
			field[GET_TYPE_NAME(Object)]=Reflectable::field[GET_TYPE_NAME(Parent)];     
		return parent_config;	
	}
};
#endif
