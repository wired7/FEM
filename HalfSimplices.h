#pragma once
#include <vector>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <boost/preprocessor/repeat.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/if.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>

namespace Geometry
{
	template<typename tPair>
	struct second_t
	{
		typename tPair::second_type operator()(const tPair& p) const
		{
			return p.second;
		}
	};

	template<typename tMap>
	second_t<typename tMap::value_type> second(const tMap& m)
	{
		return second_t<typename tMap::value_type>();
	};

	class TopologicalStruct
	{
	public:
		virtual void getAllChildren(std::unordered_set<TopologicalStruct*>& children)
		{
			children.insert(this);
		};
		virtual void getAllNthChildren(std::unordered_set<TopologicalStruct*>& children, int childDim, bool isFirstLevel = true)
		{
			getAllChildren(children);
		};
	};

	template <const int nDimensional, typename T> class HalfSimplex;

	template <const int nDimensional, typename T>
	class HalfSimplexWithChildren : public TopologicalStruct
	{
	public:
		// lower dimensional simplices that hold that binds two
		// simplices together. The direction follows winding
		// order.
		HalfSimplex<nDimensional - 1, T>* pointsTo;

		void getAllChildren(std::unordered_set<TopologicalStruct*>& children) override
		{
			auto first = pointsTo;
			auto current = first;

			if (first == nullptr)
			{
				return;
			}

			do
			{
				children.insert(reinterpret_cast<TopologicalStruct*>(current));
				current = current->next;
			} while (current != nullptr && current != first);
		};

		void getAllNthChildren(std::unordered_set<TopologicalStruct*>& children, int childDim, bool isFirstLevel = true) override
		{
			if (childDim > nDimensional)
			{
				return;
			}

			if (childDim == nDimensional)
			{
				children.insert(this);
				return;
			}

			std::unordered_set<TopologicalStruct*> immediateChildren;

			if (isFirstLevel)
			{
				getAllChildren(immediateChildren);
			}
			else if(pointsTo != nullptr)
			{
				immediateChildren.insert(pointsTo);
			}

			for (const auto& child : immediateChildren)
			{
				child->getAllNthChildren(children, childDim, false);
			}
		};

		void setPointsTo(HalfSimplex<nDimensional - 1, T>* pointsTo)
		{
			this->pointsTo = pointsTo;
		};
	};

	template <const int nDimensional, typename T>
	class HalfSimplex : public std::conditional<nDimensional, HalfSimplexWithChildren<nDimensional, T>, TopologicalStruct>::type
	{
	public:
		T halfSimplexData;
		// must have opposite orientation (the loops of pointsTo objects must
		// have opposite orders)
		HalfSimplex<nDimensional, T>* twin;
		HalfSimplex<nDimensional, T>* next;
		HalfSimplex<nDimensional, T>* previous;

		// larger-dimensional construct that this half-simplex belongs to.
		HalfSimplex<nDimensional + 1, T>* belongsTo;

		HalfSimplex() {};
		HalfSimplex(T halfSimplexData) : halfSimplexData(halfSimplexData) {};
		~HalfSimplex() {};

		void setTwin(HalfSimplex<nDimensional, T>* twin)
		{
			this->twin = twin;
			twin->twin = this;
		};

		void setNext(HalfSimplex<nDimensional, T>* next)
		{
			this->next = next;
		};

		void setPrevious(HalfSimplex<nDimensional, T>* previous)
		{
			this->previous = previous;
		};

		void setBelongsTo(HalfSimplex<nDimensional + 1, T>* belongsTo)
		{
			this->belongsTo = belongsTo;
		};

		std::map<HalfSimplex<nDimensional, T>*, HalfSimplex<nDimensional, T>*> getNeighbours()
		{
			std::map<HalfSimplex<nDimensional, T>*, HalfSimplex<nDimensional, T>*> neighbours;

			auto higherUp = reinterpret_cast<HalfSimplexWithChildren<nDimensional + 1, T>*>(belongsTo->next);
			HalfSimplex<nDimensional, T>* neighbour = higherUp->pointsTo;

			while(true)
			{
				auto twin = reinterpret_cast<HalfSimplex<nDimensional + 1, T>*>(higherUp)->twin;

				if (twin == nullptr)
				{
					break;
				}

				auto prev = twin->previous;

				if (neighbours[neighbour] == nullptr && prev != nullptr)
				{
					neighbours[neighbour] = neighbour;
				}
				else
				{
					break;
				}

				higherUp = reinterpret_cast<HalfSimplexWithChildren<nDimensional + 1, T>*>(prev);
				neighbour = higherUp->pointsTo;
			}

			return neighbours;
		}
	};

#define DEF_MAP_TYPES(i) /* ... */														\
		std::map<std::vector<T>, HalfSimplex<i, T>*> map ## i;							\
		std::map<std::vector<T>, std::unordered_set<HalfSimplex<i, T>*>> twinMap ## i;	\

#define DEF_POP_MAPS_RECURSE(dim) /* ... */									\
	void populateMapsRecursively ## dim(const std::vector<T>& indices)		\
	{																		\
		if(map ## dim[indices] == nullptr)									\
		{																	\
			map ## dim[indices] = new HalfSimplex<dim, T>(indices[0]);		\
			auto sortedIndices = indices;									\
			std::sort(sortedIndices.begin(), sortedIndices.end());			\
			twinMap ## dim[sortedIndices].insert(map ## dim[indices]);		\
																			\
			if(twinMap ## dim[sortedIndices].size() == 2)					\
			{																\
				(*twinMap ## dim[sortedIndices].begin())->setTwin(			\
					*std::next(twinMap ## dim[sortedIndices].begin(), 1));	\
			}																\
		}																	\
																			\
		HalfSimplex<BOOST_PP_DEC(dim), T>* previous = nullptr;				\
		HalfSimplex<BOOST_PP_DEC(dim), T>* first = nullptr;					\
		for(int iter = 0; iter < indices.size(); ++iter)					\
		{																	\
			std::vector<T> newIndices;										\
			for(int start = iter; start < iter + dim; ++start)				\
			{																\
				newIndices.push_back(indices[start % indices.size()]);		\
			}																\
																			\
			if(dim % 2 && iter % 2)											\
			{																\
				/* reverse indices */										\
			}																\
																			\
			BOOST_PP_IF(dim,												\
				BOOST_PP_CAT(populateMapsRecursively, BOOST_PP_DEC(dim))(newIndices);				\
				BOOST_PP_CAT(map, BOOST_PP_DEC(dim))[newIndices]->setBelongsTo(map ## dim[indices]);\
				map ## dim[indices]->setPointsTo(BOOST_PP_CAT(map, BOOST_PP_DEC(dim))[newIndices]);	\
				if(previous != nullptr)											\
				{																\
					previous->setNext(BOOST_PP_CAT(map, BOOST_PP_DEC(dim))[newIndices]);			\
					BOOST_PP_CAT(map, BOOST_PP_DEC(dim))[newIndices]->setPrevious(previous);		\
				}															\
				else														\
				{															\
					first = BOOST_PP_CAT(map, BOOST_PP_DEC(dim))[newIndices];\
				}															\
				previous = BOOST_PP_CAT(map, BOOST_PP_DEC(dim))[newIndices];\
			,)																\
		}																	\
		if(previous != nullptr)												\
		{																	\
			previous->setNext(first);										\
			first->setPrevious(previous);									\
		}																	\
	};																		\

#define DEF_CLEAR_MAPS(z, dim, _) /* ... */									\
		twinMap ## dim.clear();												\


#define DEF_CALL_POP_MAPS_RECURSE(dim) /* ... */							\
		populateMapsRecursively ## dim(newIndices);							\
		BOOST_PP_REPEAT(dim, DEF_CLEAR_MAPS)								\
		
#define MANIFOLD_CLASS_TEMPLATE(z, i, _) /* ... */							\
		template <typename T> class BOOST_PP_CAT(Manifold, BOOST_PP_CAT(i, BOOST_PP_IF(i, : public BOOST_PP_CAT(Manifold, BOOST_PP_DEC(i)) ## <T>,)))	\
		{																	\
		protected:															\
			DEF_MAP_TYPES(i)												\
			DEF_POP_MAPS_RECURSE(i)											\
																			\
			const int dimension = i;										\
		public:																\
		Manifold ## i() {};													\
			/* assumes an n-dimensional, properly tesselated structure,					
			and that winding has been taken care of. */						\
		Manifold ## i(std::vector<T> indices)								\
			{																\
				for(int simplexPts = 0;										\
					simplexPts < indices.size();							\
					simplexPts += (dimension + 1))							\
				{															\
					std::vector<T> newIndices;								\
					newIndices.insert(newIndices.end(),						\
									  indices.begin() + simplexPts,			\
									  indices.begin() + simplexPts + dimension + 1);	\
					DEF_CALL_POP_MAPS_RECURSE(i)							\
				}															\
			};																\
			~Manifold ## i() {};											\
																			\
			virtual std::vector<HalfSimplex<i, T>*> getHalfSimplices ## i()	\
			{																\
				std::vector<HalfSimplex<i, T>*> output;						\
				std::transform(map ## i.begin(), map ## i.end(), std::back_inserter(output), second(map ## i));\
				return output;												\
			};																\
		};			 														\

	BOOST_PP_REPEAT(5, MANIFOLD_CLASS_TEMPLATE, _)

	#undef DEF_MAP_TYPES
	#undef DEF_POP_MAPS_RECURSE
	#undef MANIFOLD_CLASS_TEMPLATE
};