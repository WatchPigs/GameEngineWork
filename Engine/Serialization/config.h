#ifndef __CONFIG_H__
#define __CONFIG_H__
#include"configpair.h"
#include<unordered_map>
class Config
{            
public:
	using Field=std::unordered_map<std::string,std::pair<std::string,std::size_t>>;
	Config():field_info(nullptr){}
	template<typename T>
	Config(Field*field_info,T*object);
	
	std::string serialized_to_string(bool first_nested_layer=true)const;      
	std::string&operator[](const std::string&key)const; 
	std::string&operator[](std::string&key);  
	void update(const std::initializer_list<ConfigPair>&pairs);
	
	auto begin();
	auto end();
private:
	mutable std::unordered_map<std::string,std::string>config;
	Field*field_info;
	std::size_t class_header_address;
	std::size_t class_size;
};
std::ostream operator<<(std::ostream&os,Config&config);
template<typename T>
Config::Config(Config::Field*field_info,T*object): 
	field_info(field_info),                                                                           
	class_header_address((std::size_t)(object)),                                                        
	class_size(sizeof(T)){}
void Config::update(const std::initializer_list<ConfigPair>& pairs)
{
	for(auto&it:pairs)
	{
		if(it.is_field)
		{
			config[it.key]=it.value;
			if(field_info!=nullptr)                                                 
			{
				(*field_info)[it.key].first=it.type;                                
				(*field_info)[it.key].second=it.address-this->class_header_address; 
			}
		}
	}
}
std::string&Config::operator[](const std::string&key)const 
{
	return config[key];
}
std::string&Config::operator[](std::string&key) 
{
	return config[key];
}
std::string Config::serialized_to_string(bool first_nested_layer)const 
{
	std::ostringstream oss;
	char end=first_nested_layer?'\n':' ';
	oss<<"{"<<end;
	for(auto it=config.begin(),next=++config.begin();it!=config.end();++it,next!=config.end()?++next:next)
	{
		if(it!=config.end()&&next==config.end()) 
			oss<<"\""<<(*it).first<<"\":"<<(*it).second<<end;
		else
			oss << "\"" << (*it).first << "\":" << (*it).second << end;;
	}
	oss<<"}"<<end;
	return oss.str();
}
auto Config::begin()
{
	return config.begin();
}
auto Config::end()
{
	return config.end();
}
#endif
