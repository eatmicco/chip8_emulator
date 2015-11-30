#ifndef BASE_SINGLETON_H
#define BASE_SINGLETON_H

namespace eatmicco
{
	template<typename T>
	class BaseSingleton {
	private:
		BaseSingleton(const BaseSingleton<T> &other);
		BaseSingleton &operator=(const BaseSingleton<T> &other);

	protected:
		static T *_instance;
		BaseSingleton()
		{
			_instance = static_cast<T*>(this);
		}

		~BaseSingleton() {}

	public:
		static T *get_instance();
		static void destroy_instance();
	};

	template<typename T>
	T* BaseSingleton<T>::_instance = nullptr;

	template<typename T>
	T *BaseSingleton<T>::get_instance()
	{
		if (!_instance)
		{
			BaseSingleton<T>::_instance = new T();
		}
		return _instance;
	}

	template<typename T>
	void BaseSingleton<T>::destroy_instance()
	{
		delete BaseSingleton<T>::_instance;
		BaseSingleton<T>::_instance = nullptr;
	}
}

#endif //BASE_SINGLETON_H