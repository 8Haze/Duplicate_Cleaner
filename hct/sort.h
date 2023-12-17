#ifndef _HCT_SORT_
#define _HCT_SORT_

#include <cmath>
#include <utility>
#include <iterator>
#include <future>
#include <algorithm>

namespace hct
{

	constexpr size_t _SORT_THRESHOLD_SEQ = 32;
	constexpr size_t _SORT_THRESHOLD_PAR = 50000;

	template <typename Iter>
	using _iter_diff_type = typename std::iterator_traits<Iter>::difference_type;

	template <typename Iter, typename Comp>
	void _sort_insertion(Iter iLeft, Iter iRight, Comp comp)
	{
		if (iLeft >= iRight)
			return;

		using T = typename std::iterator_traits<Iter>::value_type;

		for (Iter iMid = iLeft; ++iMid != iRight;)
		{
			Iter iGap = iMid;
			T temp = std::move(*iMid);

			if (comp(temp, *iLeft))
			{
				if constexpr (std::contiguous_iterator<Iter> && std::is_trivially_copyable<T>::value)
					memmove(std::addressof(*iLeft) + 1, std::addressof(*iLeft), sizeof(T) * (iGap - iLeft));
				else
				{
					Iter iBackup = iMid;
					++iMid;
					while (iGap != iLeft)
						*--iMid = std::move(*--iGap);
					iMid = iBackup;
				}
				*iLeft = std::move(temp);
			}
			else
			{
				for (Iter iPrev = iGap; comp(temp, *--iPrev); --iGap)
					*iGap = std::move(*iPrev);
				*iGap = std::move(temp);
			}
		}
	}

	template <typename Iter, typename Comp>
	void _heapify(Iter iLeft, Iter iRight, size_t idx_parent, _iter_diff_type<Iter> idx_first_incomplete_parent, size_t size, Comp comp)
	{
		using T = typename std::iterator_traits<Iter>::value_type;
		using Diff = typename std::iterator_traits<Iter>::difference_type;
		Diff idx_gap, idx;

		while (idx_parent > 0)
		{
			--idx_parent;
			T temp = std::move(*(iLeft + idx_parent));
			idx_gap = idx_parent;
			idx = idx_parent;

			while (idx < idx_first_incomplete_parent)
			{
				idx = (idx << 1) + 2;
				if (comp(*(iLeft + idx), *(iLeft + idx - 1)))
					--idx;
				if (comp(temp, *(iLeft + idx)))
				{
					*(iLeft + idx_gap) = std::move(*(iLeft + idx));
					idx_gap = idx;
				}
				else
					break;
			}

			if (idx == idx_first_incomplete_parent && size % 2 == 0)
				if (comp(temp, *(iRight - 1)))
				{
					*(iLeft + idx_gap) = std::move(*(iRight - 1));
					idx_gap = size - 1;
				}

			*(iLeft + idx_gap) = std::move(temp);
		}
	}

	template <typename Iter, typename Comp>
	void _make_heap(Iter iLeft, Iter iRight, Comp comp)
	{
		using Diff = typename std::iterator_traits<Iter>::difference_type;
		Diff size = iRight - iLeft;
		Diff idx_first_incomplete_parent = (size - 1) >> 1;
		Diff idx_parent = size >> 1;

		hct::_heapify(iLeft, iRight, idx_parent,
			idx_first_incomplete_parent, size, comp);
	}

	template <typename Iter, typename Comp>
	void _sort_heap(Iter iLeft, Iter iRight, Comp comp)
	{
		using Diff = typename std::iterator_traits<Iter>::difference_type;
		Diff size, idx, idx_gap;
		Diff idx_first_incomplete_parent;

		while (--iRight != iLeft)
		{
			std::swap(*iRight, *iLeft);

			size = iRight - iLeft;
			idx_first_incomplete_parent = (size - 1) >> 1;
			idx_gap = 0;
			idx = 0;

			hct::_heapify(iLeft, iRight, 1,
				idx_first_incomplete_parent, size, comp);
		}
	}

	template <typename Iter, typename Comp>
	void _median_of_three(Iter iLeft, Iter iMid, Iter iRight, Comp comp)
	{
		--iRight;

		if (comp(*iMid, *iLeft))
			std::swap(*iMid, *iLeft);
		if (comp(*iRight, *iMid))
		{
			std::swap(*iRight, *iMid);
			if (comp(*iMid, *iLeft))
				std::swap(*iMid, *iLeft);
		}
	}

	template <typename Iter, typename Comp>
	Iter _partition(Iter iLeft, Iter iRight, Comp comp)
	{
		Iter iMid = iLeft + ((iRight - iLeft - 1) >> 1);
		Iter iPivot = iMid;
		hct::_median_of_three(iLeft, iMid, iRight, comp);

		while (true)
		{
			while (comp(*iLeft, *iPivot)) ++iLeft;
			while (comp(*iPivot, *--iRight));

			if (iLeft >= iRight)
				return iRight;

			if (comp(*iLeft, *iRight) || comp(*iRight, *iLeft))
			{
				std::swap(*iLeft, *iRight);
				if (iPivot == iLeft)
					iPivot = iRight;
				else if (iPivot == iRight)
					iPivot = iLeft;
			}
		}
	}

	template <typename Iter, typename Comp>
	void _sort_procedure_seq(Iter iLeft, Iter iRight, _iter_diff_type<Iter> depth, Comp comp)
	{
		for (;;)
		{
			if (iRight - iLeft <= _SORT_THRESHOLD_SEQ)
			{
				hct::_sort_insertion(iLeft, iRight, comp);
				return;
			}

			if (depth <= 0)
			{
				hct::_make_heap(iLeft, iRight, comp);
				hct::_sort_heap(iLeft, iRight, comp);
				return;
			}

			Iter iPart = hct::_partition(iLeft, iRight, comp);

			depth = (depth >> 1) + (depth >> 2);

			if (iPart - iLeft < iRight - iPart)
			{
				hct::_sort_procedure_seq(iLeft, iPart + 1, depth, comp);
				iLeft = iPart + 1;
			}
			else
			{
				hct::_sort_procedure_seq(iPart + 1, iRight, depth, comp);
				iRight = iPart + 1;
			}
		}
	}

	template <typename Iter, typename Comp>
	void _sort_procedure_par(Iter iLeft, Iter iRight, _iter_diff_type<Iter> depth, Comp comp)
	{
		if (iRight - iLeft < _SORT_THRESHOLD_PAR || depth <= 0)
		{
			hct::_sort_procedure_seq(iLeft, iRight, depth, comp);
			return;
		}

		Iter iPart = hct::_partition(iLeft, iRight, comp);

		depth = (depth >> 1) + (depth >> 2);

		auto task = std::async(std::launch::async, [iLeft, iPart, depth, &comp]()
			{ hct::_sort_procedure_par(iLeft, iPart + 1, depth, comp); });

		hct::_sort_procedure_par(iPart + 1, iRight, depth, comp);

		task.wait();
	}

	template <typename Iter, typename Comp>
	inline void sort(Iter iLeft, Iter iRight, Comp comp)
	{
		if (iLeft >= iRight)
			return;

		hct::_sort_procedure_par(iLeft, iRight, iRight - iLeft, comp);
	}

	template <typename Iter>
	inline void sort(Iter iLeft, Iter iRight)
	{
		hct::sort(iLeft, iRight, std::less<>{});
	}

}

#endif // _HCT_SORT_