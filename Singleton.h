#pragma once

template<typename T>
class Singleton
{
public:
	static T* s_instance;
	static T* Instance()
	{
		if (s_instance == NULL)
			s_instance = new T;
		return s_instance;
	}
};

template<typename T>
T *Singleton<T>::s_instance = NULL;