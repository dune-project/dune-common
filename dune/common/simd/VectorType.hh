#include <array>

class VectorType{
	
	//@ToDo: templatisieren
	private static final int size = 5;
	
	private std::array<int,size> content;

	//Constructors
	VectorType(const std::array<int,size> &a){
		content = a;
	}

	
	//defining the unary operators
#define DUNE_SIMD_VECTORTYPE_PREFIX_OP(SYMBOL)
	auto operator ##SYMBOL(VectorType& V){
		for(auto &element : content)
			element SYMBOL;
	}


#define DUNE_SIMD_VECTORTYPE_POSTFIX_OP(SYMBOL)
	auto operator ##SYMBOL(VectorType& V){
		for(auto &element : content)
			SYMBOL element;
	}

#undef DUNE_SIMD_VECTORTYPE_PREFIX_OPS
#undef DUNE_SIMD_VECTORTYPE_POSTFIX_OPS	

};
