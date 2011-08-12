#include <bitset.hpp>
#include <stdint.h>
#include <cstdio>

class zero_copy_container
{
	public:
		typedef uint8_t value_type;
		typedef std::vector<value_type> Buffer;
		typedef Buffer::const_iterator const_iterator;
		typedef Buffer::size_type size_type;
	private:
		Buffer::const_iterator begin;
		Buffer::const_iterator end;
	public:
		zero_copy_container(const_iterator begin, const_iterator end)
			: begin(begin), end(end)
		{}

		value_type operator [] (size_type i) const
		{
			if (static_cast<size_type>(end - begin) < i) return value_type();
			return *(begin + i);
		}
};

static void dump(uint8_t v)
{
	for (int i = 0; i < 8; ++i) printf("%d", (v >> (7-i)) & 0x1);
}

int main(int, char **)
{
	bitset<uint8_t> a;
	a.append(0x0f, 8);
	a.append(0xf0, 8);

	zero_copy_container::Buffer buf;
	buf.push_back(0x0f);
	buf.push_back(0xf0);

//	dump(buf[0]); printf(" "); dump(buf[1]); printf("\n");

	bitset<uint8_t, zero_copy_container> b(buf.begin(), buf.end());

//	dump(0x0f); printf(" "); dump(0xf0); printf("\n");

	uint8_t w;
	a.get(w, 7, 6);
	printf("w = %02x : ", w); dump(w); printf("\n");

	uint8_t v;
	b.get(v, 7, 6);
	printf("v = %02x : ", v); dump(v); printf("\n");

	return 0;
}

