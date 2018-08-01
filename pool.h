#ifndef __POOL_H__
#define __POOL_H__

/// #include <stdio.h>
#include <deque>
#include <stdlib.h>

/// 避免动态分配内存创建对象，不回收，不释放，内存足够，注意此内存池不是为了
/// 节约内存、复用内存，而是为了节约时间

template<typename T, size_t size>
class objpool 
{
public:
	T& get ()
	{
		if (index < size) {
			return que[index++];
		} else {
			index = 0;
		}

		/// 使用回收的内存，尽量不要执行这部分代码，遍历搜索效率比较低
		for (; index < size; ++index) {
			if ((*(int *)(que + index)) == 0x0a0b0c0dl) {
				return que[index++];
			}
		}

		abort ();
	}

	void put (T& v)
	{
		*((int *)&v) = 0x0a0b0c0dl;
	}
private:
	T que[size];
	size_t index = 0;
};

#if 0
struct AAA
{
    int a;
    int b;
};

int
main ()
{
    objpool<AAA, 512ull> pool;

    for (;;) {
        AAA& aaa = pool.get ();
        aaa.a = 10;
        aaa.b = 20;

        pool.put (aaa);
    }
}
#endif
#endif
