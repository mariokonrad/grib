#ifndef __OCTETS__HPP__
#define __OCTETS__HPP__

#include <vector>
#include <istream>

namespace grib2 {

class octets
{
	private:
		typedef std::vector<char> Data;
	public:
		typedef Data::size_type size_type;
		class exception {};

		/// @todo: TEST
		/// @todo: documentation
		class const_iterator
		{
				friend class octets;
			public:
				typedef Data::const_iterator::value_type value_type;
				typedef Data::const_iterator::reference reference;
				typedef Data::const_iterator::pointer pointer;
				typedef Data::const_iterator::difference_type difference_type;
				typedef Data::const_iterator::iterator_category iterator_category;
			private:
				const octets * data;
				size_type pos;
			private:
				const_iterator(const octets * const data, size_type pos)
					: data(data)
					, pos(pos)
				{}
			public:
				const_iterator()
					: data(NULL)
					, pos(0)
				{}

				const_iterator(const const_iterator & other)
					: data(other.data)
					, pos(other.pos)
				{}

				const_iterator & operator = (const const_iterator & other)
				{
					data = other.data;
					pos = other.pos;
					return *this;
				}

				bool operator == (const const_iterator & other) const
				{
					return true
						&& data == other.data
						&& pos == other.pos
						;
				}

				bool operator != (const const_iterator & other) const
				{
					return false
						|| data != other.data
						|| pos != other.pos
						;
				}

				bool operator < (const const_iterator & other) const
				{
					return true
						&& data == other.data
						&& pos < other.pos
						;
				}

				bool operator > (const const_iterator & other) const
				{
					return true
						&& data == other.data
						&& pos > other.pos
						;
				}

				bool operator <= (const const_iterator & other) const
				{
					return true
						&& data == other.data
						&& pos <= other.pos
						;
				}

				bool operator >= (const const_iterator & other) const
				{
					return true
						&& data == other.data
						&& pos >= other.pos
						;
				}

				char operator * () const throw (exception)
				{
					if (data == NULL || pos >= data->size()) throw exception();
					return data->get(pos);
				}

				const_iterator & operator += (size_type ofs)
				{
					if (data != NULL && pos < data->size()) {
						pos += ofs;
						if (pos > data->size()) pos = data->size();
					}
					return *this;
				}

				const_iterator & operator -= (size_type ofs)
				{
					if (data != NULL) {
						pos = (ofs > pos) ? 0 : pos - ofs;
					}
					return *this;
				}

				const_iterator & operator ++ () // ++const_iterator
				{
					if (data != NULL && pos < data->size()) {
						++pos;
					}
					return *this;
				}

				const_iterator & operator -- () // --const_iterator
				{
					if (data != NULL && pos > 0) {
						--pos;
					}
					return *this;
				}

				const_iterator operator ++ (int) // const_iterator++
				{
					const_iterator res(*this);
					if (data != NULL && pos < data->size()) {
						++pos;
					}
					return res;
				}

				const_iterator operator -- (int) // const_iterator--
				{
					const_iterator res(*this);
					if (data!= NULL && pos > 0) {
						--pos;
					}
					return res;
				}

				template <typename T> void peek(T & v, size_type n = sizeof(T)) const throw (exception)
				{
					if (data == NULL) throw exception();
					if (pos + n > data->size()) throw exception();
					v = T();
					for (size_type i = 0; i < n; ++i) {
						v <<= 8;
						v += static_cast<uint8_t>(data->get(pos + i));
					}
				}

				template <typename T> void read(T & v, size_type n = sizeof(T)) throw (exception)
				{
					peek(v, n);
					*this += n;
				}
		};
	private:
		Data data;
	public:
		octets()
		{}

		const_iterator begin() const
		{
			return const_iterator(this, 0);
		}

		const_iterator end() const
		{
			return const_iterator(this, data.size());
		}

		size_type size() const
		{
			return data.size();
		}

		void reserve(size_type n)
		{
			data.reserve(n);
		}

		void clear()
		{
			data.clear();
		}

		size_type append(std::istream & is, size_type n)
		{
			int8_t c;
			size_type cnt = 0;
			for (; is.good() && !is.eof() && cnt < n; ++cnt) {
				is.read(reinterpret_cast<char *>(&c), sizeof(c));
				data.push_back(c);
			}
			return cnt;
		}

		char get(size_type i) const throw (exception)
		{
			if (i >= data.size()) throw exception();
			return data[i];
		}

		char operator [] (size_type i) const throw (exception)
		{
			return get(i);
		}
};

}

#endif
