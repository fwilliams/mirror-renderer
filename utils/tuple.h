#include <type_traits>

#ifndef UTILS_TYPELIST_H_
#define UTILS_TYPELIST_H_

namespace utils {
namespace detail {

/*
 * A type representing the empty list
 */
struct EmptyListType {};


/*
 * A cons cell for a list of types
 */
template <size_t I, class... Elements>
struct TypeConsCell;

template <size_t I, class T>
struct TypeConsCell<I, T> {
	typedef T HeadType;
	typedef EmptyListType TailType;
};

template <size_t I, class T, class... Rest>
struct TypeConsCell<I, T, Rest...> {
	typedef T HeadType;
	typedef TypeConsCell<I + 1, Rest...> TailType;
};


/*
 * A cons cell for a list of types and values of those types
 */
#define __TL_DECL_VALUE_CONS_CELL(name) \
		template <size_t I, class... Elements> \
		struct name; \
		\
		template <size_t I, class T, class... Rest> \
		struct name<I, T, Rest...> { \
			T head; \
			name<I + 1, Rest...> tail; \
			\
			typedef T HeadType; \
			typedef name<I + 1, Rest...> TailType; \
		}; \
		\
		template <size_t I, class T> \
		struct name<I, T> { \
			T head; \
			\
			typedef T HeadType; \
			typedef EmptyListType TailType; \
		};

__TL_DECL_VALUE_CONS_CELL(ValueConsCell)

#pragma pack(push, 1)
__TL_DECL_VALUE_CONS_CELL(ValueConsCell1)
#pragma pack(pop)

#pragma pack(push, 2)
__TL_DECL_VALUE_CONS_CELL(ValueConsCell2)
#pragma pack(pop)

#pragma pack(push, 4)
__TL_DECL_VALUE_CONS_CELL(ValueConsCell4)
#pragma pack(pop)

#undef __TL_DECL_VALUE_CONS_CELL

/*
 * Metafunction determining if ConsCell contains only type information
 * i.e. is true if ConsCell does not contain and head, and tail element
 *      accessible at runtime
 */
template <class T>
struct IsConsCellTypes {
	constexpr static const bool value = false;
};

template <size_t S, class... Types>
struct IsConsCellTypes<TypeConsCell<S, Types...>> {
	constexpr static const bool value = true;
};

template <size_t S, class... Types>
struct IsConsCellTypes<ValueConsCell<S, Types...>> {
	constexpr static const bool value = false;
};

template <size_t S, class... Types>
struct IsConsCellTypes<ValueConsCell1<S, Types...>> {
	constexpr static const bool value = false;
};

template <size_t S, class... Types>
struct IsConsCellTypes<ValueConsCell2<S, Types...>> {
	constexpr static const bool value = false;
};

template <size_t S, class... Types>
struct IsConsCellTypes<ValueConsCell4<S, Types...>> {
	constexpr static const bool value = false;
};


/*
 * Metafunction determining if ConsCell is the tail of a compile-time list.
 */
template <class ConsCell>
struct IsTail {
	constexpr static const bool value =
			std::is_same<
			typename ConsCell::TailType,
			EmptyListType>();
};


/*
 * Metafunction determining the number of elements in a compile-time list.
 */
template <size_t I, bool isTail, class ConsCell>
struct NumElements;

template <size_t I, class  ConsCell>
struct NumElements<I, false, ConsCell> {
	constexpr static const size_t value =
			NumElements<
			I + 1,
			IsTail<typename ConsCell::TailType>::value,
			typename ConsCell::TailType>::value;
};

template <size_t I, class ConsCell>
struct NumElements<I, true, ConsCell> {
	constexpr static const size_t value = I;
};


/*
 * A structure used to extract the type, value and offset of the ith element in
 * a compile-time list.
 */
template <size_t I, class ConsCell>
struct ListElement {
private:
	typedef typename ConsCell::TailType CSTailType;
	typedef typename ConsCell::HeadType CSHeadType;

	static constexpr void assertValuelist() {
		// TODO: Make value holding a trait of ConsCells
		static_assert(
				IsConsCellTypes<ConsCell>::value == false,
				"Error: Cannot call value() on a TypeList");
	}

	static constexpr void assertInRange() {
		static_assert(
				I < NumElements<1, IsTail<ConsCell>::value, ConsCell>::value,
				"List index out of range");
	}

public:
	typedef typename ListElement<I - 1, typename ConsCell::TailType>::type type;

	static constexpr const type& value(const ConsCell& list) {
		assertValuelist();
		assertInRange();
		return ListElement<I-1, CSTailType>::value(list.tail);
	}

	static constexpr type& value(ConsCell& list) {
		assertValuelist();
		assertInRange();
		return ListElement<I-1, CSTailType>::value(list.tail);
	}

	static constexpr size_t offset() {
		assertValuelist();
		assertInRange();
		size_t padding = offsetof(ConsCell, tail) - sizeof(CSHeadType);
		size_t total = offsetof(ConsCell, head) + sizeof(CSHeadType) + padding;
		return total + ListElement<I-1, CSTailType>::offset();
	}
};

template <class ConsCell>
struct ListElement<0, ConsCell> {
private:
	static constexpr void assertValuelist() {
		static_assert(
				IsConsCellTypes<ConsCell>::value == false,
				"Error: Cannot call value() on a TypeList");
	}

	static constexpr void assertInRange() {
		static_assert(
				0 < NumElements<1, IsTail<ConsCell>::value, ConsCell>::value,
				"List index out of range");
	}

public:
	typedef typename ConsCell::HeadType type;

	static constexpr const type& value(const ConsCell& list) {
		assertValuelist();
		assertInRange();
		return list.head;
	}

	static constexpr type& value(ConsCell& list) {
		assertValuelist();
		assertInRange();
		return list.head;
	}

	static constexpr size_t offset() {
		assertValuelist();
		assertInRange();
		return offsetof(ConsCell, head);
	}
};

template <size_t I, class ListType, class... Args>
struct ListBuilder;

template <size_t I, class LT, class T, class... Args>
struct ListBuilder<I, LT, T, Args...> {
	static constexpr void build(LT& list, const T& val, const Args&... rest) {
		typedef detail::ListElement<I, LT> LE;
		LE::value(list) = val;
		ListBuilder<I+1, LT, Args...>::build(list, rest...);
	}
};

template <size_t I, class LT>
struct ListBuilder<I, LT> {
	static constexpr void build(LT& list) {
		return;
	}
};

//template <size_t I, class... Types>
//struct InitializerListListBuilder;
//
//template <size_t I, class T, class... Types>
//struct InitializerListListBuilder<I, T, Types...> {
//	static constexpr void build(T&) {
//
//	}
//};

template <template <size_t S, class... R> class ListNode, class... Attribs>
class Tuple {
	typedef ListNode<0, Attribs...> ElementListType;
	typedef Tuple<ListNode, Attribs...> MyType;

	template <size_t I>
	using ListElem = detail::ListElement<I, ElementListType>;

	ElementListType mAttribList;

public:
	typedef ListNode<0, Attribs...> ListType;

	template <size_t N>
	using ElementType = typename ListElem<N>::type;

	Tuple(const Attribs&... elements) {
		ListBuilder<0, ElementListType, Attribs...>::build(mAttribList, elements...);
	}

	Tuple() = default;

	static constexpr size_t size() {
		const bool isTail = detail::IsTail<ElementListType>::value;
		return detail::NumElements<1, isTail, ElementListType>::value;
	}

	template <size_t N>
	static constexpr size_t offset() {
		return ListElem<N>::offset();
	}

	template <size_t N>
	static constexpr const ElementType<N>& get(const MyType& v) {
		return ListElem<N>::value(v.mAttribList);
	}

	template <size_t N>
	constexpr const ElementType<N>& get() const {
		return ListElem<N>::value(mAttribList);
	}

	template <size_t N>
	static constexpr ElementType<N>& get(MyType& v) {
		return ListElem<N>::value(v.mAttribList);
	}

	template <size_t N>
	constexpr ElementType<N>& get() {
		return ListElem<N>::value(mAttribList);
	}
};

}


/*
 * A fixed-size tuple of values and associated type information.
 * Similar to TypeList but holds values which can be accessed/mutated at runtime
 * TODO: Proper layout parameters
 * TODO: Rvalue reference get()
 * TODO: Iterators
 */
template <class... Types>
using Tuple = detail::Tuple<detail::ValueConsCell, Types...>;

template <class... Types>
using TightTuple = detail::Tuple<detail::ValueConsCell1, Types...>;

template <class... Types>
using Tuple2 = detail::Tuple<detail::ValueConsCell2, Types...>;

template <class... Types>
using Tuple4 = detail::Tuple<detail::ValueConsCell4, Types...>;

/*
 * A list of types which can be queried at compile time
 */
template <class... Types>
using TypeList = detail::Tuple<detail::TypeConsCell, Types...>;


/*
 * Trait determining if a type is a tuple of elements
 */
template <class T>
struct IsTypeTuple {
	static constexpr const bool value = false;
};

template <class... Types>
struct IsTypeTuple<Tuple<Types...>> {
	static constexpr const bool value = true;
};

template <class... Types>
struct IsTypeTuple<TightTuple<Types...>> {
	static constexpr const bool value = true;
};

template <class... Types>
struct IsTypeTuple<Tuple2<Types...>> {
	static constexpr const bool value = true;
};

template <class... Types>
struct IsTypeTuple<Tuple4<Types...>> {
	static constexpr const bool value = true;
};

template <class... Types>
struct IsTypeTuple<TypeList<Types...>> {
	static constexpr const bool value = true;
};


}


#endif /* UTILS_TYPELIST_H_ */
