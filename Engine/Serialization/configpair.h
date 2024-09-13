#ifndef __CONFIGPAIR_H__
#define __CONFIGPAIR_H__
#include<string>
#include<typeinfo>
#include<sstream>
#include<vector>
#include<unordered_map>
#include<functional>
#include"utils.h"
#include"exception.h"
#define GET_TYPE_NAME(type) typeid(type).name()
struct ConfigPair
{
	template<typename T>
	ConfigPair(const std::string&name,const T&object);
	std::string key;    
	std::string value;  
	std::string type;   
	std::size_t address;
	bool is_field;      
	static std::unordered_map<std::string,std::function<void(void*,const std::string&)>>from_config_string; 
	static std::unordered_map<std::string,std::unordered_map<std::string,void(EmptyClass::*)(void*)>>from_classmethod_string;
	template<typename Object>
	static std::string get_config_string(const Object&object);
	template<typename ClassType,typename ReturnType,typename...Args>
	void get_classmethod_string(ReturnType(ClassType::*method)(Args...args));
};
std::unordered_map<std::string,std::function<void(void*,const std::string&)>>ConfigPair::from_config_string; 
std::unordered_map<std::string,std::unordered_map<std::string,void(EmptyClass::*)(void*)>>ConfigPair::from_classmethod_string;


template<typename T>
ConfigPair::ConfigPair(const std::string&name,const T&object):
	key(name),
	type(GET_TYPE_NAME(T)),
	address((std::size_t)&object),
	is_field(!IsClassMethodType<T>::value)
	{
		if constexpr(IsClassMethodType<T>::value)
			get_classmethod_string(object);
		else
			value=get_config_string<T>(object);
	}
template<typename ClassType,typename ReturnType,typename...Args>
void ConfigPair::get_classmethod_string(ReturnType(ClassType::*method)(Args...args))
{
		from_classmethod_string[GET_TYPE_NAME(ClassType)][this->key]=reinterpret_cast<void(EmptyClass::*)(void*)>(method);
}
template<typename Object>
std::string ConfigPair::get_config_string(const Object&field)
{
	std::ostringstream oss;
	if constexpr(std::is_fundamental<Object>::value&(!std::is_same<Object,char*>::value))
	{
		#ifdef __SERIALIZABLE_H__ 
		from_config_string[GET_TYPE_NAME(Object)]=[](void*field,const std::string&str)->void 
		{
			std::istringstream iss(str);
			Object value;
			iss>>value;
			*(Object*)field=value;
		};
		#endif
		oss<<field;
		return oss.str();
	}
	else if constexpr(std::is_same<Object,std::string>::value)  
	{
		#ifdef __SERIALIZABLE_H__
		from_config_string[GET_TYPE_NAME(Object)]=[](void*field,const std::string&str)->void 
		{
			Object value;
			char ch;
			std::istringstream iss(str);
			iss>>ch>>value;
			value.pop_back();
			*(Object*)field=value;
		};
		#endif
		oss<<"\""<<field<<"\"";
		return oss.str();
	}
	else if constexpr(std::is_same<Object,char*>::value)  
	{
		#ifdef __SERIALIZABLE_H__
		from_config_string[GET_TYPE_NAME(Object)]=[](void*field,const std::string&str)->void
		{
			std::string value;
			char ch;
			std::istringstream iss(str);
			iss>>ch>>value;
			value.pop_back();
			field=(void*)malloc(sizeof(char)*(value.size()+1)); 
			memcpy(field,value.c_str(),value.size()+1);
		};
		#endif
		oss<<"\""<<field<<"\"";
		return oss.str();
	}
	else if constexpr(std::is_pointer<Object>::value) 
	{
		#ifdef __SERIALIZABLE_H__
		from_config_string[GET_TYPE_NAME(Object)]=[](void*field,const std::string&str)->void
		{
			using type=typename std::remove_pointer<Object>::type;
			Object value=nullptr;
			if(str!="null")
			{
				value=new typename std::remove_pointer<Object>::type();
				if constexpr(!IsSerializableType<Object>::value)
					from_config_string[GET_TYPE_NAME(type)](&(*value),str);
				else
					Object::from_config_string[GET_TYPE_NAME(type)](&(*value),str);
			}
			*(Object*)field=value;
		};
		#endif
		if(field==nullptr)
			return"null";
		return get_config_string<typename std::remove_pointer<Object>::type>(*field);
	}
	else if constexpr(IsSerializableType<Object>::value) 
	{
		
		return field.get_config().serialized_to_string(false);
	}
	else if constexpr(IsTupleOrPair<Object>::value)
	{
		oss<<"[";
		for_each_element(field,[&](auto&it,int index){
			if(index+1<(int)std::tuple_size<Object>::value)
				oss<<get_config_string(it)<<",";
			else
				oss<<get_config_string(it);
		});
		oss<<"]";
		from_config_string[GET_TYPE_NAME(Object)]=[](void*field,const std::string&str)->void 
		{
			auto values=unpacking_list(str);                                     
			for_each_element(*(Object*)field,[&](auto&it,int index){
				std::string type_name=GET_TYPE_NAME(decltype(it));               
				from_config_string[type_name]((void*)(&it),values[index]);	     
			}); 
		};
		return oss.str();
	}
	else if constexpr(IsIterableType<Object>::value&(!std::is_same<Object,std::string>::value))
	{
		using element_type=typename std::remove_const<typename std::remove_reference<decltype(*field.begin())>::type>::type;
		std::ostringstream oss;
		oss<<"[";
		std::size_t index=0;
		for(auto&it:field)
		{
			index++;
			if(index==field.size()) 
				oss<<get_config_string<element_type>(it);
			else
				oss<<get_config_string<element_type>(it)<<",";
		}
		oss<<"]";
		#ifdef __SERIALIZABLE_H__
		from_config_string[GET_TYPE_NAME(Object)]=[](void*field,const std::string&str)->void
		{
			if constexpr(IsSetOrMap<Object>::value)
			{
				std::vector<std::string>values=unpacking_list(str);
				Object object;
				for(auto&it:values)
				{
					element_type element;
					from_config_string[GET_TYPE_NAME(element_type)](&element,it);
					object.insert(element);
				}
			}
			else
			{
				std::vector<std::string>values=unpacking_list(str);
				Object object(values.size());
				int index=0;
				for(auto&it:object)                                     
				{
					from_config_string[GET_TYPE_NAME(element_type)](&it,values[index]);
					++index;
				
				}
				*(Object*)field=object;	
			}
		};
		#endif	
		return oss.str();
	}
	else if constexpr(IsArrayType<Object>::value) 
	{
		using element_type=typename std::remove_const<typename std::remove_reference<decltype(field[0])>::type>::type;
		constexpr std::size_t length=sizeof(Object)/sizeof(element_type);
		oss<<"[";
		for(unsigned int i=0;i<length;i++)
		{
			oss<<get_config_string<element_type>(field[i]);
			if(i+1<length)
				oss<<",";
		}
		oss<<"]";
		from_config_string[GET_TYPE_NAME(Object)]=[](void*field,const std::string&str)->void
		{
			std::string type_name=GET_TYPE_NAME(element_type);
			auto values=unpacking_list(str);
			const std::size_t length=values.size();
			for(unsigned i=0;i<length;i++)
				from_config_string[type_name]((void*)((std::size_t)field+sizeof(element_type)*i),values[i]);
		};
		return oss.str();
	}
	else if constexpr(IsClassMethodType<Object>::value) 
	{
		return "<classmethod>";
	}
	throw NotSerializableException(GET_TYPE_NAME(Object));
	return "<not serializable object>";
}
#endif
