#ifndef __SERIALIZABLE_H__
#define __SERIALIZABLE_H__
#include<iostream>
#include<functional>
#include<algorithm>
#include"reflectable.h"
#include"exception.h"
class Serializable:public Reflectable
{
public: 
	virtual ~Serializable(){};
	template<typename T> 
	struct Inherit;      
	template<typename T,typename ...Args>
	struct MultiInherit;
	template<typename T,typename ...Args>
	struct Regist;                      
	template<typename T>                        
	struct Regist<T>;
	template<typename Object>
	inline static Config get_config(const Object*object);        
	template<typename Object>
	inline static std::string dumps(const Object&object);        
	template<typename Type>
	inline static std::string dumps(const std::initializer_list<Type>&object);
	template<typename Object=void*>
	inline static auto loads(const std::string&json);            
	inline static void*loads(const std::string&json,const std::string&class_name)
	{
		void*object=nullptr;
		try
		{
			object=Reflectable::get_instance(class_name);
			ConfigPair::from_config_string[class_name](object,json);
			return object;
		}
		catch(JsonDecodeDelimiterException&e)
		{
			throw e;
		}
		catch(JsonDecodeUnknowException&e)
		{
			throw e;
		}
		catch(std::exception)                                                                   
		{
			throw JsonDecodeUnknowException(__LINE__,__FILE__);
		}	
	}
	template<typename Object>                             
	inline static void from_config(Object*object,Config&config); 
	
	inline static Config decode(const std::string&json);   
	inline static Config decode_view(const std::string&json);   
};
template<typename Object>
Config Serializable::get_config(const Object*object)
{
	return Reflectable::get_config(object);	
}
template<typename Object>
void Serializable::from_config(Object*object,Config&config)
{
	std::string class_name=GET_TYPE_NAME(Object);
	for(auto&it:config)
	{
		if(it.first!="class_name")
		{
			auto&field_name=it.first;
			auto&value=it.second;
			std::string type=Reflectable::get_field_type(class_name,field_name);     
			void*field=Reflectable::get_field(object,class_name,field_name);         
			if(type[type.size()-1]=='*'&&value=="null")                              
				*(void**)field=nullptr;
			else 
				ConfigPair::from_config_string[type](field,value);                   
		}
	}
}
template<typename Type>
std::string Serializable::dumps(const std::initializer_list<Type>&object)
{
	return ConfigPair::get_config_string<std::vector<Type>>(std::vector<Type>(object));
}
template<typename Object>
std::string Serializable::dumps(const Object&object)
{
	if constexpr(IsSerializableType<Object>::value)        
		return object.get_config().serialized_to_string(); 
	else
		return ConfigPair::get_config_string(object); 
}                                                     
Config Serializable::decode(const std::string&json)                    
{
	enum State{init,parse_value,parse_struct,parse_fundamental,parse_iterable,parse_string,end_parse}state=init;
	std::string key,value;
	int nested_struct_layer=0,nested_iterable_layer=0;
	std::string serialized=[&]()->std::string 
	{
		bool is_in_string=false;
		std::string strip;
		for(auto&it:json)
		{
			if(it=='\"')
				is_in_string^=1;
			if(!is_in_string&&(it=='\n'||it=='\t'||it==' '))
				continue;
			strip.push_back(it);
		}
		return strip;
	}();
	int length=static_cast<int>(serialized.size());
	Config config;
	if(serialized[0]!='{')
		throw JsonDecodeDelimiterException('{');
	if(serialized[length-1]!='}')
		throw JsonDecodeDelimiterException('}');
	for(int i=0;i<length;++i)
	{
		auto&it=serialized[i];
		if(state==init)                                        
		{
			if(it==':')
				state=parse_value;                             
			else if(it!='\"'&&it!='{'&&it!=','&&it!=' ')       
				key.push_back(it);
		}
		else if(state==parse_value)                            
		{
			if(it=='{')                                        
			{
				value.push_back(it);
				nested_struct_layer++;                         
				state=parse_struct;
			}
			else if(it=='[')                                   
			{
				value.push_back(it);
				nested_iterable_layer++;                       
				state=parse_iterable;
			}
			else if(it=='\"')
			{
				value.push_back(it);
				state=parse_string;
			}
			else if(it!=' ')                                   
			{
				value.push_back(it);
				state=parse_fundamental;
			}
		}
		else if(state==parse_string)
		{
			value.push_back(it);
			if(it=='\"'&&serialized[i-1]!='\\')                
			{
				state=end_parse;
				--i;
			}
		}
		else if(state==parse_fundamental) 
		{
			if(it==','||it=='}'||it=='\"')                     
			{
				if(it=='\"')
					value.push_back(it);                       
				state=end_parse;
				--i;
				continue;
			}
			value.push_back(it);
		}
		else if(state==parse_iterable)                          
		{
			if(it==']'||it=='[')
			{
				nested_iterable_layer+=(it==']'?-1:1);
				value.push_back(it);
				if(nested_iterable_layer==0)
				{
					state=end_parse;
					--i;
				}
				continue;
			}
			value.push_back(it);
		}
		else if(state==parse_struct)                             
		{
			if(it=='}'||it=='{')
			{
				nested_struct_layer+=(it=='}'?-1:1);
				value.push_back(it);
				if(nested_struct_layer==0)
				{
					state=end_parse;
					--i;
				}
				continue;
			}
			value.push_back(it);
		}
		else if(state==end_parse)                                
		{

			state=init;
			config[key]=value;
			key.clear();
			value.clear();
		}
	}
	if(!(state==end_parse&&nested_iterable_layer==0&&nested_struct_layer==0)) 
	{ 
		if(nested_iterable_layer>0)
			throw JsonDecodeDelimiterException(']');
		else if(nested_iterable_layer<0)
			throw JsonDecodeDelimiterException('[');
		if(nested_struct_layer>0)
			throw JsonDecodeDelimiterException('}');
		else if(nested_struct_layer<0)
			throw JsonDecodeDelimiterException('{');
	}
	return config;
}
template<typename Object>
auto Serializable::loads(const std::string&json)
{
	std::string class_name=GET_TYPE_NAME(Object);
	if constexpr(IsSerializableType<Object>::value) 
	{
		try
		{																																																																																																																																													
			Object object;
			ConfigPair::from_config_string[class_name](&object,json);                              
			return object;
		}
		catch(JsonDecodeDelimiterException&e)
		{
			throw e;
		}
		catch(JsonDecodeUnknowException&e)
		{
			throw e;
		}
		catch(std::exception)                                                                   
		{
			throw JsonDecodeUnknowException(__LINE__,__FILE__);
		}	
	}
	else 
	{
		try
		{
			Object object;
			ConfigPair::from_config_string[class_name](&object,json);
			return object;
		}
		catch(JsonDecodeDelimiterException&e)
		{
			throw e;
		}
		catch(JsonDecodeUnknowException&e)
		{
			throw e;
		}
		catch(std::exception&e)
		{
			throw JsonDecodeUnknowException(__LINE__,__FILE__);
		}
	}
}
template<typename T,typename ...Args>
struct Serializable::Regist
{
	Regist()
	{
		ConfigPair::from_config_string[GET_TYPE_NAME(T)]=[](void*object,const std::string&value)->void    
		{
			Config config=Serializable::decode(value);
			Serializable::from_config((T*)object,config);
		};
		Reflectable::Regist<T>();
		Serializable::Regist<Args...>();
	}
};
template<typename T>
struct Serializable::Regist<T>
{
	Regist()
	{
		ConfigPair::from_config_string[GET_TYPE_NAME(T)]=[](void*object,const std::string&value)->void   
		{
			Config config=Serializable::decode(value);
			Serializable::from_config((T*)object,config);
		};
		Reflectable::Regist<T>();
	}
};
template<typename Parent>
struct Serializable::Inherit
{
	template<typename Object>
	static Config get_config(const Object*object)
	{
		Config parent_config=Reflectable::Inherit<Parent>::get_config(object);
		Config config=Serializable::get_config(object);                                 
		for(auto&it:parent_config)
			if(it.first!="class_name")
				config[it.first]=it.second;
		return config;
	}
};
#endif
