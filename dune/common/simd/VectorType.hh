#ifndef VECTORTYPE_HH
#define VECTORTYPE_HH

#include <array>

template<class T, int S>
class VectorType{

	std::array<T, S> content;

public:
	//Testconstructors
	//VectorType<int, 5>(): content( {5,4,3,2,1} ){
	//}

	VectorType<T, S>(std::array<T, S> &a){
		content = a;
	}

	//Access operator
	auto operator[](int idx) const	{
		return content[idx];
	}

#define DUNE_SIMD_VECTORTYPE_PREFIX_OP(SYMBOL)
	auto operator##SYMBOL(){
		for(auto &element : content)
			SYMBOL element;
		return *this;
	}

/**
#define DUNE_SIMD_VECTORTYPE_POSTFIX_OP(SYMBOL)
	auto operator SYMBOL(int){
		VectorType temp(
		for(auto &element : content)
			SYMBOL element;
	}*/

	DUNE_SIMD_VECTORTYPE_PREFIX_OP(++);
	DUNE_SIMD_VECTORTYPE_PREFIX_OP(--);
	//DUNE_SIMD_VECTORTYPE_POSTFIX_OP(++);
	//DUNE_SIMD_VECTORTYPE_POSTFIX_OP(--);

#undef DUNE_SIMD_VECTORTYPE_PREFIX_OPS
//#undef DUNE_SIMD_VECTORTYPE_POSTFIX_OPS
};
#endif
