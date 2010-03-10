#ifndef _JDK_jdk_vecstring_H
#define _JDK_jdk_vecstring_H

#include <algorithm>
#include <vector>
#include <functional>
#include <iostream>

class jdk_vecstring  
{
private:
	std::vector<char> Rep;
	inline void terminate() const
	{
		const_cast<char&>(*Rep.rbegin()) = '\0';
	}

public:
	typedef std::vector<char>::size_type size_type;
	typedef std::vector<char>::iterator iterator;
	typedef std::vector<char>::const_iterator const_iterator;
	typedef std::vector<char>::reverse_iterator reverse_iterator;
	typedef std::vector<char>::const_reverse_iterator const_reverse_iterator;

	inline explicit jdk_vecstring(size_type n = 0) : Rep(n+1) {}
	inline jdk_vecstring(const jdk_vecstring& x) : Rep(x.Rep) {}
	inline jdk_vecstring(const char *Source)
	{
		Rep.resize(strlen(Source)+1);
		std::copy(Source,Source+Rep.size(),Rep.begin());
	}


	inline jdk_vecstring& operator=(const jdk_vecstring& x)
	{
		Rep=x.Rep;
		return *this;
	}

	inline jdk_vecstring& operator= (const char *Source)
	{
		Rep.resize(strlen(Source)+1);
		std::copy(Source,Source+Rep.size(),Rep.begin());
		return *this;
	}

	inline jdk_vecstring& operator+=(const jdk_vecstring& x)
	{
		size_type len = length();
		Rep.resize( len + x.length() + 1 );
		std::copy (x.Rep.begin(),x.Rep.end(),Rep.begin()+len);
		return *this;
	}

	inline jdk_vecstring& operator+=(const char*  x)
	{
		size_type len = length();
		int len2 = strlen(x);
		Rep.resize( len + len2 + 1 );
		std::copy (x,x+len2,Rep.begin()+len);
		return *this;
	}


	inline const char* c_str() const { terminate(); return &Rep[0]; }

	inline jdk_vecstring& operator+=(char x) 
	{ 
		*Rep.rbegin() = x; 
		Rep.push_back('\0');
	}

	inline size_type length() const { return Rep.size() - 1; }
	inline void reserve(size_type n) { Rep.reserve(n+1); }

	inline iterator begin() { return Rep.begin(); }
	inline iterator end() { return Rep.end()-1; }
	inline const_iterator begin() const { return Rep.begin(); }
	inline const_iterator end() const { return Rep.end()-1; }

	inline reverse_iterator rbegin() { return Rep.rbegin()+1; }
	inline reverse_iterator rend() { return Rep.rend(); }
	inline const_reverse_iterator rbegin() const { return Rep.rbegin()+1; }
	inline const_reverse_iterator rend() const { return Rep.rend(); }

};

#endif
