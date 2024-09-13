#ifndef __UNPACKING_H__
#define __UNPACKING_H__
#include<vector>
#include<string>
#include<tuple>
#include<utility>
#define GET_TYPE_NAME_v2(Type) get_type_name<Type>().str

struct EmptyClass{}; 

template<char...args>
struct static_string; 

template<typename Object>
constexpr auto get_type_name(); 

template<typename T>
struct HasCustomSerializeMethod;


template<typename T>
struct IsSetOrMap;       


template<typename T>
struct IsSerializableType; 


template<typename T>
struct IsIterableType;     


template<typename T>
struct IsTupleOrPair;


template<typename Object>
struct IsArrayType;	


template<typename Method>
struct IsClassMethodType;


struct HashFunc;

struct EqualKey;

template <typename T, size_t N>
inline constexpr size_t GetArrayLength(const T(&)[N]);


inline std::vector<std::string>unpacking_list(const std::string&serialized); 


template<typename Object,int index, typename T>
inline auto for_each_element(Object&object,T&&callback);                 


template<char...args>
struct static_string
{
	static constexpr const char str[]={args...};
	operator const char*()const{return static_string::str;}
};
template<typename Object>
constexpr auto get_type_name()
{
	constexpr std::string_view fully_name= __FUNCSIG__;
	constexpr std::size_t begin=[&]()
	{
		for(std::size_t i=0;i<fully_name.size();i++)
			if(fully_name[i]=='=')
				return i+2;
	}();
	constexpr std::size_t end=[&]()
	{
		for(std::size_t i=0;i<fully_name.size();i++)
			if(fully_name[i]==']')
				return i;
	}();
	constexpr auto type_name_view=fully_name.substr(begin,end-begin);
	constexpr auto indices=std::make_index_sequence<type_name_view.size()>();
	constexpr auto type_name=[&]<std::size_t...indices>(std::integer_sequence<std::size_t,indices...>)
	{
		constexpr auto str=static_string<type_name_view[indices]...,'\0'>();
		return str;
	}(indices);
	return type_name;
}
template<typename T>
struct HasCustomSerializeMethod
{
    template<typename U>                                               
    static auto check_dumps(int)->decltype(std::declval<U>().custom_dumps(),std::true_type());
    template<typename U>
	static std::false_type check_dumps(...);
    template<typename U>                                               
    static auto check_loads(int)->decltype(std::declval<U>().custom_loads(),std::true_type());
    template<typename U>
	static std::false_type check_loads(...);
    static constexpr int value1=std::is_same<decltype(check_dumps<T>(0)),std::true_type>::value;
    static constexpr int value2=std::is_same<decltype(check_loads<T>(0)),std::true_type>::value;
    static constexpr int value=value1&&value2;
};
template<typename T>
struct IsSetOrMap                                               
{                                                                 
    template<typename U>                                               
        static auto check(int)->decltype(std::declval<U>().insert(std::declval<decltype(*std::declval<U>().begin())>()),std::true_type());
    template<typename U>
        static std::false_type check(...);
    static constexpr int value = std::is_same<decltype(check<T>(0)),std::true_type>::value;
};
template<typename T>
struct IsSerializableType                                              
{                                                                      
    template<typename U>                                               
        static auto check(int)->decltype(std::declval<U>().get_config(),std::true_type());
    template<typename U>
        static std::false_type check(...);
    static constexpr int value = std::is_same<decltype(check<T>(0)),std::true_type>::value;
};
template<typename T>
struct IsIterableType                                                  
{
    template<typename U>
        static auto check(int)->decltype(std::declval<typename U::iterator>(),std::true_type());
    template<typename U>
        static std::false_type check(...);
    static constexpr int value = std::is_same<decltype(check<T>(0)),std::true_type>::value;
};
template<typename Object>
struct IsTupleOrPair
{
	template<typename T>
	static constexpr auto check(int)->decltype(std::get<0>(std::declval<T>()),std::true_type());
	template<typename T>
	static constexpr auto check(...)->std::false_type;
	static constexpr int value=std::is_same<decltype(check<Object>(0)),std::true_type>::value;
};
template<typename Object>
struct IsArrayType
{
	template<typename T>
	static constexpr auto check(int)->decltype(GetArrayLength(std::declval<T>()),std::true_type());
	template<typename T>
	static constexpr auto check(...)->std::false_type;
	static constexpr int value=std::is_same<decltype(check<Object>(0)),std::true_type>::value;
};

template<typename Method>
struct IsClassMethodType
{
	template<typename ClassType,typename ReturnType,typename...Args>
	static constexpr ReturnType match(ReturnType(ClassType::*method)(Args...args));
	template<typename T>
	static constexpr auto check(int)->decltype(match(std::declval<T>()),std::true_type());
	template<typename T>
	static constexpr std::false_type check(...);
	static constexpr int value=std::is_same<decltype(check<Method>(0)),std::true_type>::value;
};
struct HashFunc
{
	template<typename T, typename U>
	size_t operator()(const std::pair<T, U>& p) const {
		return std::hash<T>()(p.first) ^ std::hash<U>()(p.second);
	}
};


struct EqualKey {
	template<typename T, typename U>
	bool operator ()(const std::pair<T, U>& p1, const std::pair<T, U>& p2) const {
		return p1.first == p2.first && p1.second == p2.second;
	}
};
template <typename T, size_t N>
inline constexpr size_t GetArrayLength(const T(&)[N])
{
    return N;
}
template<typename Object,int index = 0, typename T >
inline auto for_each_element(Object&object,T&&callback)                              
{
	callback(std::get<index>(object),index);
	if constexpr(index+1<std::tuple_size<Object>::value)
		for_each_element<Object,index+1>(object,callback);
}
inline std::vector<std::string>unpacking_list(const std::string&serialized)              
{                                                                                        
	enum State{init,parse_fundamental,parse_string,parse_struct,parse_iterable,end_parse}state=init;
	std::vector<std::string>vec;
	std::string temp;
	int length=static_cast<int>(serialized.size());
	int nested_struct=0;             
	int nested_iterable=0;           
	for(int i=0;i<length;++i)
	{
		auto&it=serialized[i];
		if(i==0)
			continue;
		if(state==init)
		{
			if(it=='{')
			{
				state=parse_struct;
				nested_struct++;
				temp.push_back(it);
			}
			else if(it=='[')
			{
				state=parse_iterable;
				nested_iterable++;
				temp.push_back(it);
			}	
			else if(it!=','&&it!=' ')
			{
				state=parse_fundamental;
				temp.push_back(it);
			}
			else if(it=='\"')
			{
				state=parse_string;
				temp.push_back(it);
			}
		}
		else if(state==parse_string)
		{
			temp.push_back(it);
			if(it=='\"'&&serialized[i-1]!='\\') 
			{
				state=end_parse;
				--i;
			}
		}
		else if(state==parse_struct)
		{
			if(it=='}'||it=='{')
				nested_struct+=(it=='}'?-1:1);
			if(nested_struct==0) 
			{
				state=end_parse;
				--i;
				temp.push_back(it);
				continue;
			}
			temp.push_back(it);
		}
		else if(state==parse_iterable)
		{
			if(it==']'||it=='[')
				nested_iterable+=(it==']'?-1:1);
			if(nested_iterable==0)
			{
				state=end_parse;
				--i;
				temp.push_back(it);
				continue;
			}
			temp.push_back(it);
		}
		else if(state==parse_fundamental)
		{
			if(it==','||it==']')
			{
				state=end_parse;
				--i;
				continue;
			}
			temp.push_back(it);
		}
		else if(state==end_parse)
		{

			vec.push_back(temp);
			temp.clear();
			state=init;
		}
	}
	return vec;
}
#endif
